
#include <iostream>

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
        int id, llvm::Function* func, const std::unordered_map<std::string, double*> lookup)
{
    auto& node = tree[id];

    auto *block = llvm::BasicBlock::Create(*llvmContext, std::to_string(id), func);
    llvm::IRBuilder<> blockBuilder(block);
    llvm::Value* condOrLeaf  = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*llvmContext), node.condOrLeafVal);

    if((node.yesJump == -1) && (node.noJump == -1))
    {
        blockBuilder.CreateRet(condOrLeaf);
    }
    else
    {
        auto fptr = lookup.find(node.name);
        if(fptr == lookup.end())
        {
            assert(false && "Feature not in lookup");
            return nullptr;
        }
        auto* ptrAsInt = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*llvmContext), (uintptr_t)fptr->second);
        auto* constPtr = llvm::ConstantExpr::getIntToPtr(ptrAsInt, llvm::Type::getDoublePtrTy(*llvmContext));
        auto* featureVal = blockBuilder.CreateLoad(constPtr);
        auto* goLeft = blockBuilder.CreateFCmpOLT(blockBuilder.CreateSIToFP(featureVal,blockBuilder.getDoubleTy()),
            blockBuilder.CreateSIToFP(condOrLeaf,blockBuilder.getDoubleTy()));
        blockBuilder.CreateCondBr(goLeft, BuildTree(llvmContext, tree, node.yesJump, func, lookup), 
            BuildTree(llvmContext, tree, node.noJump, func, lookup));
    }
    return block;
}
   
std::unique_ptr<llvm::Module> JitTree(const std::string& treeStr,
        const std::unordered_map<std::string, double*> lookup)
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
    std::vector<llvm::Type*> noargs;

    auto* predictFunc =
    llvm::cast<llvm::Function>(M->getOrInsertFunction("predict",
                    llvm::FunctionType::get(
                        llvm::Type::getDoubleTy(*llvmContext), // return double
                        noargs,
                        false))); // no var args

    auto *entry = llvm::BasicBlock::Create(*llvmContext, "predict-entry", predictFunc);
    llvm::IRBuilder<> predictBuilder(entry);

    llvm::Value* pred = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*llvmContext), 0);
    int i = 0;
    for(auto& booster : boosters)
    {
        auto* bfunc = 
        llvm::cast<llvm::Function>(M->getOrInsertFunction(std::string("booster_") + std::to_string(i),
                        llvm::FunctionType::get(
                            llvm::Type::getDoubleTy(*llvmContext), // return double
                            noargs,
                            false))); // no var args
        BuildTree(llvmContext, booster.tree, 0, bfunc, lookup);
        pred = predictBuilder.CreateFAdd(pred, predictBuilder.CreateCall(bfunc));
        ++i;

        std::string out;
        llvm::raw_string_ostream rawout(out);
        rawout << *bfunc;
        std::cout << out;
    }

    predictBuilder.CreateRet(pred);

    // Output asm
    std::string out;
    llvm::raw_string_ostream rawout(out);
    if(llvm::verifyModule(*M, &rawout))
    {
        rawout << *predictFunc;
        std::cout << out;
        throw std::runtime_error("Failed to verify module");
    }
        rawout << *predictFunc;
        std::cout << out;
    return module;
}

YakkaFunc BuildYakkaTree(const std::string& treeStr, 
        const std::unordered_map<std::string, double*> lookup)
{
    auto module = JitTree(treeStr, lookup);

    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    std::string error;
    auto* llvmExecEngine = llvm::EngineBuilder(std::move(module))
                                .setEngineKind(llvm::EngineKind::JIT)
                                .setOptLevel(llvm::CodeGenOpt::Level::Aggressive)
                                .setErrorStr(&error)
                                .create();
    llvmExecEngine->DisableGVCompilation(true);
    if(!llvmExecEngine)
    {
        // throw here
        std::cout << error;
        assert(llvmExecEngine);
    }

    auto ret = reinterpret_cast<YakkaFunc>(llvmExecEngine->getPointerToNamedFunction("predict"));
    llvmExecEngine->finalizeObject();
    return ret;
}
};
