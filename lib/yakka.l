%{
#include <stdio.h>
#include <yakka_parser.hh>
%}

realnumber [e0-9.-]+
signalid [a-zA-Z0-9_]*
integer [0-9]+

%%
booster                    { return BOOSTER;}
{integer}                  { yylval.id=atoi(yytext);    return  ID;}
\[                         { return OPEN;}
\]                         { return CLOSE;}
=                          { return EQ;}
\<                         { return CMP;}
{signalid}                 { yylval.str=strdup(yytext); return  SIGNALNAME;}
{realnumber}               { yylval.d=atof(yytext);     return  FLOAT;}
yes=                       { return YES; }
,no=                       { return NO; }
,missing=                  { return MISS; }
leaf=                      { return LEAF; }
\n                         { return LF; }
[ \t:,]+                  /*     ignore  whitespace       */;
%%
