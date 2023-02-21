%option noyywrap
%{
    /*
    * You will need to comment this line in lab5.
    */
    #define ONLY_FOR_LEX
    
    #ifdef ONLY_FOR_LEX

    #include<vector>
    struct depth_with_tag{
        int depth_num;
        int tag;
        int fa;
    };
    long unsigned int previous_depth = 0;//当前所处{}层数
    int previous_tag = 0;//单增常量，当前要分配的tag号，每个语句块有一个唯一tag

    //记录{}的层结构，symbol_table_depth_list[i]代表第i层深的{}，
    //symbol_table_depth_list[i].depth_num代表该语句块是当前层的第几个
    //symbol_table_depth_list[i].tag代表当前语句块的标识符
    //symbol_table_depth_list[i].fa代表当前语句块的父语句块的tag
    //最外层tag为0，其fa为0
    std::vector<depth_with_tag> symbol_table_depth_list={(depth_with_tag){0,previous_tag++,0}};

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

    void DEBUG_FOR_LAB4(std::string s){
        std::string DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\n";
        fputs(DEBUG_INFO.c_str(), yyout);
    }
    #endif
%}

DECIMIAL ([1-9][0-9]*|0)
ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
%x BLOCKCOMMENT

%%

"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("INT\tint");
    #else
        return INT;
    #endif
}
"void" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("VOID\tvoid");
    #else
        return VOID;
    #endif 
}
"if" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("IF\tif");
    #else
        return IF;
    #endif
};
"else" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ELSE\telse");
    #else
        return ELSE;
    #endif
};
"return" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RETURN\treturn");
    #else
        return RETURN;
    #endif
}

"=" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ASSIGN\t=");
    #else
        return ASSIGN;
    #endif
}
"<" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LESS\t<");
    #else
        return LESS;
    #endif
}
"+" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("ADD\t+");
    #else
        return ADD;
    #endif
}
";" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("SEMICOLON\t;");
    #else
        return SEMICOLON;
    #endif
}
"(" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("LPAREN\t(");
    #else
        return LPAREN;
    #endif
}
")" {
    #ifdef ONLY_FOR_LEX
        DEBUG_FOR_LAB4("RPAREN\t)");
    #else
    return RPAREN;
    #endif
}
"{" {
    #ifdef ONLY_FOR_LEX
        previous_depth++;
        if(previous_depth>=symbol_table_depth_list.size()){
            symbol_table_depth_list.push_back(
                (depth_with_tag){0,previous_tag++,symbol_table_depth_list[previous_depth-1].tag});
        }
        else{
            symbol_table_depth_list[previous_depth].depth_num++;
            symbol_table_depth_list[previous_depth].tag = previous_tag++;
            symbol_table_depth_list[previous_depth].fa = symbol_table_depth_list[previous_depth-1].tag;
        }
        DEBUG_FOR_LAB4("LBRACE\t{");
    #else
        return LBRACE;
    #endif
}
"}" {
    #ifdef ONLY_FOR_LEX
        previous_depth--;
        DEBUG_FOR_LAB4("RBRACE\t}");
    #else
        return RBRACE;
    #endif
}
{ID} {
    #ifdef ONLY_FOR_LEX 
        char str[100];
        sprintf(str,"ID\t%s\t%d\t%d",yytext,symbol_table_depth_list[previous_depth].tag,symbol_table_depth_list[previous_depth].fa);
        DEBUG_FOR_LAB4(str);
    #else
        return ID;
    #endif
}

{EOL} yylineno++;
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
