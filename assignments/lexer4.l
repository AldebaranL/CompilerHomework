%option noyywrap
%top{
#include "SymbolTable.h"
#include<stdlib.h>
#include<stdio.h>
}
%{
    /*
    * You will need to comment this line in lab5.
    */
    #define ONLY_FOR_LEX
    
    #ifdef ONLY_FOR_LEX
    #else
    #define YYSTYPE void *
    #include "parser.h"
    
    #endif

    #define YY_NO_UNPUT
    #define YY_NO_INPUT
    #include <string>
    #include <algorithm>

    #ifdef ONLY_FOR_LEX
    #include <ostream>
    #include <fstream>
    #include <sstream>
    #include<iomanip>
    using namespace std;
    extern FILE *yyin; 
    extern FILE *yyout;

    int offset=0;

    SymbolTableNode* top=new SymbolTableNode(nullptr);
    SymbolTableNode* curr=nullptr;
    int level=0;

    string int2str(int n){
        string s;
        char c;
        if(n==0)
            return "0";
        while(n!=0){
            int tmp=n%10;
            c=(char)(tmp+'0');
            s+=c;
            n/=10;
        }
        reverse(s.begin(),s.end());
        return s;
    }
    string oct2dec(string o){
        int len=o.length();
        int v=0;
        int e=1;
        int res=0;
        for(int i=len-1;i>=0;i--){
            v=(int)(o[i]-'0');
            res+=v*e;
            e*=8;
        }
        return int2str(res);
    }
    string hex2dec(string h){
        int len=h.length();
        int v=0;
        int e=1;
        int res=0;
        for(int i=len-1;i>=2;i--){
            v=(int)(h[i]-'0');
            res+=v*e;
            e*=16;
        }
        return int2str(res);
    }

    void DEBUG_FOR_LAB4(string token, string val, string id_pointer=""){
        stringstream outss;
        outss<<setiosflags(ios::fixed)<<setprecision(6)<<setiosflags(ios::left);
        outss<<setw(10)<<token<<setw(10)<<val<<setw(10)<<yylineno<<setw(10)<<offset<<setw(10)<<id_pointer<<endl;
        //fputs(DEBUG_INFO.c_str(), yyout);
        //c_str ()函数返回一个指向正规C字符串的指针常量, 内容与本string串相同.
        fputs(outss.str().c_str(), yyout);
    }
    #endif
%}

DECIMIAL ([1-9][0-9]*|0)
OCTAL 0([1-7][0-7]*|0)
HEX (0x|0X)([1-9a-fA-F][0-9a-fA-F]*|0)
FLOAT {DECIMIAL}+(.{DECIMIAL}+)?((e|E)(\+|-)?{DECIMIAL}+)?
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
LINECOMMENT \/\/[^\n]*
commentbegin "/*"
commentelement .|\n
commentend "*/"
%x BLOCKCOMMENT
%option yylineno
%%

