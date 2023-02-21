#ifndef __MACHINECODE_H__
#define __MACHINECODE_H__
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <fstream>
#include "SymbolTable.h"
#include "Type.h"

using namespace std;

/* Hint:
* MachineUnit: Compiler unit
* MachineFunction: Function in assembly code 
* MachineInstruction: Single assembly instruction  
* MachineOperand: Operand in assembly instruction, such as immediate number, register, address label */

/* Todo:
* We only give the example code of "class BinaryMInstruction" and "class AccessMInstruction" (because we believe in you !!!),
* You need to complete other the member function, especially "output()" ,
* After that, you can use "output()" to print assembly code . */

class MachineUnit;
class MachineFunction;
class MachineBlock;
class MachineInstruction;

class MachineOperand
{
private:
    MachineInstruction* parent;
    int type;
    int val;  // value of immediate number
    int reg_no; // register no
    std::string label; // address label
    bool isfunc;//区分label的打印

    bool is_fp=false;
    float fval;
public:
    enum { IMM, VREG, REG, LABEL };
    MachineOperand(int tp, int val, bool fp=false);
    MachineOperand(int tp, float val, bool fp=true);
    MachineOperand(std::string label, bool isfunc=false);
    bool operator == (const MachineOperand&) const;
    bool operator < (const MachineOperand&) const;
    bool isImm() { return this->type == IMM; }; 
    bool isReg() { return this->type == REG; };
    bool isVReg() { return this->type == VREG; };
    bool isLabel() { return this->type == LABEL; };
    int getVal() {return this->val; };
    
    int getReg() {return this->reg_no; };
    void setReg(int regno) {this->type = REG; this->reg_no = regno;};
    std::string getLabel() {return this->label; };
    void setParent(MachineInstruction* p) { this->parent = p; };
    MachineInstruction* getParent() { return this->parent;};
    void PrintReg();
    void output();

    bool isFloat() { return this->is_fp; }
    void setFVal(float val) {fval=val; is_fp=true;};
    float getFVal(){return fval;};

};

class MachineInstruction
{
protected:
    MachineBlock* parent;
    int no;
    int type;  // Instruction type
    int cond = MachineInstruction::NONE;  // Instruction execution condition, optional !!
    int op;  // Instruction opcode
    

    // Instruction operand list, sorted by appearance order in assembly instruction
    std::vector<MachineOperand*> def_list;
    std::vector<MachineOperand*> use_list;
    void addDef(MachineOperand* ope) { def_list.push_back(ope); };
    void addUse(MachineOperand* ope) { use_list.push_back(ope); };
    // Print execution code after printing opcode
    void PrintCond();
    enum instType { BINARY, LOAD, STORE, MOV, BRANCH, CMP, STACK, VMRS, VCVT };
public:
    bool dead=false;
    enum condType { E, NE, L, LE , G, GE, NONE };//和CmpInstruction保持统一 enum {E, NE, L, GE, G, LE};

    virtual void output() = 0;
    void setNo(int no) {this->no = no;};
    int getNo() {return no;};
    MachineBlock* getParent(){return parent;};
    std::vector<MachineOperand*>& getDef() {return def_list;};
    std::vector<MachineOperand*>& getUse() {return use_list;};

    bool isBinary(){return type==BINARY;};
    bool isStack(){return type==STACK;};
    bool isLoad(){return type==LOAD;};
    void insertBefore(MachineInstruction* inst);
    void insertAfter(MachineInstruction* inst);
};

class BinaryMInstruction : public MachineInstruction
{
    bool bp;
public:
    enum opType { ADD, SUB, MUL, DIV, AND, OR, VADD, VSUB, VMUL, VDIV};
    BinaryMInstruction(MachineBlock* p, int op, 
                    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2, 
                    bool bp=false,
                    int cond = MachineInstruction::NONE);
    void output();
    bool isbp(){return bp;};
    void set_src2(MachineOperand* newsrc2){use_list[1]=newsrc2;};
};


class LoadMInstruction : public MachineInstruction
{
    bool bp;
    int kind;
public:
    enum{LDR, VLDR};
    LoadMInstruction(MachineBlock* p, int kind,
                    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2 = nullptr, 
                    int cond = MachineInstruction::NONE, bool bp=false);
    void output();
    void set_src1(MachineOperand* newsrc1){use_list[0]=newsrc1;};
    void set_src2(MachineOperand* newsrc2){use_list[1]=newsrc2;};
    bool isbp(){return bp;};
};

class StoreMInstruction : public MachineInstruction
{
    int kind;
public:
    enum{STR, VSTR};
    StoreMInstruction(MachineBlock* p, int kind,
                    MachineOperand* src1, MachineOperand* src2, MachineOperand* src3 = nullptr, 
                    int cond = MachineInstruction::NONE);
    void output();
};

class MovMInstruction : public MachineInstruction
{
public:
    enum opType { MOV, VMOV, VMOVF32, MVN, MOVEQ, MOVNE, MOVGE, MOVGT, MOVLE, MOVLT };
    MovMInstruction(MachineBlock* p, int op, 
                MachineOperand* dst, MachineOperand* src,
                int cond = MachineInstruction::NONE);
    void output();
};

class BranchMInstruction : public MachineInstruction
{
public:
    enum opType { B, BL, BX };
    BranchMInstruction(MachineBlock* p, int op, 
                MachineOperand* dst, 
                int cond = MachineInstruction::NONE);
    void output();
};

