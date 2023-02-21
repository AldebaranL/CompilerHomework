%code top{
    #include <iostream>
    #include <assert.h>
    #include <map>
    #include <vector>
    #include "parser.h"
    using namespace std;
    extern Ast ast;
    int yylex();
    int yyerror( char const * );

    map<std::string, ExprNode*> idlist;
    map<std::string, vector<ExprNode*>> arraylist;
    vector<string> curr_array;
    std::vector<Type*> paramtypes;
    vector<SymbolEntry*> sesymlist;
    std::vector<std::string> paramsymbols;
    map<int, vector<ExprNode*>> globle_tmp_paramcalls;
    int call_level=0;
    bool isret=false;
    
    bool alarm=false;
    extern int yylineno;
    string curr_func="";

    map<int,vector<BreakStmt*>> breakList;
    map<int,vector<ContinueStmt*>> continueList;
    int while_level=0;

    int l_br=-1;
    vector<int>dims;
    vector<int>dc_record;
    map<string, vector<ExprNode*>> init_;
    string init_array;
}

%code requires {
    #include "Ast.h"
    #include "SymbolTable.h"
    #include "Type.h"
}

%union {
    double itype;
    char* strtype;
    StmtNode* stmttype;
    ExprNode* exprtype;
    Type* type;
}

%start Program
%token <strtype> ID 
%token <itype> INTEGER FLOATING_POINT
%token IF ELSE WHILE
%token INT FLOAT BOOL VOID
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE SEMICOLON COMMA
%token NOT AADD SSUB
%token MUL DIV MOD ADD SUB OR AND LESS MORE EQUAL MORE_E LESS_E NOT_EQUAL ASSIGN
%token RETURN CONST
%token BREAK CONTINUE


%nterm <stmttype> Stmts Stmt AssignStmt BlockStmt IfStmt WhileStmt AssignExpr ReturnStmt DeclStmt ExprStmt BreakStmt ContinueStmt FuncDef
%nterm <exprtype> Cond Exp MulExp AddExp LOrExp PrimaryExp LVal EqlExp RelExp LAndExp preSinExp sufSinExp FuncCall
%nterm <type> Type
%nterm IDList

%precedence THEN
%precedence ELSE
%%
Program
    : Stmts {
        //cout<<"Stmts"<<endl;
        ast.setRoot($1);
        //ast.output();
    }
    ;
Stmts
    : Stmt {$$=$1;}
    | Stmts Stmt{
        //cout<<"Stmts1"<<endl;
        $$ = new SeqNode($1, $2);
        
    }
    ;
Stmt
    : AssignStmt {$$=$1;}
    | BlockStmt {$$=$1;}
    | IfStmt {$$=$1;}
    | WhileStmt {$$=$1;}
    | ReturnStmt {$$=$1;}
    | DeclStmt {$$=$1;}
    | FuncDef {$$=$1;}
    | ExprStmt{$$ = $1;}
    | SEMICOLON {$$ = new EmptyStmt();}
    | BreakStmt{$$=$1;}
    | ContinueStmt{$$=$1;}
    ;

BreakStmt
    :
    BREAK SEMICOLON{
        BreakStmt* bs = new BreakStmt();
        breakList[while_level].push_back(bs);
        $$ = bs;
    }
    ;

ContinueStmt
    :
    CONTINUE SEMICOLON{
        ContinueStmt* cs = new ContinueStmt();
        continueList[while_level].push_back(cs);
        $$ = cs;
    }
    ;


ARRAY
    :
    ID LBRACKET RBRACKET{
        curr_array.push_back($1);
        //vector<ExprNode*> emp;
        //arraylist[curr_array.back()]=emp;
        arraylist[curr_array.back()].push_back(nullptr);
    }
    |
    ID LBRACKET Exp RBRACKET
    {
        //arraylist.erase($1);
        cout<<"hi array----"<<$1<<endl;
        curr_array.push_back($1);
        arraylist[curr_array.back()].push_back($3);
    }
    |
    ARRAY LBRACKET Exp RBRACKET
    {
        // cout<<"hi array----"<<curr_array.size()<<endl;
        //Exp可能调用另外的ARRAY，导致错误。
        if($3->getSymPtr()->getType()->isArray()||$3->get_arrflag()){
            cout<<"hi array----"<<curr_array.back()<<endl;
            curr_array.erase(curr_array.end()-1);
            cout<<"!!!!!!!!!!!!!!!!!!!!"<<endl;
            cout<<"hi array----"<<curr_array.back()<<endl;
        }
        arraylist[curr_array.back()].push_back($3);
    }
    ;

LVal
    : 
    ID {
        SymbolEntry *se;
        se = identifiers->lookup($1);

        //类型检查：使用前未声明
        if(se == nullptr)
        {
            cout<<"line: "<<yylineno<<endl;
            fprintf(stderr, "identifier \"%s\" is undefined\n", (char*)$1);
            // delete [](char*)$1;
            // cout<<se->toStr()<<endl;
            // assert(se != nullptr);
                        
            se = new IdentifierSymbolEntry(TypeSystem::intType, $1, identifiers->getLevel());
            se->set_value(idlist[$1]->getSymPtr()->get_value());        
            identifiers->install($1, se);
        }

        if(se->getType()->isArray()){
              cout<<"ID isArray!!  "<<$1<<endl;
            //$$ = (ExprNode*)(((IdentifierSymbolEntry*)se)->getParent());
            vector<ExprNode*> emp;
            $$=new ArrayItem(se, emp, true);
       }
        else{
            $$ = new Id(se);
        }
        delete []$1;
    }
    |
    ARRAY {
        cout<<"curr_array.back():"<<curr_array.back()<<endl;
        SymbolEntry *array_se;
        array_se = identifiers->lookup(curr_array.back());
        
        vector<ExprNode*> offsets=arraylist[curr_array.back()];
        $$ = new ArrayItem(array_se, offsets);
        $$->set_arrflag(true);
        ((IdentifierSymbolEntry*)array_se)->setParent($$);
        cout<<"hi"<<endl;
        //一定要！因为可能有重复！
        arraylist.erase(curr_array.back());
    }
    ;


