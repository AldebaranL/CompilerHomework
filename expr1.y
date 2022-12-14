%{
 /************************************************************
 expr1.y
 YACC f i l e
 Date: xxxx/xx/xx
 xxxxx <xxxxx@nbjl.nankai.edu.cn>
 ************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifndef YYSTYPE
#define YYSTYPE double
#endif
int yylex ();
extern int yyparse();
FILE* yyin ;
void yyerror(const char* s );
%}

%token ADD
%token SUB
%token MUL
%token DIV
%token NUMBER
%left ADD SUB
%left MUL DIV
%right UMINUS

%%

// rules section

lines : lines expr '\n' { printf("%f\n", $2); }
| lines '\n'
|
;

expr : expr ADD expr { $$ = $1 + $3; }
| expr SUB expr { $$ = $1 - $3; }
| expr MUL expr { $$ = $1 * $3; }
| expr DIV expr { $$ = $1 / $3; }
| '(' expr ')' { $$ = $2; }
| '-' expr %prec UMINUS { $$ = -$2; }
| NUMBER { $$ = $1; }
;

%%

// programs section
int yylex()
{
    char t = getchar();
    while(t == ' ' || t == '\t') t = getchar();
    if(t<='9'&&t>='0'){
        yylval = 0;
        while(t<='9'&&t>='0'){
            yylval = yylval * 10 + t - '0';
            t = getchar();
        }
        ungetc(t,stdin);
        return NUMBER;
    }
    else switch(t){
        case '+':
            return ADD;
        case '-':
            return SUB;
        case '*':
            return MUL;
        case '/':
            return DIV;
        default:
            return t;
    }
}

int main(void)
{
    yyin = stdin ;
    do {
        yyparse();
    } while (!feof (yyin));
    return 0;
}
void yyerror(const char* s) {
    fprintf (stderr ,"Parse error : %s\n", s );
    exit (1);
}
