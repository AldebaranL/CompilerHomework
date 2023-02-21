%{
 /****************************************************************************
 expr3.y
 YACC f i l e
 Date: xxxx/xx/xx
 xxxxx <xxxxx@nbjl.nankai.edu.cn>
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>

using namespace std;


map<string,double>  symbol_table;
char idStr[50];
int yylex ();
extern int yyparse();
FILE*yyin ;
void yyerror(const char*s);
%}

%union YYSTYPE{
  string IDStr;
  double val;
};
%token<val> NUMBER
%token<IDStr> ID
%token ADD
%token SUB
%token MUL
%token DIV
%token EQ
%left ADD SUB
%left MUL DIV
%right UMINUS
%right EQ

%nterm <val> expr
%nterm <IDStr> LVal

%%


lines : lines expr ';' { printf("%lf\n", $2);}
|       lines ';'
|       LVal EQ expr ';' {symbol_table[$1] = $3;}
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
| ID     { $$ = symbol_table[$1];}
;



%%

//programs section

int yylex()
{
    //place your token retrieving code here
    char t = getchar ();
    while (t == ' ' || t== '\t' || t == '\n') t = getchar ();
    if (t >= '0' && t <= '9') {
        yylval = 0;
        while(t<='9'&&t>='0'){
            yylval = yylval * 10 + t - '0';
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
        if(symbol_table.find(string(idStr)) == symbol_table.end()){
            symbol_table[string(idStr)]=0;
        }
        yylval = string(idStr) ;
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