ExprStmt
    :
    Exp SEMICOLON{
        $$ = new ExprStmt($1);
        //std::cout<<"ExprStmt"<<endl;
    }
    ;
AssignStmt
    :
    AssignExpr SEMICOLON {
        $$ = $1;
    }
    ;
AssignExpr
    :
    LVal ASSIGN{alarm=false;} Exp {
        //类型检查：函数返回值为void（空）
        if(alarm){
            // cout<<"error"<<endl;
            cout<<"lineno: "<<yylineno+1<<endl;
            alarm=false;                          
            fprintf(stderr, "assign error: operation's type should not be <void>.\n");
            //assert(alarm);
        }
        SymbolEntry *se = $1->getSymPtr();
        double val=$4->getSymPtr()->get_value();
        if(se->getType()->isInt())
            val=(int)val;
        $1->getSymPtr()->set_value(val);
        
        $$ = new AssignStmt($1, $4);
    }
    ;
BlockStmt
    :   LBRACE 
        {identifiers = new SymbolTable(identifiers);} 
        Stmts RBRACE 
        {
            $$ = new CompoundStmt($3);
            SymbolTable *top = identifiers;
            identifiers = identifiers->getPrev();
            delete top;
        }
    |
    LBRACE RBRACE
    {
        $$ = new EmptyStmt();
    }
    ;
IfStmt
    : IF LPAREN Cond RPAREN Stmt %prec THEN {
        $$ = new IfStmt($3, $5);
    }
    | IF LPAREN Cond RPAREN Stmt ELSE Stmt {
        $$ = new IfElseStmt($3, $5, $7);
    }
    ;
WhileStmt
    : 
    WHILE {

        while_level++;
        breakList[while_level].clear();
        continueList[while_level].clear();
        
    }
    LPAREN Cond RPAREN Stmt{
        $$ = new WhileStmt($4, $6);
        cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<$$<<endl;
        for(int i=0;i<(int)(breakList[while_level].size());i++){
            cout<<"break??"<<endl;
            breakList[while_level][i]->setParent($$);
        }
        for(int i=0;i<(int)(continueList[while_level].size());i++){
            continueList[while_level][i]->setParent($$);
        }

        // breakList.clear();
        // continueList.clear();

        while_level--;
    }   
    ;

ReturnStmt
    :
    RETURN SEMICOLON{

        $$ = new ReturnStmt();
    }
    |
    RETURN Exp SEMICOLON{
        isret=true;

        $$ = new ReturnStmt($2);
    }
    ;
Exp
    :
    LOrExp {
        $$ = $1;//std::cout<<"LOrExp"<<endl;
    }
    ;
Cond
    :
    Exp 
    {
        //类型检查：int->bool
        if($1->getSymPtr()->getType()->toStr()=="i32"){
            cout<<"line:"<<yylineno+1<<endl;
            cout<<"cond expr should be bool!"<<endl;
        }

        $$ = $1;
    }
    ;

PrimaryExp
    :
    LVal {

        $$ = $1;
    }
    |
    INTEGER {
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, $1);
        se->set_value($1);
        $$ = new Constant(se);
       // std::cout<<"INTEGER"<<$1<<endl;
    }
    |
    FLOATING_POINT  {
        cout<<$1<<endl;
        SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::floatType, $1);
        se->set_value($1);
        cout<<se->get_value()<<endl;
        $$ = new Constant(se);
        cout<<"FLOATING_POINT"<<endl;
    }
    |
    LPAREN Exp RPAREN {
        $$ = $2;
    }
    |
    FuncCall {
        //类型检查：返回值是否为void
        SymbolEntry* se = ((FunctionCall*)$1)->getSymPtr();
        if(((FunctionType*)(se->getType()))->getRetType()->isVoid()){
            // cout<<"line:"<<yylineno+1<<endl;
            // cout<<"Exp should not be void!"<<endl;
            alarm=true;
        }

        $$ = $1;
    }
    ;

sufSinExp
    :
    PrimaryExp{
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {               
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;
        //std::cout<<"PrimaryExp"<<endl;
    }
    |
    sufSinExp AADD {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isInt())
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        else if($1->getSymPtr()->getType()->isFloat())
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        
        $$ = new sufSingleExpr(se, $1, sufSingleExpr::AADD);
        $$->set_arrflag($1->get_arrflag());
    }
    | 
    sufSinExp SSUB {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isInt())
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        else if($1->getSymPtr()->getType()->isFloat())
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());     
        $$->set_arrflag($1->get_arrflag());
    }
    
    ;