"break" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("BREAK","break");
        offset++;
    #else
        return BREAK;
    #endif 
}
"const" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONST","const");
        offset++;
    #else
        return CONST;
    #endif 
}
"continue" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("CONTINUE","continue");
        offset++;
    #else
        return CONTINUE;
    #endif 
}
"for" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("FOR","for");
        offset++;
    #else
        return FOR;
    #endif 
}
"while" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("WHILE","while");
        offset++;
    #else
        return WHILE;
    #endif 
}
"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("INT","int");
        offset++;
    #else
        return INT;
    #endif
}
"void" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("VOID","void");
        offset++;
    #else
        return VOID;
    #endif 
}
"if" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("IF","if");
        offset++;
    #else
        return IF;
    #endif
};
"else" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ELSE","else");
        offset++;
    #else
        return ELSE;
    #endif
};
"return" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RETURN","return");
        offset++;
    #else
        return RETURN;
    #endif
}
"while" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("WHILE","while");
        offset++;
    #else
        return WHILE;
    #endif
}
"!" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LOGIC_NOT","!");
        offset++;
    #else
        return LOGIC_NOT;
    #endif
}
"&&" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LOGIC_AND","&&");
        offset++;
    #else
        return LOGIC_AND;
    #endif
}
"||" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LOGIC_OR","||");
        offset++;
    #else
        return LOGIC_OR;
    #endif
}
"*" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("PRODUCT","*");
        offset++;
    #else
        return PRODUCT;
    #endif
}
"/" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DIVISION","/");
        offset++;
    #else
        return DIVISION;
    #endif
}
"%" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("REMAINDER","%");
        offset++;
    #else
        return REMAINDER;
    #endif
}
"~" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOT","~");
        offset++;
    #else
        return NOT;
    #endif
}
"&" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("AND","&");
        offset++;
    #else
        return AND;
    #endif
}
"|" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("OR","|");
        offset++;
    #else
        return OR;
    #endif
}
"^" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("XOR","^");
        offset++;
    #else
        return XOR;
    #endif
}
"+=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADDITION ASSIGNMENT","+=");
        offset++;
    #else
        return ADDITION_ASSIGNMENT;
    #endif
}
"-=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SUBTRACTION ASSIGNMENT","-=");
        offset++;
    #else
        return SUBTRACTION_ASSIGNMENT;
    #endif
}
"*=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MULTIPLICATION ASSIGNMENT","*=");
        offset++;
    #else
        return MULTIPLICATION_ASSIGNMENT;
    #endif
}
"/=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DIVISION ASSIGNMENT","/=");
        offset++;
    #else
        return DIVISION_ASSIGNMENT;
    #endif
}
"%=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("MODULO ASSIGNMENT","%=");
        offset++;
    #else
        return MODULO_ASSIGNMENT;
    #endif
}
"==" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("EQUAL TO","==");
        offset++;
    #else
        return EQUAL_TO;
    #endif
}
"!=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("NOT EQUAL TO","!=");
        offset++;
    #else
        return NOT_EQUAL_TO;
    #endif
}
"<=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS EQUAL","<=");
        offset++;
    #else
        return LESS_EQUAL;
    #endif
}
">=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATER EQUAL",">=");
        offset++;
    #else
        return GREATER_EQUAL;
    #endif
}
"=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ASSIGN","=");
        offset++;
    #else
        return ASSIGN;
    #endif
}
"<" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS","<");
        offset++;
    #else
        return LESS;
    #endif
}
">" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("GREATER",">");
        offset++;
    #else
        return GREATER;
    #endif
}
"+" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADD","+");
        offset++;
    #else
        return ADD;
    #endif
}
";" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SEMICOLON",";");
        offset++;
    #else
        return SEMICOLON;
    #endif
}
"(" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LPAREN","(");
        offset++;
    #else
        return LPAREN;
    #endif
}
")" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RPAREN",")");
        offset++;
    #else
    return RPAREN;
    #endif
}
"[" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LSQUARE","[");
        offset++;
    #else
    return LSQUARE;
    #endif
}
"]" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RSQUARE","]");
        offset++;
    #else
    return RSQUARE;
    #endif
}
"{" {
    #ifdef ONLY_FOR_LEX
        curr=top;
        top=new SymbolTableNode(curr);
        cout<<yylineno<<" "<<top<<endl;
        level++;
        DEBUG_FOR_LAB4("LBRACE","{");
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RBRACE","}");
        offset++;
        if(top!=nullptr){
            top=top->prev;
            level--;
        }
    #else
        return RBRACE;
    #endif
}
"," {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("COMMA",",");
        offset++;
    #else
        return COMMA;
    #endif
}
{ID}    {
    #ifdef ONLY_FOR_LEX
        top->idlist->insert(yytext);
        std::stringstream ss1;
        ss1 << (void *)top->idlist->find(yytext);  
        DEBUG_FOR_LAB4("ID", yytext, ss1.str());
        offset++;
    #else
        return ID;
    #endif
}

{DECIMIAL}  {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("DECIMIAL",yytext);
        offset++;
    #else
        return DECIMIAL;
    #endif
}
{HEX} {
    #ifdef ONLY_FOR_LEX
        int val;
        sscanf(yytext, "%x", &val);
        DEBUG_FOR_LAB4("HEX",std::to_string(val));
        offset++;
    #else
        return DECIMIAL;
    #endif
}
{OCTAL} {
    #ifdef ONLY_FOR_LEX
        int val;
        sscanf(yytext, "%o", &val);
        DEBUG_FOR_LAB4("OCTAL",std::to_string(val));
        offset++;
    #else
        return DECIMIAL;
    #endif
}
{FLOAT} {
        #ifdef ONLY_FOR_LEX
            double val;
            sscanf(yytext, "%lf", &val);
            DEBUG_FOR_LAB4("FLOAT",yytext);
        #else
            sscanf(yytext, "%f", &yylval.ftype);
            return FLOAT;
        #endif
}
{LINECOMMENT} {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LINECOMMENT","0");
    #endif
}
{commentbegin} {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("commentbegin","0");
        BEGIN BLOCKCOMMENT;
    #endif
}
<BLOCKCOMMENT>{commentend} {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("commentend","0");
        BEGIN INITIAL;
    #endif
}
<BLOCKCOMMENT>{commentelement} {}


{EOL} {yylineno++;offset=0;}
{WHITE}
%%

#ifdef ONLY_FOR_LEX
int main(int argc, char **argv){
    if(argc != 5){
        fprintf(stderr, "Argument Not Enough");
        exit(EXIT_FAILURE);
    }

    if(!(yyin = fopen(argv[1], "r"))){
        fprintf(stderr, "No such file or directory: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    if(!(yyout = fopen(argv[3], "w"))){
        fprintf(stderr, "No such file or directory: %s", argv[3]);
        exit(EXIT_FAILURE);
    }

    yylex();
    return 0;
}
#endif
