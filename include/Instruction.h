#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#include "Operand.h"
#include "AsmBuilder.h"
#include <vector>
#include <map>
#include <sstream>

using namespace std;

class BasicBlock;

class Instruction
{
public:
    Instruction(unsigned instType, BasicBlock *insert_bb = nullptr);
    virtual ~Instruction();
    BasicBlock *getParent();
    bool isUncond() const {return instType == UNCOND;};
    bool isCond() const {return instType == COND;};
    bool isAlloc() const {return instType == ALLOCA;};
    void setParent(BasicBlock *);
    void setNext(Instruction *);
    void setPrev(Instruction *);
    Instruction *getNext();
    Instruction *getPrev();
    virtual void output() const = 0;
    MachineOperand* genMachineOperand(Operand*);
    MachineOperand* genMachineFPOperand(Operand*);
    MachineOperand* genMachineReg(int reg);
    MachineOperand* genMachineFReg(int freg);
    MachineOperand* genMachineVReg(bool fp=false);
    MachineOperand* genMachineImm(int val);
    MachineOperand* genMachineLabel(int block_no);
    virtual void genMachineCode(AsmBuilder*) = 0;
    std::vector<Operand*> getOperands(){return operands;};

    bool killed=false;
protected:
    unsigned instType;
    unsigned opcode;
    Instruction *prev;
    Instruction *next;
    BasicBlock *parent;
    std::vector<Operand*> operands;
    enum {BINARY, COND, UNCOND, RET, LOAD, STORE, CMP, ALLOCA, CALL, GLOBAL, TYPEFIX, ARRAYITEMFETCH, MEMSET};
};

// meaningless instruction, used as the head node of the instruction list.
class DummyInstruction : public Instruction
{
public:
    DummyInstruction() : Instruction(-1, nullptr) {};
    void output() const {};
    void genMachineCode(AsmBuilder*) {};
};

class AllocaInstruction : public Instruction
{
public:
    AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb = nullptr);
    ~AllocaInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
private:
    SymbolEntry *se;
};

class LoadInstruction : public Instruction
{
public:
    LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb = nullptr);
    ~LoadInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

class StoreInstruction : public Instruction
{
public:
    StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb = nullptr);
    ~StoreInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

class BinaryInstruction : public Instruction
{
public:
    BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~BinaryInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    enum {SUB, ADD, MUL, DIV, MOD, AND, OR, NOT, SAME};
};

class CmpInstruction : public Instruction
{
public:
    CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb = nullptr);
    ~CmpInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    enum {E, NE, L, LE , G, GE};
};

// unconditional branch
class UncondBrInstruction : public Instruction
{
public:
    UncondBrInstruction(BasicBlock*, BasicBlock *insert_bb = nullptr);
    void output() const;
    void setBranch(BasicBlock *);
    BasicBlock *getBranch();
    void genMachineCode(AsmBuilder*);
protected:
    BasicBlock *branch;
};

// conditional branch
class CondBrInstruction : public Instruction
{
public:
    CondBrInstruction(BasicBlock*, BasicBlock*, Operand *, BasicBlock *insert_bb = nullptr);
    ~CondBrInstruction();
    void output() const;
    void setTrueBranch(BasicBlock*);
    BasicBlock* getTrueBranch();
    void setFalseBranch(BasicBlock*);
    BasicBlock* getFalseBranch();
    void genMachineCode(AsmBuilder*);
protected:
    BasicBlock* true_branch;
    BasicBlock* false_branch;
};

class RetInstruction : public Instruction
{
public:
    RetInstruction(Operand *src, BasicBlock *insert_bb = nullptr);
    ~RetInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

//函数调用 call
class CallInstruction : public Instruction
{
public:
    std::string ret_type;
    std::string names;
    vector<Operand*> vo;
    vector<Type*> types;
    int isvoid;
    //SymbolEntry *se;
public:
    CallInstruction(SymbolEntry *symbolentry,Operand *dst,vector<Operand*> vo,BasicBlock *insert_bb = nullptr);
    ~CallInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

//global 全局变量声明指令
//@a = dso_local global i32 3, align 4
class GlobalInstruction : public Instruction
{
    string global_arr;
public:
    GlobalInstruction(Operand *dst, Operand* src, BasicBlock *insert_bb=nullptr, string global_arr="");
    ~GlobalInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    vector<Operand*> getOperands(){return operands;};
};

//强制类型转换
class TypefixInstruction : public Instruction
{
        
    int kind;
public:
    enum{BOOL2INT, INT2FLOAT, FLOAT2INT};
    TypefixInstruction(Operand* dst, Operand *src, BasicBlock *insert_bb = nullptr, int kind=BOOL2INT);
    ~TypefixInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};

//取数组元素指令
class ArrayItemFetchInstruction : public Instruction
{
    Operand* tag;
    int size;
    Type* type;
    bool f;
    bool param_flag;
    Operand* param_addr;
    bool memset_flag;
public:
    ArrayItemFetchInstruction(Operand* tag, Type* type, Operand *dst_addr, Operand* item_addr, Operand *offset, BasicBlock *insert_bb = nullptr, bool f=false, Operand* addr=nullptr);
    ArrayItemFetchInstruction(Type* array_type, Operand* addr, bool f, BasicBlock* insert_bb=nullptr):Instruction(ARRAYITEMFETCH, insert_bb),type(array_type),memset_flag(f){
        operands.push_back(addr);
        addr->addUse(this);
    };
    ~ArrayItemFetchInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
    void set_paramFlag(bool f){this->param_flag=f;};
    bool get_paramFlag(){return param_flag;};
    void set_paramAddr(Operand* pd){param_addr=pd;};
    Operand* get_paramAddr(){return param_addr;};
    void set_memsetFlag(bool f){this->memset_flag=f;};
    bool get_memsetFlag(){return memset_flag;};
};

//取数组元素指令
class MemsetInstruction : public Instruction
{
    Type* type;
public:
    MemsetInstruction(Type* array_type, Operand* addr, BasicBlock* insert_bb=nullptr);    
    ~MemsetInstruction();
    void output() const;
    void genMachineCode(AsmBuilder*);
};



#endif