preSinExp
    :
    sufSinExp {
        $$ = $1;
        //std::cout<<"sufSinExp"<<endl;
    }
    |
    AADD preSinExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$2->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($2->getSymPtr()->getType()->isInt())
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        else if($2->getSymPtr()->getType()->isFloat())
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());       
        $$ = new preSingleExpr(se, preSingleExpr::AADD, $2);
        $$->set_arrflag($2->get_arrflag());
    }
    | 
    SSUB preSinExp{
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$2->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($2->getSymPtr()->getType()->isInt())
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        else if($2->getSymPtr()->getType()->isFloat())
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());      
        $$ = new preSingleExpr(se, preSingleExpr::SSUB, $2);
        $$->set_arrflag($2->get_arrflag());
    }
    |
    NOT preSinExp{
        //是不是应该存布尔？
        //类型检查：int->bool
        if(!$2->getSymPtr()->isTemporary())
            cout<<"wrong int in line:"<<yylineno+1<<endl;

        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new preSingleExpr(se, preSingleExpr::NOT, $2);
    }
    |
    ADD preSinExp {
        //+出现不对表达式的值产生影响 直接忽略
        // SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
        // $$ = new preSingleExpr(se, preSingleExpr::ADD, $2);
        $$=$2;
        $$->set_arrflag($2->get_arrflag());
    }
    |
    SUB preSinExp{
        if($2->getSymPtr()->isConstant()){
            SymbolEntry *se = new ConstantSymbolEntry(TypeSystem::intType, -1*((ConstantSymbolEntry*)($2->getSymPtr()))->getValue());
            se->set_value(-1*($2->getSymPtr()->get_value()));
            $$ = new Constant(se);$$->set_arrflag($2->get_arrflag());
        }
        else{
            SymbolEntry *se;
            if($2->getSymPtr()->getType()->isInt())
                se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());
            else if($2->getSymPtr()->getType()->isFloat())
                se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());       
            $$ = new preSingleExpr(se, preSingleExpr::SUB, $2);
            $$->set_arrflag($2->get_arrflag());
        }
    }
    ; 
MulExp
    :
    preSinExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;
        //std::cout<<"MulExp"<<endl;
        }
    |
    MulExp MUL preSinExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {               
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isFloat()||$3->getSymPtr()->getType()->isFloat()
            ||($1->getSymPtr()->getType()->isFunc()&&((FunctionType*)($1->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($3->getSymPtr()->getType()->isFunc()&&((FunctionType*)($3->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($1->getSymPtr()->getType()->isArray()&&((ArrayType*)($1->getSymPtr()->getType()))->gettype()->isFloat())
            ||($3->getSymPtr()->getType()->isArray()&&((ArrayType*)($3->getSymPtr()->getType()))->gettype()->isFloat()))
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());       

        double val1=$1->getSymPtr()->get_value(), val2=$3->getSymPtr()->get_value();
        if($1->getSymPtr()->getType()->isInt()) val1=(int)val1;
        if($3->getSymPtr()->getType()->isInt()) val2=(int)val2;

        double res=(val1)*(val2);
        se->set_value((float)res);
        
        $$ = new BinaryExpr(se, BinaryExpr::MUL, $1, $3);
        $$->set_arrflag($1->get_arrflag()||$3->get_arrflag());
    }
    |
    MulExp DIV preSinExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isFloat()||$3->getSymPtr()->getType()->isFloat()
            ||($1->getSymPtr()->getType()->isFunc()&&((FunctionType*)($1->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($3->getSymPtr()->getType()->isFunc()&&((FunctionType*)($3->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($1->getSymPtr()->getType()->isArray()&&((ArrayType*)($1->getSymPtr()->getType()))->gettype()->isFloat())
            ||($3->getSymPtr()->getType()->isArray()&&((ArrayType*)($3->getSymPtr()->getType()))->gettype()->isFloat()))
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());       

        if($3->getSymPtr()->get_value()!=0){
            double val1=$1->getSymPtr()->get_value(), val2=$3->getSymPtr()->get_value();
            if($1->getSymPtr()->getType()->isInt()) val1=(int)val1;
            if($3->getSymPtr()->getType()->isInt()) val2=(int)val2;

            double res=(val1)/(val2);
            se->set_value((float)res);
        }
        $$ = new BinaryExpr(se, BinaryExpr::DIV, $1, $3);
        $$->set_arrflag($1->get_arrflag()||$3->get_arrflag());
    }
    |
    MulExp MOD preSinExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isFloat()||$3->getSymPtr()->getType()->isFloat()
            ||($1->getSymPtr()->getType()->isFunc()&&((FunctionType*)($1->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($3->getSymPtr()->getType()->isFunc()&&((FunctionType*)($3->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($1->getSymPtr()->getType()->isArray()&&((ArrayType*)($1->getSymPtr()->getType()))->gettype()->isFloat())
            ||($3->getSymPtr()->getType()->isArray()&&((ArrayType*)($3->getSymPtr()->getType()))->gettype()->isFloat()))
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());       

        if($3->getSymPtr()->get_value()!=0){
            int t1=($1->getSymPtr()->get_value());
            int t2=($3->getSymPtr()->get_value());
            int res=t1%t2;
            se->set_value(res);
        }
        $$ = new BinaryExpr(se, BinaryExpr::MOD, $1, $3);
        $$->set_arrflag($1->get_arrflag()||$3->get_arrflag());
    }
    ;
AddExp
    :
    MulExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;//std::cout<<"add2"<<endl;
    }
    |
    AddExp ADD MulExp
    {
       // std::cout<<"add1"<<endl;
       SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isFloat()||$3->getSymPtr()->getType()->isFloat()
            ||($1->getSymPtr()->getType()->isFunc()&&((FunctionType*)($1->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($3->getSymPtr()->getType()->isFunc()&&((FunctionType*)($3->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($1->getSymPtr()->getType()->isArray()&&((ArrayType*)($1->getSymPtr()->getType()))->gettype()->isFloat())
            ||($3->getSymPtr()->getType()->isArray()&&((ArrayType*)($3->getSymPtr()->getType()))->gettype()->isFloat()))
            
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());       

        double val1=$1->getSymPtr()->get_value(), val2=$3->getSymPtr()->get_value();
        if($1->getSymPtr()->getType()->isInt()) val1=(int)val1;
        if($3->getSymPtr()->getType()->isInt()) val2=(int)val2;

        double res=(val1)+(val2);
        se->set_value(res);

        $$ = new BinaryExpr(se, BinaryExpr::ADD, $1, $3);
        $$->set_arrflag($1->get_arrflag()||$3->get_arrflag());
    }
    |
    AddExp SUB MulExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se;
        if($1->getSymPtr()->getType()->isFloat()||$3->getSymPtr()->getType()->isFloat()
            ||($1->getSymPtr()->getType()->isFunc()&&((FunctionType*)($1->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($3->getSymPtr()->getType()->isFunc()&&((FunctionType*)($3->getSymPtr()->getType()))->getRetType()->isFloat())
            ||($1->getSymPtr()->getType()->isArray()&&((ArrayType*)($1->getSymPtr()->getType()))->gettype()->isFloat())
            ||($3->getSymPtr()->getType()->isArray()&&((ArrayType*)($3->getSymPtr()->getType()))->gettype()->isFloat()))
            se = new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel());
        else
            se = new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel());      

        double val1=$1->getSymPtr()->get_value(), val2=$3->getSymPtr()->get_value();
        if($1->getSymPtr()->getType()->isInt()) val1=(int)val1;
        if($3->getSymPtr()->getType()->isInt()) val2=(int)val2;

        double res=(val1)-(val2);
        se->set_value(res);
        
        $$ = new BinaryExpr(se, BinaryExpr::SUB, $1, $3);
        $$->set_arrflag($1->get_arrflag()||$3->get_arrflag());
    }
    ;
RelExp
    :
    AddExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;//std::cout<<"AddExp"<<endl;
    }
    |
    RelExp LESS AddExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS, $1, $3);
    }
    |
    RelExp MORE AddExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MORE, $1, $3);
    }
    |
    RelExp LESS_E AddExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::LESS_E, $1, $3);
    }
    |
    RelExp MORE_E AddExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::MORE_E, $1, $3);
    }
    ;
EqlExp  
    :
    RelExp{
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;//std::cout<<"RelExp"<<endl;
    }
    |
    EqlExp EQUAL RelExp{
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::EQUAL, $1, $3);
    }
    |
    EqlExp NOT_EQUAL RelExp{
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::NOT_EQUAL, $1, $3);
    }
    ;
LAndExp
    :
    EqlExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        
        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;//std::cout<<"RelExp"<<endl;
    }
    |
    LAndExp AND EqlExp
    {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        
        //类型检查：int->bool
        if($3->getSymPtr()->getType()->toStr()=="i32"){
            cout<<"line:"<<yylineno+2<<endl;
            cout<<"cond expr should be bool!"<<endl;
        }

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        
        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::AND, $1, $3);
    }
    ;
