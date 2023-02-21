#ifndef __AST_H__
#define __AST_H__

#include <fstream>
#include "Operand.h"
#include "Type.h"
#include <map>
#include <vector>
#include "MachineCode.h"

using namespace std;

class SymbolEntry;
class Unit;
class Function;
class BasicBlock;
class Instruction;
class IRBuilder;
class IdentifierSymbolEntry;

class Node
{
private:
    static int counter;
    int seq;
protected:
    std::vector<Instruction*> true_list;
    std::vector<Instruction*> false_list;
    static IRBuilder *builder;
    void backPatch(std::vector<Instruction*> &list, BasicBlock*bb, bool branch=true);
    std::vector<Instruction*> merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2);

public:
    Node();
    int getSeq() const {return seq;};
    static void setIRBuilder(IRBuilder*ib) {builder = ib;};
    virtual void output(int level) = 0;
    virtual void typeCheck() = 0;
    virtual void genCode() = 0;
    std::vector<Instruction*>& trueList() {return true_list;}
    std::vector<Instruction*>& falseList() {return false_list;}
};

class ExprNode : public Node
{
protected:
    enum{BASIC, ID, ARRAY};
    SymbolEntry *symbolEntry;
    Operand *dst;   // The result of the subtree is stored into dst.
    int expr_type;//加一个枚举类型，为了区分id和arrayitem
    double value;
    
    bool arr_flag;
public:
    ExprNode(SymbolEntry *symbolEntry, int expr_type=BASIC) : symbolEntry(symbolEntry), expr_type(expr_type){
        value=0;
        arr_flag=false;
    };//dst=new Operand(symbolEntry);
    Operand* getOperand() {return dst;};
    SymbolEntry* getSymPtr() {return symbolEntry;};
    bool isBasic(){return expr_type==BASIC;};
    bool isId(){return expr_type==ID;};
    bool isArray(){return expr_type==ARRAY;};
    void set_value(double val){value=val;};
    double get_value(){return value;};
    void set_arrflag(bool f){arr_flag=f;};
    bool get_arrflag(){return arr_flag;};
};

class BinaryExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr1, *expr2;
public:
    enum {ADD, SUB, MUL, DIV, MOD, AND, OR, LESS, LESS_E, EQUAL, NOT_EQUAL, MORE_E, MORE};
    BinaryExpr(SymbolEntry *se, int op, ExprNode*expr1, ExprNode*expr2) : ExprNode(se), op(op), expr1(expr1), expr2(expr2){
        if(op >= ADD && op <= MOD ) dst = new Operand(se);
        else dst = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        };
    void output(int level);
    void typeCheck();
    void genCode();
};
class preSingleExpr : public ExprNode
{
private:
    int op;
    ExprNode *expr;
public:
    enum {NOT, AADD, SSUB,ADD,SUB};
    preSingleExpr(SymbolEntry *se, int op, ExprNode*expr) : ExprNode(se), op(op), expr(expr){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class sufSingleExpr : public ExprNode
{
private:
    ExprNode *expr;
    int op;    
public:
    enum {AADD, SSUB};
    sufSingleExpr(SymbolEntry *se, ExprNode*expr, int op) : ExprNode(se), expr(expr), op(op){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};
class Constant : public ExprNode
{
public:
    Constant(SymbolEntry *se) : ExprNode(se){dst = new Operand(se);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class Id : public ExprNode
{
public:
    Id(SymbolEntry *se) : ExprNode(se, ExprNode::ID){SymbolEntry *temp = new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()); dst = new Operand(temp);};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ArrayItem : public ExprNode
{
   vector<ExprNode*> offsets;//有可能是INTEGER，有可能是ID
    bool f;
    bool isparam;
public:
    vector<Operand*> heads;
public:
    ArrayItem(SymbolEntry *se, vector<ExprNode*> offsets, bool f=false) : ExprNode(se, ExprNode::ARRAY), offsets(offsets), f(f){
        SymbolEntry *temp = new TemporarySymbolEntry(((ArrayType*)(se->getType()))->gettype(), SymbolTable::getLabel()); 
        dst = new Operand(temp);//元素自己

    // for(int i=0;i<this->offsets.size();i++){
        //     Type* type = new PointerType(((ArrayType*)(this->getSymPtr()->getType()))->gettype());//数组元素的类型
        //     SymbolEntry *tmp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
        //     //addr = iter->first->getOperand();//new Operand(addr_se);
        //     heads.push_back(new Operand(tmp_se));
        // }
    };
    // ExprNode* get_offset(){return offset;};
    // Operand* gethead(){return heads[0];};
    bool getf(){return f;};
    void setf(bool flag){f=flag;};
    void setParam(bool flag){isparam=flag;};
    bool getParam(){return isparam;}
    void output(int level);
    void typeCheck();
    void genCode();
    vector<ExprNode*> get_offsets(){return offsets;};
};

class StmtNode : public Node
{};

class CompoundStmt : public StmtNode
{
private:
    StmtNode *stmt;
public:
    CompoundStmt(StmtNode *stmt) : stmt(stmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class SeqNode : public StmtNode
{
private:
    StmtNode *stmt1, *stmt2;
public:
    SeqNode(StmtNode *stmt1, StmtNode *stmt2) : stmt1(stmt1), stmt2(stmt2){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class DeclStmt : public StmtNode
{
private:
    //Id *id;
    std::map<Id*, ExprNode*> idlist;
    std::map<Id*, vector<ExprNode*>> arraylist;
public:
    DeclStmt(){};
    void insert(Id* id, ExprNode* init=nullptr){
        this->idlist[id]=init;
    }
    void insert_array(Id* id, vector<ExprNode*> initlist){
        this->arraylist[id]=initlist;
    }
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
public:
    IfStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class IfElseStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    StmtNode *elseStmt;
public:
    IfElseStmt(ExprNode *cond, StmtNode *thenStmt, StmtNode *elseStmt) : cond(cond), thenStmt(thenStmt), elseStmt(elseStmt) {};
    void output(int level);
    void typeCheck();
    void genCode();
};
class WhileStmt : public StmtNode
{
private:
    ExprNode *cond;
    StmtNode *thenStmt;
    BasicBlock* end_bb;
    BasicBlock* cond_bb;
public:
    WhileStmt(ExprNode *cond, StmtNode *thenStmt) : cond(cond), thenStmt(thenStmt){};
    void set_end_bb(BasicBlock* end_bb){this->end_bb=end_bb;};
    BasicBlock* get_end_bb(){return end_bb;};
    void set_cond_bb(BasicBlock* cond_bb){this->cond_bb=cond_bb;};
    BasicBlock* get_cond_bb(){return cond_bb;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ReturnStmt : public StmtNode
{
private:
    ExprNode *retValue;
public:
    ReturnStmt(ExprNode*retValue=nullptr) : retValue(retValue) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class AssignStmt : public StmtNode
{
private:
    ExprNode *lval;
    ExprNode *expr;
public:
    AssignStmt(ExprNode *lval, ExprNode *expr) : lval(lval), expr(expr) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class FunctionDef : public StmtNode
{
private:
    SymbolEntry *se;
    //增加两个vector
    vector<Id*> paramlist;
    vector<string> paramNames;
    StmtNode *stmt;
public:
    FunctionDef(SymbolEntry *se, StmtNode *stmt, vector<Id*> paramlist, vector<string> paramNames) : se(se), stmt(stmt){
        this->paramlist.assign(paramlist.begin(),paramlist.end());
        this->paramNames.assign(paramNames.begin(),paramNames.end());
    };
    void output(int level);
    void typeCheck();
    void genCode();
};
class FunctionCall : public ExprNode
{
private:
    SymbolEntry *se;
    vector<ExprNode*> params;
public:
    FunctionCall(SymbolEntry *se, vector<ExprNode*> params) : ExprNode(se), se(se) {
        SymbolEntry *se0 = new TemporarySymbolEntry(((FunctionType*)(se->getType()))->getRetType(), SymbolTable::getLabel()); 
        dst=new Operand(se0);
        this->params.assign(params.begin(),params.end());
    };
    void output(int level);
    void typeCheck();
    void genCode();
};
class ExprStmt : public StmtNode
{
private:
    ExprNode *exp;
public:
    ExprStmt(ExprNode*exp=nullptr) : exp(exp) {};
    void output(int level);
    void typeCheck();
    void genCode();
};

class EmptyStmt : public StmtNode
{
public:
    EmptyStmt(){};
    void output(int level);
    void typeCheck();
    void genCode();
};

class BreakStmt : public StmtNode
{
    StmtNode* parent;
public:
    BreakStmt(){};
    void setParent(StmtNode* parent){this->parent=parent;};
    StmtNode* getParent(){return parent;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class ContinueStmt : public StmtNode
{
    StmtNode* parent;
public:
    ContinueStmt(){};
    void setParent(StmtNode* parent){this->parent=parent;};
    StmtNode* getParent(){return parent;};
    void output(int level);
    void typeCheck();
    void genCode();
};

class Ast
{
private:
    Node* root;
public:
    Ast() {root = nullptr;}
    void setRoot(Node*n) {root = n;}
    void output();
    void typeCheck();
    void genCode(Unit *unit);
    
};

#endif
