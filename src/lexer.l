%option noyywrap
%{
    #define YY_NO_UNPUT
    #define YY_NO_INPUT

    #include "parser.h"
    #include <iostream>
    #include <fstream>
    using namespace std;

    extern FILE *yyin; 
    extern FILE *yyout;
    extern bool dump_tokens;

    void DEBUG_FOR_LAB4(std::string s){
        std::string DEBUG_INFO = "[DEBUG LAB4]: \t" + s + "\n";
        fputs(DEBUG_INFO.c_str(), yyout);
    }
%}

ADD "+"
SUB "-"
DECIMIAL ([1-9][0-9]*|0)
OCTAL 0([1-7][0-7]*|0)
HEX (0x|0X)([1-9a-fA-F][0-9a-fA-F]*|0)

FLOAT_DECIMAL ((([0-9]*[.][0-9]*([eE][+-]?[0-9]+)?)|([0-9]+[eE][+-]?[0-9]+))[fLlL]?)
FLOAT_HEXADECIMAL (0[xX](([0-9A-Fa-f]*[.][0-9A-Fa-f]*([pP][+-]?[0-9]+)?)|([0-9A-Fa-f]+[pP][+-]?[0-9]+))[fLlL]?)

ID [[:alpha:]_][[:alpha:][:digit:]_]*
EOL (\r\n|\n|\r)
WHITE [\t ]
commentbegin "/*"
commentelement1 .|\n
commentelement2 (.)*\n
commentend "*/"
%x COMMENT1
%x COMMENT2
%x BLOCKCOMMENT

%%
"int" {
    /*
    * Questions: 
    *   Q1: Why we need to return INT in further labs?
    *   Q2: What is "INT" actually?
    */
    if(dump_tokens)
        DEBUG_FOR_LAB4("INT\tint");
    return INT;
}
"float" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("FLOAT\tfloat");
    return FLOAT;
}
"bool" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("BOOL\tbool");
    return BOOL;
}
"void" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("VOID\tvoid");
    return VOID;
}
"if" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("IF\tif");
    return IF;
};
"then" {
    if(dump_tokens)
            DEBUG_FOR_LAB4("THEN\telse");
        return THEN;
}
"else" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ELSE\telse");
    return ELSE;
}
"while" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ELSE\telse");
    return WHILE;
}
"return" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RETURN\treturn");
    return RETURN;
}
"const" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONST\treturn");
    return CONST;
}

{ADD} {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ADD\t+");
    //std::cout<<"addhere!"<<endl;
    return ADD;
}
{SUB} {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SUB\t+");
    return SUB;
}
"*" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MUL\t+");
    return MUL;
}
"/" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("DIV\t+");
    return DIV;
}
"%" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MOD\t+");
    return MOD;
}
"||" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("OR\t+");
    return OR;
}
"&&" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("AND\t+");
    return AND;
}
"<" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LESS\t<");
    return LESS;
}
">" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MORE\t<");
    return MORE;
}
"==" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("EQUAL\t<");
    return EQUAL;
}
"!=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOT_EQUAL\t<");
    return NOT_EQUAL;
}
"<=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LESS_E\t<");
    return LESS_E;
}
">=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("MORE_E\t<");
    return MORE_E;
}

"=" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("ASSIGN\t=");
    return ASSIGN;
}

"!" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("NOT\t=");
    return NOT;
}
"++" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("AADD\t=");
    return AADD;
}
"--" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SSUB\t=");
    return SSUB;
}


"," {
    if(dump_tokens)
        DEBUG_FOR_LAB4("COMMA\t;");
    return COMMA;
}

";" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("SEMICOLON\t;");
    return SEMICOLON;
}

"(" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LPAREN\t(");
    return LPAREN;
}
")" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RPAREN\t)");
    return RPAREN;
}
"[" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LBRACKET\t(");
    return LBRACKET;
}
"]" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RBRACKET\t)");
    return RBRACKET;
}
"{" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("LBRACE\t{");
    return LBRACE;
}
"}" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("RBRACE\t}");
    return RBRACE;
}