LOrExp
    :
    LAndExp {
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$1->getSymPtr();
        
        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {
                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }

        $$ = $1;//std::cout<<"LAndExp"<<endl;
    }
    |
    LOrExp OR LAndExp
    {
        cout<<"OR!"<<endl;
        //类型检查：函数返回值为void（空）
        SymbolEntry* se1=$3->getSymPtr();
        
        //类型检查：int->bool
        if($3->getSymPtr()->getType()->toStr()=="i32"){
            cout<<"line:"<<yylineno+2<<endl;
            cout<<"cond expr should be bool!"<<endl;
        }

        if(((se1->getType()))->isFunc()){
            if(((FunctionType*)(se1->getType()))->getRetType()->isVoid())
            {                
                alarm=true; 
                //fprintf(stderr, "error: operation's type should not be <void>.");
                //assert(!((FunctionType*)(se1->getType()))->getRetType()->isVoid());
            }
        }
        

        SymbolEntry *se = new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel());
        $$ = new BinaryExpr(se, BinaryExpr::OR, $1, $3);
    }
    ;
Type
    : INT {
        $$ = TypeSystem::intType;
    }
    |
    FLOAT {
        $$ = TypeSystem::floatType;
    }
    |
    BOOL{
        $$ = TypeSystem::boolType;
    }
    | VOID {
        $$ = TypeSystem::voidType;
    }
    | CONST Type {
        if($2->isInt())
            $$ = TypeSystem::intType_const;
        else if($2->isFloat())
            $$ = TypeSystem::floatType_const;
    }
    ;
