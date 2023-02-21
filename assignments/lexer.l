
%option noyywrap
%{
    /*
    * You will need to comment this line in lab5.
    */
    #define ONLY_FOR_LEX
    
    #ifdef ONLY_FOR_LEX

    int offset;
    #include <ostream>
    #include <fstream>
    #include <sstream>
    #include<iomanip>
    #include<vector>
    #include<map>
    #include<string>
    //语句块树，block_tree[i]为tag==i的节点，其存储父亲的tag，block_tree[0]=0
    struct node{
        int fa;
        //std::map<std::string,double>;//为每个id分配内存空间，但在本实验中不必要，将在后续实现。
    };
    std::vector<node> block_tree={(node){0}};
    
    struct depth_with_tag{
        int depth_num;
        int tag;
    };
    long unsigned int previous_depth = 0;//当前所处{}层数
    int previous_tag = 0;//单增常量，当前要分配的tag号，每个语句块有一个唯一tag
    //previous_depth_list是临时记录{}的层结构，以构建block_tree，symbol_table_depth_list[i]代表第i层深的{}，
    //symbol_table_depth_list[i].depth_num代表该语句块是当前层的第几个
    //symbol_table_depth_list[i].tag代表当前语句块的标识符
    //最外层tag为0
    std::vector<depth_with_tag> previous_depth_list={(depth_with_tag){0,previous_tag++}};

    #else
    #define YYSTYPE void *
    #include "parser.h"
    #endif

    #define YY_NO_UNPUT
    #define YY_NO_INPUT
    #include <string>

    #ifdef ONLY_FOR_LEX
    #include <ostream>
    #include <fstream>
    using namespace std;
    extern FILE *yyin; 
    extern FILE *yyout;

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
        previous_depth++;
        if(previous_depth>=previous_depth_list.size()){
            previous_depth_list.push_back((depth_with_tag){0,previous_tag++});
        }
        else{
            previous_depth_list[previous_depth].depth_num++;
            previous_depth_list[previous_depth].tag = previous_tag++;
        }
        block_tree.push_back((node){previous_depth_list[previous_depth-1].tag});
        DEBUG_FOR_LAB4("LBRACE","{");
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        previous_depth--;
        DEBUG_FOR_LAB4("RBRACE","}");
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
{ID} {
    #ifdef ONLY_FOR_LEX 
        char str[100];
        int the_tag=previous_depth_list[previous_depth].tag;
        sprintf(str,"%s\t%d\t%d",yytext,the_tag,block_tree[the_tag].fa);
        //for(int i=0;i<previous_tag;i++)printf("%d ",block_tree[i].fa);printf("\n");
        DEBUG_FOR_LAB4("ID",str);
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