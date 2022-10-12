%{
/****************************************************************************
 expr3.y
 YACC f i l e
 Date: xxxx/xx/xx
 xxxxx <xxxxx@nbjl.nankai.edu.cn>
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

char idStr[50];
int yylex ();
extern int yyparse();
FILE*yyin ;
void yyerror(const char*s);
#define MAXsynbol_num 50
struct tableItem{
    char name[50];
    double val;
}symbol_table[MAXsynbol_num];

int tableCount = 0;

int inTable(char* str){
    for(int i=0;i<tableCount;i++){
        if(strcmp(symbol_table[i].name, str) == 0){
            return i;
        }
    }
    if(tableCount<MAXsynbol_num){
        strcpy(symbol_table[tableCount].name,str);
        tableCount++;
        return tableCount-1;
    }
    else{
        printf("symbol_table is full!\n");
        return -1;   
    }
}
%}

%union{
    char* name;
    double val;
}
%token<val> NUMBER
%token<name> ID
%token <double>ADD
%token <double>SUB
%token <double>MUL
%token <double>DIV
%token <double>EQ

%left ADD SUB
%left MUL DIV
%right UMINUS
%right EQ

%nterm <val> expr
%nterm <name> LVal

%%


lines : lines expr ';' { printf("%lf\n", $2);}
|       lines ';'
|       lines LVal EQ expr ';' 
{if(inTable($2)!=-1){
    symbol_table[inTable($2)].val=$4;
}}
|
;

LVal : ID {$$ = $1;};

expr :
  expr ADD expr { $$ = $1 + $3; }
| expr SUB expr { $$ = $1 - $3; }
| expr MUL expr { $$ = $1* $3 ; }
| expr DIV expr { $$ = $1 / $3; }
| '(' expr ')' { $$ = $2; }
| '-' expr %prec UMINUS { $$ = -$2; }
| NUMBER { $$ = $1; }
| ID     
{if(inTable($1)!=-1){
    $$ = symbol_table[inTable($1)].val;
}
else
$$ = 0;
}
;



%%

//programs section
int yylex()
{
    //place your token retrieving code here
    char t = getchar ();
    while (t == ' ' || t== '\t' || t == '\n') t = getchar ();
    if (t >= '0' && t <= '9') {
        yylval.val = 0;
        while(t<='9'&&t>='0'){
            yylval.val = yylval.val * 10.0 + (double)t - (double)'0';
            t = getchar();
        }
        ungetc(t,stdin);
        return NUMBER;
    }
    else if (( t >= 'a' && t <= 'z' ) || ( t >= 'A' && t<= 'Z' ) || ( t == '_' ) ){
        int ti=0;
        while (( t >= 'a' && t <= 'z' ) || ( t >= 'A' && t<= 'Z' )
            || ( t == '_') || ( t >= '0' && t <= '9' ) ) {
            idStr[ti++] = t ;
            t = getchar ();
        }
        idStr[ ti ] = '\0';
        //printf("%s\n",idStr);
        yylval.name = idStr;
        ungetc(t, stdin);
        return ID;
    }
    else if (t=='+')
        return ADD;
    else if (t=='-')
        return SUB;
    else if (t=='*')
        return MUL;
    else if (t=='/')
        return DIV;
    else if (t=='=')
        return EQ;
    else { return t;}
}

int main(void){
    yyin = stdin ;
    do {
        yyparse();
    } while (! feof (yyin));
    return 0;
}

void yyerror(const char*s) {
    fprintf (stderr , "Parse error : %s\n", s );
    exit (1);
}
