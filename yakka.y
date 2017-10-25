

%{
#include <stdio.h>
#include <string.h>

int yydebug=1; 
 
void yyerror(const char *str)
{
    fprintf(stderr,"error: %s\n",str);
}
 
int yywrap()
{
    return 1;
} 
  
main()
{
        yyparse();
} 

%}

%union 
{
    int id;
    double d;
    char* str;
}

%token BOOSTER OPEN CLOSE EQ CMP YES NO MISS LEAF
%token <id> ID 
%token <str> SIGNALNAME 
%token <d> FLOAT 

%%

boosters: booster
        | boosters booster
        ;

booster: booster_id treenodes;

treenodes: leaf leaf
        | treenode
        | treenodes leaf
        | treenodes treenode
        ;

booster_id: BOOSTER OPEN ID CLOSE
       {
       printf("booster %d\n", $3);
       };

treenode:
        treenode_flt
        |
        treenode_int
        ;

treenode_flt: ID OPEN SIGNALNAME CMP FLOAT CLOSE YES ID NO ID MISS ID
        {
            printf("TREENODE %d %s %f %d %d %d\n", $1, $3, $5, $8, $10, $12);
        };

treenode_int: ID OPEN SIGNALNAME CMP ID CLOSE YES ID NO ID MISS ID
        {
            printf("TREENODE %d %s %f %d %d %d\n", $1, $3, $5, $8, $10, $12);
        };

leaf: ID LEAF FLOAT
        {
            printf("LEAF %d %f\n", $1, $3);
        }
        ;
%%