BRACEUnit
    :
    Exp{
        //计算位置
        int pos=0;
        cout<<init_array<<": ";
        int size=dc_record.size();
        int total=1;
        for(auto & dim:dims){
            total*=dim;
        }
        for(int i=0;i<size-1;i++){
            total/=dims[i];
            pos+=dc_record[i]*total;
        }
        for(int i=0;i<size;i++){
            cout<<"["<<dc_record[i]<<"]";
        }
        pos+=dc_record[size-1];
        cout<<endl;
        //赋值！
        cout<<"pos:"<<pos<<endl;
        init_[init_array][pos]=$1;
        cout<<"haha"<<endl;
        //--------------------------------------------------
        cout<<"dc_record.back(): "<<dc_record.back()<<endl;
        cout<<"dims.back(): "<<dims.back()<<endl;
        if(dc_record.back()+1==dims.back()){
            int i=dc_record.size()-1;
            //进位
            while(i>=0){
                dc_record[i]=0;
                if(i>0){
                    if(dc_record[i-1]+1==dims[i-1]){
                        i--;
                        continue;
                    }
                    else{
                        dc_record[i-1]++;
                        break;
                    }
                }
                else{
                    break;
                }
            }
            // if(i==l_br){
            //     if(i>0){
            //         dc_record[i-1]--;
            //     }
            // }
                
        }
        else
            dc_record.back()++;
    }
    |
    LBRACE{
        //空
        if(l_br==-1){
            std::map <std::string, vector<ExprNode*>>::iterator it1=arraylist.begin();
            //初始化dims和dc_record
            //一定要while循环 因为里面有可能有很多个数组id（但是未初始化的目标数组只有一个）
            int total=1;//计算一共有多少元素
            while(it1!=arraylist.end()){
                if(!identifiers->lookup(it1->first)){
                    init_array=it1->first;
                    Type* arrayType;
                    //vector<int>dims;
                    dims.clear();
                    dc_record.clear();
                    for(auto & iter:it1->second){
                        total*=((ConstantSymbolEntry*)(iter->getSymPtr()))->getValue();
                        dims.push_back(((ConstantSymbolEntry*)(iter->getSymPtr()))->getValue());
                        dc_record.push_back(0);
                    }
                }
                it1++;
            }
            // cout<<"dims:";
            // for(int i=0;i<dims.size();i++){
            //     cout<<dims[i]<<' ';
            // }
            // cout<<endl;
            //初始化init_
            init_[init_array].clear();
            while(total>0){
                init_[init_array].push_back(nullptr);
                total--;
            }
        }
        // else{
        //     //l_br--;
        // }
        l_br++;
    } RBRACE{
        l_br--;
        if(l_br>=0){
            dc_record[l_br]++;
        }
        //cout<<"rbr"<<l_br<<" "<<endl;
        for(int i=l_br+1;i<dc_record.size();i++){
            dc_record[i]=0;
        }
        if(l_br==-1){
            dc_record.clear();
        }
    }
    |
    LBRACE {
        //最开始
        if(l_br==-1){
            std::map <std::string, vector<ExprNode*>>::iterator it1=arraylist.begin();
            //初始化dims和dc_record
            //一定要while循环 因为里面有可能有很多个数组id（但是未初始化的目标数组只有一个）
            int total=1;//计算一共有多少元素
            while(it1!=arraylist.end()){
                if(!identifiers->lookup(it1->first)||((IdentifierSymbolEntry*)(identifiers->lookup(it1->first)))->getScope()!=identifiers->getLevel()){
                    init_array=it1->first;
                    Type* arrayType;
                    //vector<int>dims;
                    dims.clear();
                    dc_record.clear();
                    for(auto & iter:it1->second){
                        total*=iter->getSymPtr()->get_value();
                        dims.push_back(iter->getSymPtr()->get_value());
                        dc_record.push_back(0);
                    }
                    cout<<"total:"<<total<<endl;
                }
                else{
                    cout<<"scope:"<<((IdentifierSymbolEntry*)(identifiers->lookup(it1->first)))->getScope()<<endl;
                    cout<<"now scope:"<<identifiers->getLevel()<<endl;
                }
                it1++;
            }
            cout<<"------------init_array:"<<init_array<<endl;
            // cout<<"dims:";
            // for(int i=0;i<dims.size();i++){
            //     cout<<dims[i]<<' ';
            // }
            // cout<<endl;
            //初始化init_
            init_[init_array].clear();
            while(total>0){
                init_[init_array].push_back(nullptr);
                total--;
            }
            cout<<"init_ size:"<<init_[init_array].size()<<endl;
        }
        l_br++;
    }
    BRACEList RBRACE
    {
        l_br--;
        cout<<"dc_record:";
        for(auto& r:dc_record){
            cout<<r<<", ";
        }
        cout<<endl;
        if(l_br>=0){
            bool f=false;
            for(int x=l_br+1;x<dc_record.size();x++){
                if(dc_record[x]!=0){
                    f=true;
                    break;
                }
            }
            if(f){
                dc_record[l_br]++;
            }
        }
        //cout<<"rbr"<<l_br<<" "<<endl;
        for(int i=l_br+1;i<dc_record.size();i++){
            dc_record[i]=0;
        }
        if(l_br==-1){
            dc_record.clear();
        }
    }
    ;
BRACEList
    :
    BRACEUnit{
        //dc_record.back()++;
    }
    |
    BRACEList COMMA BRACEUnit{
        // cout<<"comma lbr:"<<l_br<<endl;
        // if(dc_record.back()+1==dims.back()){
        //     // dc_record[l_br-1]++;
        //     // dc_record.back()=0;
        //     int i=dc_record.size()-1;
        //     //进位
        //     while(i>=0){
        //         dc_record[i]=0;
        //         if(dc_record[i-1]+1==dims[i-1]){
        //             i--;
        //             continue;
        //         }
        //         else{
        //             dc_record[i-1]++;
        //             break;
        //         }
        //         i--;
        //     }
        //     if(i==l_br)
        //         dc_record[i-1]--;
        // }
        // else
        //     dc_record.back()++;
    }
    ;