class CmpMInstruction : public MachineInstruction
{
    int kind;
public:
    enum opType { CMP, VCMP };
    CmpMInstruction(MachineBlock* p, int kind,
                MachineOperand* src1, MachineOperand* src2, 
                int cond = MachineInstruction::NONE);
    void output();
};

class VmrsMInstruction : public MachineInstruction {
   public:
    VmrsMInstruction(MachineBlock* p);
    void output();
};

class VcvtMInstruction : public MachineInstruction {
   public:
    enum opType { S2F, F2S };
    VcvtMInstruction(MachineBlock* p,
                     int op,
                     MachineOperand* dst,
                     MachineOperand* src,
                     int cond = MachineInstruction::NONE);
    void output();
};

class StackMInstructon : public MachineInstruction
{
public:
    enum opType { PUSH, POP, VPUSH, VPOP };
    StackMInstructon(MachineBlock* p, int op, 
                vector<MachineOperand*> src_list,
                int cond = MachineInstruction::NONE);
    void output();
    void addSrc(vector<MachineOperand*> src_list);
    bool isPOP(){return op==POP;};
    bool isVPOP(){return op==VPOP;};
};

// class GlobalMInstruction : public MachineInstruction
// {
//     int size;
// public:
//     GlobalMInstruction(MachineBlock* p, 
//                 MachineOperand* dst,
//                 MachineOperand* src,
//                 int size,
//                 int cond = MachineInstruction::NONE);
//     void output();
// };

class MachineBlock
{
private:
    MachineFunction* parent;
    int no;  
    std::vector<MachineBlock *> pred, succ;
    std::vector<MachineInstruction*> inst_list;
    std::set<MachineOperand*> live_in;
    std::set<MachineOperand*> live_out;
    int opcode;//记录cmp的opcode
    static int label;
public:
    std::vector<MachineInstruction*>& getInsts() {return inst_list;};
    std::vector<MachineInstruction*>::iterator begin() { return inst_list.begin(); };
    std::vector<MachineInstruction*>::iterator end() { return inst_list.end(); };
    std::vector<MachineInstruction*>::reverse_iterator rbegin() { return inst_list.rbegin(); };
    MachineBlock(MachineFunction* p, int no) { this->parent = p; this->no = no; };
    void InsertInst(MachineInstruction* inst) { this->inst_list.push_back(inst); };
    void addPred(MachineBlock* p) { this->pred.push_back(p); };
    void addSucc(MachineBlock* s) { this->succ.push_back(s); };
    std::set<MachineOperand*>& getLiveIn() {return live_in;};
    std::set<MachineOperand*>& getLiveOut() {return live_out;};
    std::vector<MachineBlock*>& getPreds() {return pred;};
    std::vector<MachineBlock*>& getSuccs() {return succ;};
    MachineFunction* getParent(){return parent;};
    void output();
    void deadinst_mark();
    void set_op(int op){this->opcode = op;};
    int get_op(){return this->opcode;};

    int getCount(){return inst_list.size();};
    
};

class MachineFunction
{
private:
    MachineUnit* parent;
    std::vector<MachineBlock*> block_list;
    int stack_size;
    std::set<int> saved_regs;
    SymbolEntry* sym_ptr;
public:
    vector<MachineOperand*> src_list;
    vector<MachineOperand*> v_src_list;
    vector<MachineInstruction*> stack_list;
    vector<int> use_regnos;

public:
    std::vector<MachineBlock*>& getBlocks() {return block_list;};
    std::vector<MachineBlock*>::iterator begin() { return block_list.begin(); };
    std::vector<MachineBlock*>::iterator end() { return block_list.end(); };
    MachineFunction(MachineUnit* p, SymbolEntry* sym_ptr);
    /* HINT:
    * Alloc stack space for local variable;
    * return current frame offset ;
    * we store offset in symbol entry of this variable in function AllocInstruction::genMachineCode()
    * you can use this function in LinearScan::genSpillCode() */
    int AllocSpace(int size) { this->stack_size += size; return this->stack_size; };
    int getSize(){return this->stack_size; };
    void InsertBlock(MachineBlock* block) { this->block_list.push_back(block); };
    void addSavedRegs(int regno) {saved_regs.insert(regno);};
    int num_SavedRegs(){return saved_regs.size();};
    void output();
    void deadinst_mark();
    MachineUnit* getParent(){return parent;};

    int getCount(){
        int count=0;
        for(auto& block:block_list){
            count+=block->getCount();
        }
        return count;
    }
};

class MachineUnit
{
public:
    int n;
private:
    std::vector<MachineFunction*> func_list;
    void PrintGlobalDecl();
    
public:
    void PrintGlobalEnd();
    std::vector<SymbolEntry*> global_dst;
    std::vector<SymbolEntry*> global_src;

    std::vector<SymbolEntry*> arr_global_dst;
    std::vector<vector<SymbolEntry*>> arr_global_src;
public:
    std::vector<MachineFunction*>& getFuncs() {return func_list;};
    std::vector<MachineFunction*>::iterator begin() { return func_list.begin(); };
    std::vector<MachineFunction*>::iterator end() { return func_list.end(); };
    void InsertFunc(MachineFunction* func) { func_list.push_back(func);};
    void output();
    void deadinst_mark();
};

#endif