"break" {
    if(dump_tokens)
        DEBUG_FOR_LAB4("BREAK\t}");
    return BREAK;
}
"continue"  {
    if(dump_tokens)
        DEBUG_FOR_LAB4("CONTINUE\t}");
    return CONTINUE;
}

"putint" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    paramTypes.push_back(TypeSystem::intType);//有一个int参数
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}

"getint" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    Type* funcType = new FunctionType(TypeSystem::intType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}

"putfloat" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    paramTypes.push_back(TypeSystem::floatType);
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}

"getfloat" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    Type* funcType = new FunctionType(TypeSystem::floatType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}


"putch" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    paramTypes.push_back(TypeSystem::intType);//有一个int参数
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}
"getch" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    Type* funcType = new FunctionType(TypeSystem::intType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);    
        st->install(yytext, se);
    }
    return ID;
}

"putarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    paramTypes.push_back(TypeSystem::intType);
    vector<int> dims;
    dims.push_back(0);
    ArrayType* arr = new ArrayType(TypeSystem::intType, dims);
    paramTypes.push_back(arr);
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}

"getarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    vector<int> dims;
    dims.push_back(0);
    ArrayType* arr = new ArrayType(TypeSystem::intType, dims);
    paramTypes.push_back(arr);
    Type* funcType = new FunctionType(TypeSystem::intType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}
"putfarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    paramTypes.push_back(TypeSystem::intType);
    vector<int> dims;
    dims.push_back(0);
    ArrayType* arr = new ArrayType(TypeSystem::floatType, dims);
    paramTypes.push_back(arr);
    Type* funcType = new FunctionType(TypeSystem::voidType, paramTypes);
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}

"getfarray" {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    std::vector<Type*> paramTypes;
    vector<int> dims;
    dims.push_back(0);
    ArrayType* arr = new ArrayType(TypeSystem::intType, dims);
    paramTypes.push_back(arr);
    Type* funcType = new FunctionType(TypeSystem::intType, paramTypes); 
    SymbolTable* st = identifiers;
    while(st->getPrev())
        st = st->getPrev();
    if(!st->lookup(yytext)){
        SymbolEntry* se = new IdentifierSymbolEntry(funcType, yytext, st->getLevel(), true);
        st->install(yytext, se);
    }
    return ID;
}



{DECIMIAL}  {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = atoi(yytext);
    //std::cout<<yytext<<endl;
    return INTEGER;
}
{OCTAL} {
    int val;
    sscanf(yytext, "%o", &val);
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = val;
    //std::cout<<yytext<<endl;
    return INTEGER;
}
{HEX}   {
    int val;
    sscanf(yytext, "%x", &val);
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = val;
    //std::cout<<yytext<<endl;
    return INTEGER;
}

{FLOAT_DECIMAL} {
    float temp;
    sscanf(yytext, "%f", &temp);
    if (dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = (double)temp;
    return FLOATING_POINT;
}

{FLOAT_HEXADECIMAL} {
    float temp;
    sscanf(yytext, "%f", &temp);
    if (dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    yylval.itype = (double)temp;
    return FLOATING_POINT;
}


{ID} {
    if(dump_tokens)
        DEBUG_FOR_LAB4(yytext);
    char *lexeme;
    lexeme = new char[strlen(yytext) + 1];
    strcpy(lexeme, yytext);
    yylval.strtype = lexeme;
    return ID;
}

{commentbegin} {BEGIN COMMENT1;}
<COMMENT1>{commentelement1} {}
<COMMENT1>{commentend} {BEGIN INITIAL;}
"//"    {
    BEGIN COMMENT2;
}
<COMMENT2>{commentelement2} {BEGIN INITIAL;}


{EOL} {
    yylineno++;
    //do nothing!!
}

{WHITE}
%%
