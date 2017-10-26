
#include "yakka_lexer.hh"
#include "yakka_parser.hh"

const char* test_tree =
"booster[0]:\n"
"0:[f2<22485] yes=1,no=2,missing=1\n"
"        1:[f3<27] yes=3,no=4,missing=3\n"
"                3:[f2<10000] yes=7,no=8,missing=7\n"
"                        7:leaf=29.2258\n"
"                        8:leaf=66.88\n"
"                4:[f3<96.5] yes=9,no=10,missing=9\n"
"                        9:leaf=106.348\n"
"                        10:leaf=194.917\n"
"        2:[f2<48000] yes=5,no=6,missing=5\n"
"                5:[f3<56] yes=11,no=12,missing=11\n"
"                        11:leaf=166.273\n"
"                        12:leaf=349.179\n"
"                6:leaf=673.375\n"
"booster[1]:\n"
"0:[f3<80.5] yes=1,no=2,missing=1\n"
"        1:[f1<4620] yes=3,no=4,missing=3\n"
"                3:[f4<10] yes=7,no=8,missing=7\n"
"                        7:leaf=-0.967454\n"
"                        8:leaf=-33.8347\n"
"                4:[f23<-9.53674e-07] yes=9,no=10,missing=9\n"
"                        9:leaf=14.3699\n"
"                        10:leaf=87.0657\n"
"        2:[f2<48000] yes=5,no=6,missing=5\n"
"                5:[f0<80] yes=11,no=12,missing=11\n"
"                        11:leaf=86.3795\n"
"                        12:leaf=-24.2778\n"
"                6:leaf=237.083";

int main(void)
{
    YY_BUFFER_STATE buf;
    buf = yy_scan_string(test_tree);
    yyparse(nullptr);
    yy_delete_buffer(buf);
    yylex_destroy();
    return 0;
}
