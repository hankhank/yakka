
find_package(LLVM REQUIRED CONFIG)
find_package(BISON)
find_package(FLEX)

BISON_TARGET(YakkaParser yakka.y ${CMAKE_CURRENT_BINARY_DIR}/yakka_parser.cc)
FLEX_TARGET(YakkaLexer yakka.l  ${CMAKE_CURRENT_BINARY_DIR}/yakka_lexer.cc  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/yakka_lexer.hh" )
ADD_FLEX_BISON_DEPENDENCY(YakkaLexer YakkaParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(".")


add_library(yakka STATIC yakka_tree.cc
${BISON_YakkaParser_OUTPUTS}
${FLEX_YakkaLexer_OUTPUTS}
)

target_include_directories (yakka PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yakka ${FLEX_LIBRARIES} ${BISON_LIBRARIES})

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader executionengine interpreter mcjit mc nativecodegen nvptxcodegen bitreader asmparser passes)
message(STATUS "Using LLVMConfig.cmake in: ${llvm_libs}")
target_link_libraries (yakka ${llvm_libs} )