IDList
    :
    ID COMMA{
        idlist[$1]=nullptr;
    }
    |
    ID ASSIGN Exp COMMA{
        idlist[$1]=$3;
    }
    |
    ARRAY COMMA{

    }
    |
    ARRAY ASSIGN BRACEUnit COMMA{

    }
    |
    IDList ID COMMA{
        idlist[$2]=nullptr;
    }
    |
    IDList ID ASSIGN Exp COMMA{
        idlist[$2]=$4;
    }
    |
    IDList ARRAY COMMA{
        
    }
    |
    IDList ARRAY ASSIGN BRACEUnit COMMA{
        
    }
    |
    IDList ID{
        //结束
        idlist[$2]=nullptr;
    }
    |
    IDList ID ASSIGN Exp{
        //结束
        idlist[$2]=$4;
    }
    |
    IDList ARRAY {
        
    }
    |
    IDList ARRAY ASSIGN BRACEUnit {
        
    }
    ;
DeclStmt
    :
    Type ID SEMICOLON {
        // cout<<"hhh"<<endl;
        SymbolEntry *se;
        //类型检查：是否重复声明
        se = identifiers->lookup($2);
        if(se != nullptr)
        {
            cout<<"line:"<<yylineno+1<<endl;
            fprintf(stderr, "identifier \"%s\" is redefined\n", (char*)$2);
            //assert(se == nullptr);
        }

        se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        
        identifiers->install($2, se);
        DeclStmt* tmp = new DeclStmt();
        tmp->insert(new Id(se));
        $$ = tmp; 
        delete []$2;
    }
    |
    Type ID ASSIGN Exp SEMICOLON{

        //类型检查：函数返回值为void（空）
        if(alarm){
            // cout<<"error"<<endl;
            cout<<"lineno: "<<yylineno+1<<endl;
            alarm=false;                          
            fprintf(stderr, "assign error: operation's type should not be <void>.\n");
            //assert(alarm);
        }

        //应该加一个value值 在entry里也要加
        SymbolEntry *se;

        //类型检查：是否重复声明
        se = identifiers->lookup($2);
        if(se != nullptr)
        {
            cout<<"line:"<<yylineno+1<<endl;
            fprintf(stderr, "identifier \"%s\" is redefined\n", (char*)$2);
            //assert(se == nullptr);
        }

        se = new IdentifierSymbolEntry($1, $2, identifiers->getLevel());
        //if($1->isConst()){
        double val=$4->getSymPtr()->get_value();
        if(se->getType()->isInt())
            val=(int)val;
        se->set_value(val);
        //}

        identifiers->install($2, se);
        DeclStmt* tmp = new DeclStmt();

        ExprNode* exp=$4;
        if(identifiers->getLevel()==0){
            SymbolEntry* const_se=new ConstantSymbolEntry($1, $4->getSymPtr()->get_value());
            exp=new Constant(const_se);
        }

        tmp->insert(new Id(se),exp);
        $$ = tmp;       
        delete []$2;
    }
    |
    Type ARRAY SEMICOLON{
        //处理数组声明
                std::map <std::string, vector<ExprNode*>>::iterator it1=arraylist.begin();
        SymbolEntry *se1;
        DeclStmt* tmp = new DeclStmt();
        while(it1!=arraylist.end()){
            // cout<<it->first<<endl;
            Type* arrayType;
            //vector<int>dims;
            dims.clear();
            for(auto & iter:it1->second){
                //dims.push_back(((ConstantSymbolEntry*)(iter->getSymPtr()))->getValue());
                dims.push_back(iter->getSymPtr()->get_value());
            }
            arrayType=new ArrayType($1, dims);
            se1 = new IdentifierSymbolEntry(arrayType, it1->first, identifiers->getLevel());
            if(!identifiers->lookup(it1->first)||((IdentifierSymbolEntry*)(identifiers->lookup(it1->first)))->getScope()!=identifiers->getLevel()){
                identifiers->install(it1->first, se1);
                
                vector<ExprNode*> newv;
                tmp->insert_array(new Id(se1), newv);
            }
            it1++;
        }
        $$ = tmp;
        arraylist.clear();
        init_.clear();
    }
    |
    Type ARRAY ASSIGN BRACEUnit SEMICOLON{
        cout<<endl;
        cout<<"array assign??"<<endl;
        //处理数组声明
        std::map <std::string, vector<ExprNode*>>::iterator it1=arraylist.begin();
        SymbolEntry *se1;
        DeclStmt* tmp = new DeclStmt();
        
        while(it1!=arraylist.end()){
            // cout<<it->first<<endl;
            //cout<<it1->first<<" installed??"<<endl;
            if(!identifiers->lookup(it1->first)||((IdentifierSymbolEntry*)(identifiers->lookup(it1->first)))->getScope()!=identifiers->getLevel()){
                Type* arrayType;
                //vector<int>dims;
                dims.clear();
                //dc_record.clear();
                for(auto & iter:it1->second){
                    //dims.push_back(((ConstantSymbolEntry*)(iter->getSymPtr()))->getValue());
                    //dc_record.push_back(0);
                    dims.push_back(iter->getSymPtr()->get_value());
                }
                arrayType=new ArrayType($1, dims);
                se1 = new IdentifierSymbolEntry(arrayType, it1->first, identifiers->getLevel());           

                identifiers->install(it1->first, se1);
                tmp->insert_array(new Id(se1), init_[it1->first]);
                //cout<<it1->first<<" installed!!"<<endl;
            }
            //cout<<(identifiers->lookup(it1->first)==nullptr)<<endl;
            it1++;
        }
        $$ = tmp;
        arraylist.clear();
        dc_record.clear();
        init_.clear();
        l_br=-1;
    }
    |
    Type IDList SEMICOLON {
        cout<<"hhh"<<endl;
        DeclStmt* tmp = new DeclStmt();

        //处理普通变量声明
        std::map <std::string, ExprNode*>::iterator it=idlist.begin();
        SymbolEntry *se;
        while(it!=idlist.end()){
            cout<<"!!!!!!!!!!!!!"<<endl;
            cout<<it->first<<endl;
            se=identifiers->lookup(it->first);
            if(!se){
                se = new IdentifierSymbolEntry($1, it->first, identifiers->getLevel());
            }
            else{ 
                se->setType($1);
            }
            if(it->second){
                se->set_value(it->second->getSymPtr()->get_value());
                // cout<<"******************"<<it->second->getSymPtr()->get_value()<<endl;
                //救命。。
                if(identifiers->getLevel()==0){
                    SymbolEntry* const_se=new ConstantSymbolEntry($1, it->second->getSymPtr()->get_value());
                    // cout<<((ConstantSymbolEntry*)const_se)->getValue()<<endl;
                    const_se->set_value(it->second->getSymPtr()->get_value());
                    it->second=new Constant(const_se);
                }
                // cout<<"******************"<<it->second->getSymPtr()->get_value()<<endl;
            }
            cout<<it->first<<endl;
            identifiers->install(it->first, se);
            // SymbolEntry* const_se=new ConstantSymbolEntry(TypeSystem::floatType, it->second->getSymPtr()->get_value());
            // ExprNode* c=new Constant(const_se);
            
            tmp->insert(new Id(se), it->second);
            it++;
        }

        //处理数组声明
        std::map <std::string, vector<ExprNode*>>::iterator it1=arraylist.begin();
        SymbolEntry *se1;
        while(it1!=arraylist.end()){
            cout<<"---------------------"<<it1->first<<endl;
            cout<<it1->second.size()<<endl;
            Type* arrayType;
            // vector<int>dims;
            dims.clear();
            for(auto & iter:it1->second){
                //dims.push_back(((ConstantSymbolEntry*)(iter->getSymPtr()))->getValue());
                cout<<"value:"<<iter->getSymPtr()->get_value()<<endl;
                dims.push_back(iter->getSymPtr()->get_value());
            }
            arrayType=new ArrayType($1, dims);
            se1 = new IdentifierSymbolEntry(arrayType, it1->first, identifiers->getLevel());
            if(!identifiers->lookup(it1->first)){
                identifiers->install(it1->first, se1);
                tmp->insert_array(new Id(se1), init_[it1->first]);
            }
            it1++;
        }

        $$ = tmp;
        idlist.clear();//存完以后清空
        arraylist.clear();
        init_.clear();
        //delete []$2;
    }
    ;
