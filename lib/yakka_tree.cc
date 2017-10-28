
#include "yakka_tree.h"
#include "yakka_lexer.hh"
#include "yakka_parser.hh"

#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace yakka {

llvm::BasicBlock* BuildTree(llvm::LLVMContext* llvmContext, const std::vector<XGTreeNode>& tree, 
        int id, llvm::Value* pred, llvm::Function* func, const std::unordered_map<std::string, double*> lookup)
{
    auto& node = tree[id];

    auto *block = llvm::BasicBlock::Create(*llvmContext, std::to_string(id), func);
    llvm::IRBuilder<> blockBuilder(block);
    llvm::Value* condOrLeaf  = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*llvmContext), node.condOrLeafVal);

    if((node.yesJump == -1) && (node.noJump == -1))
    {
        pred = blockBuilder.CreateLoad(pred);
        blockBuilder.CreateStore(blockBuilder.CreateFAdd(pred, condOrLeaf), pred);
    }
    else
    {
        auto fptr = lookup.find(node.name);
        if(fptr == lookup.end())
        {
            assert(false && "Feature not in lookup");
            return nullptr;
        }
        auto* intptr = llvm::ConstantInt::get(llvm::Type::Int64Ty, *fptr);
        auto* constPtr = llvm::ConstantExpr::getIntToPtr(intptr, llvm::PointerType::getUnqual(llvm::Type::Int64Ty)); 
        auto* featureVal = blockBuilder.CreateLoad(constPtr);
        auto* goLeft = blockBuilder.CreateFCmpOLT(node.condOrLeafVal, featureVal);
        blockBuilder.CreateCondBr(goLeft, BuildTree(llvmContext, tree, node.yesJump, pred, func, lookup), 
            BuildTree(llvmContext, tree, node.noJump, pred, func, lookup));
    }
    return block;
}
   
std::unique_ptr<llvm::Module> JitTree(const std::string& treeStr,
        const std::unordered_map<std::string, double*> lookup, double* predPtr)
{
    yakka::XGBoosters boosters;
    YY_BUFFER_STATE buf;
    buf = yy_scan_string(treeStr.c_str());
    yyparse(boosters);
    yy_delete_buffer(buf);
    yylex_destroy();
    
    // JIT IT BABY
    auto* llvmContext = new llvm::LLVMContext;

    auto module = std::unique_ptr<llvm::Module>(new llvm::Module("yakka", *llvmContext));
    llvm::Module *M = module.get();

    // Create valuation function
    std::vector<llvm::Type*> inoutargs;

    auto* predictFunc =
    llvm::cast<llvm::Function>(M->getOrInsertFunction("predict",
                    llvm::FunctionType::get(
                        llvm::Type::getDoubleTy(*llvmContext), // void return
                        inoutargs,
                        false))); // no var args

    auto *entry = llvm::BasicBlock::Create(*llvmContext, "predict-entry", predictFunc);
    llvm::IRBuilder<> predictBuilder(entry);

    auto* intptr = llvm::ConstantInt::get(llvm::Type::Int64Ty, predPtr);
    auto* pred = llvm::ConstantExpr::getIntToPtr(intptr, llvm::PointerType::getUnqual(llvm::Type::Int64Ty)); 
    for(auto& booster : boosters)
    {
        BuildTree(llvmContext, booster.tree, 0, pred, predictFunc, lookup);
    }

    predictBuilder.CreateRetVoid();

    // Output asm
    std::string out;
    llvm::raw_string_ostream rawout(out);
    if(llvm::verifyModule(*M, &rawout))
    {
        rawout << *predictFunc;
        throw std::runtime_error("Failed to verify module");
    }
    return module;
}

};
