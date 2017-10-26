cmake_minimum_required(VERSION 3.6)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(YakkaParser yakka.y ${CMAKE_CURRENT_BINARY_DIR}/yakka_parser.cc)
FLEX_TARGET(YakkaLexer yakka.l  ${CMAKE_CURRENT_BINARY_DIR}/yakka_lexer.cc  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/yakka_lexer.hh" )
ADD_FLEX_BISON_DEPENDENCY(YakkaLexer YakkaParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(yakka_tester
test.cc
${BISON_YakkaParser_OUTPUTS}
${FLEX_YakkaLexer_OUTPUTS}
)

target_link_libraries(yakka_tester ${FLEX_LIBRARIES} ${BISON_LIBRARIES})