ParamDefs:
    Type ID{
        // paramtypes.clear();
        // paramsymbols.clear();
        paramtypes.push_back($1);
        paramsymbols.push_back($2);
    }
    |
    Type ARRAY{       
        vector<int> emp;
        for(auto & exp:arraylist[curr_array.back()]){
            int val=0;
            if(exp){
                val=exp->getSymPtr()->get_value();
            }
            emp.push_back(val);
        }
        paramtypes.push_back(new ArrayType($1, emp));
        paramsymbols.push_back(curr_array.back());
        arraylist.erase(curr_array.back());
    }
    |
    ParamDefs COMMA Type ID{
        paramtypes.push_back($3);
        paramsymbols.push_back($4);
    }
    |
    ParamDefs COMMA Type ARRAY{
        vector<int> emp;
        for(auto & exp:arraylist[curr_array.back()]){
            int val=0;
            if(exp){
                val=exp->getSymPtr()->get_value();
            }
            emp.push_back(val);
        }
        paramtypes.push_back(new ArrayType($3, emp));
        paramsymbols.push_back(curr_array.back());
        arraylist.erase(curr_array.back());

        // paramtypes.push_back($3);
        // paramsymbols.push_back($4);
    }
    ;
FuncDef
    :
    Type ID 
    LPAREN ParamDefs RPAREN
    {
        isret=false;

        //类型检查：是否重复定义（可能有问题，声明没有处理）
        SymbolEntry *se1=identifiers->lookup($2);
        //cout<<((FunctionType*)(se1->getType()))->num_params()<<endl;
        //cout<<(int)(paramtypes.size())<<endl;
        if(se1){
            if(((FunctionType*)(se1->getType()))->num_params()==(int)(paramtypes.size())){
                cout<<"line: "<<yylineno<<endl;
                fprintf(stderr, "function define error: the function is redefined.\n");
                //assert(se1);
            }
        }

        //类型检查：检查return
        curr_func=$2;


        Type *funcType;
        //std::vector<Type*> params;
        //params.swap(paramtypes);
        // for(auto& t:paramtypes){
        //     cout<<t->toStr();
        // }
        // cout<<endl;
        funcType = new FunctionType($1,paramtypes);
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);

        
        for(int i=0;i<int(paramsymbols.size());i++){
            // cout<<paramsymbols[i]<<endl;
            SymbolEntry *sesym = new IdentifierSymbolEntry(paramtypes[i], paramsymbols[i], identifiers->getLevel());
            sesymlist.push_back(sesym);
            identifiers->install(paramsymbols[i], sesym);
        }
    }
    BlockStmt
    {

        //类型检查：检查返回值类型
        if($1->isVoid()&&isret){
            cout<<"line:"<<yylineno<<endl;
            cout<<"function\""<<$2<<"\""<<"should return void!"<<endl;
        }
        else if (!$1->isVoid()&&!isret){
            cout<<"line:"<<yylineno<<endl;
            cout<<"function\""<<$2<<"\""<<"should return an int value!"<<endl;
        }

        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);

        //改动
        vector<Id*> paramlist;
        for(int i=0;i<int(paramsymbols.size());i++){
            //SymbolEntry *sesym = new IdentifierSymbolEntry(paramtypes[i], paramsymbols[i], identifiers->getLevel());
            Id *id = new Id(sesymlist[i]);//不能new！！一定要保持符号表表项
            paramlist.push_back(id);
        }
        $$ = new FunctionDef(se, $7, paramlist, paramsymbols);

        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();

        std::vector<Type*> params;
        params.swap(paramtypes);
        std::vector<std::string> params1;
        params1.swap(paramsymbols);
        std::vector<SymbolEntry*> sesyms;
        sesymlist.swap(sesyms);

        paramtypes.clear();
        paramsymbols.clear();
        delete top;
        delete []$2;
        
        cout<<"def end??"<<endl;
    }
    |
    Type ID 
    LPAREN RPAREN
    {
        isret=false;

        //类型检查：是否重复定义（可能有问题，声明没有处理）
        SymbolEntry *se1=identifiers->lookup($2);
        //cout<<((FunctionType*)(se1->getType()))->num_params()<<endl;
        //cout<<(int)(paramtypes.size())<<endl;
        if(se1){
            if(((FunctionType*)(se1->getType()))->num_params()==(int)(paramtypes.size())){
                cout<<"line: "<<yylineno<<endl;
                fprintf(stderr, "function define error: the function is redefined.\n");
                //assert(se1);
            }
        }
        //类型检查：检查return
        curr_func=$2;

        Type *funcType;
        std::vector<Type*> params;
        funcType = new FunctionType($1,params);
        SymbolEntry *se = new IdentifierSymbolEntry(funcType, $2, identifiers->getLevel());
        identifiers->install($2, se);
        identifiers = new SymbolTable(identifiers);

        for(int i=0;i<int(paramsymbols.size());i++){
            // cout<<paramsymbols[i]<<endl;
            SymbolEntry *sesym = new IdentifierSymbolEntry(paramtypes[i], paramsymbols[i], identifiers->getLevel());
            identifiers->install(paramsymbols[i], sesym);
        }
    }
    BlockStmt
    {

        //类型检查：检查返回值类型
        if($1->isVoid()&&isret){
            cout<<"line:"<<yylineno<<endl;
            cout<<"function\""<<$2<<"\""<<"should return void!"<<endl;
        }
        else if (!$1->isVoid()&&!isret){
            cout<<"line:"<<yylineno<<endl;
            cout<<"function\""<<$2<<"\""<<"should return an int value!"<<endl;
        }

        SymbolEntry *se;
        se = identifiers->lookup($2);
        assert(se != nullptr);
        //改动
        vector<Id*> paramlist;
        for(int i=0;i<int(paramsymbols.size());i++){
            SymbolEntry *sesym = new IdentifierSymbolEntry(paramtypes[i], paramsymbols[i], identifiers->getLevel());
            Id *id = new Id(sesym);
            paramlist.push_back(id);
        }
        $$ = new FunctionDef(se, $6, paramlist, paramsymbols);
        SymbolTable *top = identifiers;
        identifiers = identifiers->getPrev();

        paramtypes.clear();
        paramsymbols.clear();
        delete top;
        delete []$2;
    }
    ;
