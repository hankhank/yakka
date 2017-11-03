
%{
#include <stdio.h>
#include <string.h>
#include "yakka_lexer.hh"
#include "yakka_tree.h"

//int yydebug=1; 
 
void yyerror(yakka::XGBoosters &boosters, const char *str)
{
    fprintf(stderr, "%s around line no \n",
            str);
}
 
int yywrap()
{
    return 1;
} 
  
%}

%code requires {#include "yakka_tree.h"}
%parse-param {yakka::XGBoosters &boosters} 

%union 
{
    int id;
    double d;
    char* str;
}

%token BOOSTER OPEN CLOSE EQ CMP YES NO MISS LEAF LF
%token <id> ID 
%token <str> SIGNALNAME 
%token <d> FLOAT 

%%

boosters: booster
        | boosters booster
        ;

booster: booster_id treenodes;

treenodes: leaf
        | leaf leaf
        | treenode
        | treenodes leaf
        | treenodes treenode
        ;

booster_id: BOOSTER OPEN ID CLOSE LF
       {
           boosters.push_back(yakka::XGBooster());
       };

treenode:
        treenode_flt
        |
        treenode_int
        ;

treenode_flt: ID OPEN SIGNALNAME CMP FLOAT CLOSE YES ID NO ID MISS ID LF
        {
            assert(boosters.size());
            boosters.back().AddTreeNode($1, $3, $5, $8, $10, $12); 
        };

treenode_int: ID OPEN SIGNALNAME CMP ID CLOSE YES ID NO ID MISS ID LF
        {
            assert(boosters.size());
            boosters.back().AddTreeNode($1, $3, $5, $8, $10, $12); 
        };

leaf: ID LEAF FLOAT LF
        {
            assert(boosters.size());
            boosters.back().AddLeaf($1, $3);
        }

leaf: ID LEAF ID LF
        {
            assert(boosters.size());
            boosters.back().AddLeaf($1, $3);
        }
        ;
%%