Params:
    Exp{
        //全部变成ID了。。。
        //globle_tmp_paramcalls.clear();
        globle_tmp_paramcalls[call_level].push_back($1);

        // if($1->getSymPtr()->getType()->isFunc()) call_level++;
    }
    |
    Params COMMA Exp{
        //SymbolEntry *se;
        //se = identifiers->lookup($3);
        //assert(se != nullptr);
        globle_tmp_paramcalls[call_level].push_back($3);

        // if($3->getSymPtr()->getType()->isFunc()) call_level++;
    }
    ;
FuncCall
    :
    ID LPAREN{call_level++;} Params RPAREN
    {
        std::vector<ExprNode*> params;
        params.swap(globle_tmp_paramcalls[call_level]);
        
        SymbolEntry *se;
        se = identifiers->lookup($1);
        
        cout<<$1<<endl;
        //类型检查：函数是否声明
        if(!se){
            cout<<"line:"<<yylineno<<endl;
            cout<<"function\""<<$1<<"\" is called without declaration!"<<endl;
        }
        assert(se != nullptr);
        //类型检查：检查参数个数是否正确
        if(int(params.size())!=((FunctionType*)(se->getType()))->num_params()){
            cout<<"line: "<<yylineno<<endl;
            fprintf(stderr, "function call error: please check params.\n");
            //assert();
        }
        $$ = new FunctionCall(se, params);

        call_level--;
    }
    |
    ID LPAREN{call_level++;} RPAREN
    {
        std::vector<ExprNode*> params;
        params.swap(globle_tmp_paramcalls[call_level]);
        SymbolEntry *se;
        se = identifiers->lookup($1);
        //assert(se != nullptr);
        $$ = new FunctionCall(se, params);
    }
    ;

%%

int yyerror(char const* message)
{
    std::cerr<<message<<std::endl;
    return -1;
}
