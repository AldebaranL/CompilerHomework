#include "MachineCode.h"

#include <iostream>
#include<string.h>
#include<iomanip>
using namespace std;

extern FILE* yyout;

int MachineBlock::label = 0;

void MachineInstruction::insertBefore(MachineInstruction* inst) {
    auto& instructions = parent->getInsts();
    auto it = std::find(instructions.begin(), instructions.end(), this);
    instructions.insert(it, inst);
}

void MachineInstruction::insertAfter(MachineInstruction* inst) {
    auto& instructions = parent->getInsts();
    auto it = std::find(instructions.begin(), instructions.end(), this);
    instructions.insert(++it, inst);
}

MachineOperand::MachineOperand(int tp, int val, bool fp)
{
    //cout<<"A"<<endl;
    this->type = tp;
    this->is_fp=fp;
    if(tp == MachineOperand::IMM)
        this->val = val;
    else 
        this->reg_no = val;
}

MachineOperand::MachineOperand(int tp, float fval, bool fp)
{
    //cout<<"B"<<endl;
    this->type = tp;
    this->is_fp=fp;
    if(tp == MachineOperand::IMM)
        this->fval = fval;
    // else 
    //     this->reg_no = val;
}

MachineOperand::MachineOperand(std::string label, bool isfunc)
{
    //cout<<"C"<<endl;
    this->type = MachineOperand::LABEL;
    this->label = label;
    this->isfunc=isfunc;
}

bool MachineOperand::operator==(const MachineOperand&a) const
{
    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

bool MachineOperand::operator<(const MachineOperand&a) const
{
    if(this->type == a.type)
    {
        if(this->type == IMM)
            return this->val < a.val;
        return this->reg_no < a.reg_no;
    }
    return this->type < a.type;

    if (this->type != a.type)
        return false;
    if (this->type == IMM)
        return this->val == a.val;
    return this->reg_no == a.reg_no;
}

void MachineOperand::PrintReg()
{
    if (reg_no >= 16) {
        int sreg_no = reg_no - 16;
        if (sreg_no <= 31) {
            fprintf(yyout, "s%d", sreg_no);
        } else if (sreg_no == 32) {
            fprintf(yyout, "FPSCR");
        }
    } 
    else{
        switch (reg_no)
        {
        case 11:
            fprintf(yyout, "fp");
            break;
        case 13:
            fprintf(yyout, "sp");
            break;
        case 14:
            fprintf(yyout, "lr");
            break;
        case 15:
            fprintf(yyout, "pc");
            break;
        default:
            fprintf(yyout, "r%d", reg_no);
            break;
        }
    }
}

void MachineOperand::output() 
{
    /* HINT：print operand
    * Example:
    * immediate num 1 -> print #1;
    * register 1 -> print r1;
    * lable addr_a -> print addr_a; */
    switch (this->type)
    {
    case IMM:
        fprintf(yyout, "#%d", this->val);
        break;
    case VREG:
        fprintf(yyout, "v%d", this->reg_no);
        break;
    case REG:
        PrintReg();
        break;
    case LABEL:
        if(isfunc)
            fprintf(yyout, "%s", this->label.c_str());
        else if (this->label.substr(0, 2) == ".L")
            fprintf(yyout, "%s", this->label.c_str());
        else
            fprintf(yyout, "addr_%s%d", (char*)(this->label.c_str())+1, parent->getParent()->getParent()->getParent()->n);
    default:
        break;
    }
}

void MachineInstruction::PrintCond()
{
    // TODO
    switch (cond) {
        case E:
            fprintf(yyout, "eq");
            break;
        case NE:
            fprintf(yyout, "ne");
            break;
        case L:
            fprintf(yyout, "lt");
            break;
        case LE:
            fprintf(yyout, "le");
            break;
        case G:
            fprintf(yyout, "gt");
            break;
        case GE:
            fprintf(yyout, "ge");
            break;
        default:
            break;
    }
}

BinaryMInstruction::BinaryMInstruction(
    MachineBlock* p, int op, 
    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2, 
    bool bp,
    int cond)
{
    this->parent = p;
    this->type = MachineInstruction::BINARY;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src1);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src1->getReg());
    this->use_list.push_back(src2);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src2->getReg());
    dst->setParent(this);
    src1->setParent(this);
    src2->setParent(this);

    this->bp=bp;//是否回填
}

void BinaryMInstruction::output() 
{
    // TODO: 
    // Complete other instructions
    switch (this->op)
    {
    case BinaryMInstruction::ADD:
        fprintf(yyout, "\tadd ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::SUB:
        fprintf(yyout, "\tsub ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::MUL:
        fprintf(yyout, "\tmul ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::DIV:
        fprintf(yyout, "\tsdiv ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::VADD:
        fprintf(yyout, "\tvadd.f32 ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::VSUB:
        fprintf(yyout, "\tvsub.f32 ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::VMUL:
        fprintf(yyout, "\tvmul.f32 ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    case BinaryMInstruction::VDIV:
        fprintf(yyout, "\tvdiv.f32 ");
        this->PrintCond();
        this->def_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");
        this->use_list[1]->output();
        fprintf(yyout, "\n");
        break;
    default:
        break;
    }
}

LoadMInstruction::LoadMInstruction(MachineBlock* p, int kind,
    MachineOperand* dst, MachineOperand* src1, MachineOperand* src2,
    int cond, bool bp)
{
    this->kind=kind;
    this->parent = p;
    this->type = MachineInstruction::LOAD;
    this->op = -1;
    this->cond = cond;
    this->def_list.push_back(dst);
    cout<<"dst="<<dst<<endl;
    this->use_list.push_back(src1);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src1->getReg());
    this->bp=bp;
    if (src2){
        this->use_list.push_back(src2);
        if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src2->getReg());
    }
    dst->setParent(this);
    src1->setParent(this);
    if (src2)
        src2->setParent(this);
}

void LoadMInstruction::output()
{
    if(kind==LDR){
        fprintf(yyout, "\tldr ");
        this->def_list[0]->output();
        fprintf(yyout, ", ");

        // Load immediate num, eg: ldr r1, =8
        if(this->use_list[0]->isImm())
        {
            if (this->use_list[0]->isFloat()) {
                float fval = this->use_list[0]->getFVal();
                uint32_t temp = reinterpret_cast<uint32_t&>(fval);
                fprintf(yyout, "=%u\n", temp);
            } 
            else{
                fprintf(yyout, "=%d\n", this->use_list[0]->getVal());
            }
            return;
        }

        // Load address
        if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
            fprintf(yyout, "[");

        this->use_list[0]->output();
        if( this->use_list.size() > 1 )
        {
            fprintf(yyout, ", ");
            this->use_list[1]->output();
        }

        if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
            fprintf(yyout, "]");
        fprintf(yyout, "\n");
    }
    else if(kind==VLDR){
        fprintf(yyout, "\tvldr.32 ");
        this->def_list[0]->output();
        fprintf(yyout, ", ");

        // Load immediate num, eg: ldr r1, =8
        if(this->use_list[0]->isImm())
        {
            if (this->use_list[0]->isFloat()) {
                float fval = this->use_list[0]->getFVal();
                uint32_t temp = reinterpret_cast<uint32_t&>(fval);
                fprintf(yyout, "=%u\n", temp);
            } 
            else{
                fprintf(yyout, "=%d\n", this->use_list[0]->getVal());
            }
            return;
        }

        // Load address
        if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
            fprintf(yyout, "[");

        this->use_list[0]->output();
        if( this->use_list.size() > 1 )
        {
            fprintf(yyout, ", ");
            this->use_list[1]->output();
        }

        if(this->use_list[0]->isReg()||this->use_list[0]->isVReg())
            fprintf(yyout, "]");
        fprintf(yyout, "\n");
    }
}

StoreMInstruction::StoreMInstruction(MachineBlock* p,int kind,
    MachineOperand* src1, MachineOperand* src2, MachineOperand* src3, 
    int cond)
{
    // TODO
    this->kind=kind;
    this->parent = p;
    this->type = MachineInstruction::STORE;
    this->op = -1;
    this->cond = cond;
    cout<<"src1="<<src1<<endl;
    cout<<"src2="<<src2<<endl;
    this->use_list.push_back(src1);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src1->getReg());
    this->use_list.push_back(src2);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src2->getReg());
    if (src3){
        this->use_list.push_back(src3);
        if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src3->getReg());
    }
    src1->setParent(this);
    src2->setParent(this);
    if (src3)
        src3->setParent(this);
}

void StoreMInstruction::output()
{
    // TODO
    if(kind==STR){
        fprintf(yyout, "\tstr ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");

        // Store address
        if(this->use_list[1]->isReg()||this->use_list[1]->isVReg())
            fprintf(yyout, "[");

        this->use_list[1]->output();
        if( this->use_list.size() > 2 )
        {
            fprintf(yyout, ", ");
            this->use_list[2]->output();//打印名字？
        }

        if(this->use_list[1]->isReg()||this->use_list[1]->isVReg())
            fprintf(yyout, "]");
        fprintf(yyout, "\n");
    }
    else if(kind==VSTR){
        fprintf(yyout, "\tvstr.32 ");
        this->use_list[0]->output();
        fprintf(yyout, ", ");

        // Store address
        if(this->use_list[1]->isReg()||this->use_list[1]->isVReg())
            fprintf(yyout, "[");

        this->use_list[1]->output();
        if( this->use_list.size() > 2 )
        {
            fprintf(yyout, ", ");
            this->use_list[2]->output();//打印名字？
        }

        if(this->use_list[1]->isReg()||this->use_list[1]->isVReg())
            fprintf(yyout, "]");
        fprintf(yyout, "\n");
    }
}

MovMInstruction::MovMInstruction(MachineBlock* p, int op, 
    MachineOperand* dst, MachineOperand* src,
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::MOV;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src->getReg());
    dst->setParent(this);
    src->setParent(this);
}

void MovMInstruction::output() 
{
    // TODO
    switch (this->op) {
        case MOV:
            fprintf(yyout, "\tmov");
            break;
        case VMOV:
            fprintf(yyout, "\tvmov");
            break;
        case VMOVF32:
            fprintf(yyout, "\tvmov.f32");
            break;
        case MOVEQ:
            fprintf(yyout, "\tmoveq");
            break;
        case MOVNE:
            fprintf(yyout, "\tmovne");
            break;
        case MOVGE:
            fprintf(yyout, "\tmovge");
            break;
        case MOVGT:
            fprintf(yyout, "\tmovgt");
            break;
        case MOVLE:
            fprintf(yyout, "\tmovle");
            break;
        case MOVLT:
            fprintf(yyout, "\tmovlt");
            break;
        default:
            break;
    }
    PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");
}

BranchMInstruction::BranchMInstruction(MachineBlock* p, int op, 
    MachineOperand* dst, 
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::BRANCH;
    this->op = op;
    this->cond = cond;
    this->use_list.push_back(dst);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(dst->getReg());
    dst->setParent(this);
}

void BranchMInstruction::output()
{
    // TODO
    switch (op) {
        case B:
            fprintf(yyout, "\tb");
            break;
        case BX:
            fprintf(yyout, "\tbx");
            break;
        case BL:
            fprintf(yyout, "\tbl");
            break;
    }
    PrintCond();
    fprintf(yyout, " ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");
}

CmpMInstruction::CmpMInstruction(MachineBlock* p, int kind,
    MachineOperand* src1, MachineOperand* src2, 
    int cond)
{
    // TODO
    this->kind=kind;
    this->parent = p;
    this->type = MachineInstruction::CMP;
    this->op = op;
    this->cond = cond;
    this->use_list.push_back(src1);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src1->getReg());
    this->use_list.push_back(src2);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src2->getReg());
    src1->setParent(this);
    src2->setParent(this);

    p->set_op(cond);//为condbr记录opcode
    cout<<"----cond:"<<cond<<endl;
}

void CmpMInstruction::output()
{
    // TODO
    // Jsut for reg alloca test
    // delete it after test
    switch (this->kind) {
        case CmpMInstruction::CMP:
            fprintf(yyout, "\tcmp ");
            break;
        case CmpMInstruction::VCMP:
            fprintf(yyout, "\tvcmp.f32 ");
            break;
        default:
            break;
    }
    this->use_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[1]->output();
    fprintf(yyout, "\n");
}

VmrsMInstruction::VmrsMInstruction(MachineBlock* p) {
    this->parent = p;
    this->type = MachineInstruction::VMRS;
}

void VmrsMInstruction::output() {
    fprintf(yyout, "\tvmrs APSR_nzcv, FPSCR\n");
}

VcvtMInstruction::VcvtMInstruction(MachineBlock* p,
                                   int op,
                                   MachineOperand* dst,
                                   MachineOperand* src,
                                   int cond) {
    this->parent = p;
    this->type = MachineInstruction::VCVT;
    this->op = op;
    this->cond = cond;
    this->def_list.push_back(dst);
    this->use_list.push_back(src);
    if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src->getReg());
    dst->setParent(this);
    src->setParent(this);
}

void VcvtMInstruction::output() {
    switch (this->op) {
        case VcvtMInstruction::F2S:
            fprintf(yyout, "\tvcvt.s32.f32 ");
            break;
        case VcvtMInstruction::S2F:
            fprintf(yyout, "\tvcvt.f32.s32 ");
            break;
        default:
            break;
    }
    PrintCond();
    fprintf(yyout, " ");
    this->def_list[0]->output();
    fprintf(yyout, ", ");
    this->use_list[0]->output();
    fprintf(yyout, "\n");
}

StackMInstructon::StackMInstructon(MachineBlock* p, int op, 
    vector<MachineOperand*> src_list,
    int cond)
{
    // TODO
    this->parent = p;
    this->type = MachineInstruction::STACK;
    this->op = op;
    this->cond = cond;
    if (!src_list.empty()) {
        for(auto& src:src_list){
            this->use_list.push_back(src);
            cout<<"??????"<<endl;
            if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src->getReg());
            src->setParent(this);
        }
    }
}

void StackMInstructon::addSrc(vector<MachineOperand*> src_list){
    if (!src_list.empty()) {
        for(auto& src:src_list){
            this->use_list.push_back(src);
            if(this->parent&&this->parent->getParent())this->parent->getParent()->use_regnos.push_back(src->getReg());
            src->setParent(this);
        }
    }
}

void StackMInstructon::output()
{
    // TODO
    vector<MachineOperand *>copy=use_list;
    int size=copy.size();
    if(use_list.empty())return;
    if (op==PUSH||op==VPUSH) {
        int begin=0;
        int end=(size>16?16:size);
        while(end<=size){
            cout<<"pushing"<<endl;
            switch (op) {
                case PUSH:
                    fprintf(yyout, "\tpush ");
                    break;
                case VPUSH:
                    fprintf(yyout, "\tvpush ");
                    break;
                default:
                    break;
            }
            int count=0;
            fprintf(yyout, "{");
            if (!use_list.empty()) {
                for(int i=begin;i<end;i++){
                    use_list[i]->output();
                    if(i!=(end-1))
                        fprintf(yyout, ", ");
                }
            } 
            fprintf(yyout, "}\n");
            if(end==size)break;
            begin+=16;
            end+=((size-end)>16?16:(size-end));
        }
    }

    if (op==POP||op==VPOP) {
        int begin=(size%16==0?(size-16):(size/16)*16);
        int end=size;
        while(begin>=0){
            cout<<"poping"<<end<<endl;
            switch (op) {
                case POP:
                    fprintf(yyout, "\tpop ");
                    break;
                case VPOP:
                    fprintf(yyout, "\tvpop ");
                    break;
                default:
                    break;
            }
            int count=0;
            fprintf(yyout, "{");
            if (!use_list.empty()) {
                for(int i=begin;i<end;i++){
                    use_list[i]->output();
                    if(i!=(end-1))
                        fprintf(yyout, ", ");
                }
            } 
            fprintf(yyout, "}\n");
            end=begin;
            begin-=16;
        }
    }
}

// GlobalMInstruction::GlobalMInstruction(MachineBlock* p,
//     MachineOperand* dst,
//     MachineOperand* src,
//     int size,
//     int cond)
// {
//     // TODO
//     this->parent = p;
//     this->type = MachineInstruction::STACK;
//     this->op = op;
//     this->size=size;
//     this->cond = cond;
//     this->use_list.push_back(dst);
//     if (src) {
//         this->use_list.push_back(src);
//         src->setParent(this);
//     }
// }

// void GlobalMInstruction::output()
// {
//     // TODO
//     if (!this->use_list.empty()) {
//         fprintf(yyout, "\t.global %s\n", use_list[0]->getLabel().c_str());
//         fprintf(yyout, "\t.align 4\n");
//         fprintf(yyout, "\t.size %s, %d\n", use_list[0]->getLabel().c_str(), size);
//         fprintf(yyout, "%s:\n", use_list[0]->getLabel().c_str());
//         fprintf(yyout, "\t.word %s", use_list[1]->getLabel().c_str());
//     }
// }

MachineFunction::MachineFunction(MachineUnit* p, SymbolEntry* sym_ptr) 
{ 
    this->parent = p; 
    this->sym_ptr = sym_ptr; 
    this->stack_size = 0;
};

void MachineBlock::output()
{
    fprintf(yyout, ".L%d:\n", this->no);
    //cout<<"total:"<<inst_list.size()<<endl;
    int count=0;
    for(auto inst : inst_list){
        //死代码删除
        if(inst->dead)continue;
        // cout<<"count: "<<count<<endl;
        // cout<<"-----------inst?"<<(inst==nullptr)<<endl;
        // cout<<"isBinary?"<<inst->isBinary()<<endl;
        // cout<<"isbp?"<<((BinaryMInstruction*)inst)->isbp()<<endl;
        if(inst->isLoad()&&((LoadMInstruction*)inst)->isbp()){
            //说明是return语句的add sp  需要重填
            cout<<"?????"<<parent->AllocSpace(0)<<endl;
            ((LoadMInstruction*)inst)->set_src1(new MachineOperand(MachineOperand::IMM, parent->AllocSpace(0)));
        }
        if(inst->isStack()){
            //说明是return语句的add sp
            if(((StackMInstructon*)inst)->isPOP())
                ((StackMInstructon*)inst)->addSrc(parent->src_list);
            if(((StackMInstructon*)inst)->isVPOP())
                ((StackMInstructon*)inst)->addSrc(parent->v_src_list);
        }
        if(inst->isLoad()&&find(parent->stack_list.begin(),parent->stack_list.end(),inst)!=parent->stack_list.end()){
            //说明是载参数的ldr语句，需要根据saved_regs的个数重写
            cout<<"*****************isLoad!!!"<<endl;
            cout<<"num_SavedRegs():"<<parent->num_SavedRegs()<<endl;
            int base=parent->num_SavedRegs()*4+8;
            auto f=find(parent->stack_list.begin(),parent->stack_list.end(),inst);
            int d=distance(parent->stack_list.begin(), f);
            int loc=d*4+base;
            ((LoadMInstruction*)inst)->set_src2(new MachineOperand(MachineOperand::IMM, loc));
        }
        // cout<<"count: "<<count<<endl;
        inst->output();
        // cout<<"count: "<<count++<<endl;
        count++;
        if (count % 500 == 0) {
            fprintf(yyout, "\tb .B%d\n", label);
            fprintf(yyout, ".LTORG\n");
            parent->getParent()->PrintGlobalEnd();
            fprintf(yyout, ".B%d:\n", label++);
        }
    }
    if(inst_list.empty()){
        
    }
}

void MachineBlock::deadinst_mark()
{
    for(auto inst : inst_list){
        if(!inst->getDef().empty()){
            bool elim=true;
            for(auto &def:inst->getDef()){
                if(count(parent->use_regnos.begin(), parent->use_regnos.end(), def->getReg())!=0){
                    elim=false;
                }
            }
            if(elim){
                inst->dead=true;
            }
        }
    }
    if(inst_list.empty()){
        
    }
}

void MachineFunction::output()
{
    cout<<"#################Machine Fucntion######################"<<endl;
    string func_name = (const char*)(this->sym_ptr->toStr().c_str()) + 1;//what???
    cout<<func_name<<endl;
    fprintf(yyout, "\t.global %s\n", func_name.c_str());
    fprintf(yyout, "\t.type %s , %%function\n", func_name.c_str());
    fprintf(yyout, "%s:\n", func_name.c_str());
    // TODO
    /* Hint:
    *  1. Save fp
    *  2. fp = sp
    *  3. Save callee saved register
    *  4. Allocate stack space for local variable */
    
    // Traverse all the block in block_list to print assembly code.

    auto fp = new MachineOperand(MachineOperand::REG, 11);
    auto sp = new MachineOperand(MachineOperand::REG, 13);
    auto lr = new MachineOperand(MachineOperand::REG, 14);

    for(auto& reg:saved_regs){
        auto r=new MachineOperand(MachineOperand::REG, reg);
        if(reg<16)
            src_list.push_back(r);
        else
            v_src_list.push_back(r);
    }
    src_list.push_back(fp);
    src_list.push_back(lr);
    (new StackMInstructon(nullptr, StackMInstructon::PUSH, src_list))->output();
    (new StackMInstructon(nullptr, StackMInstructon::VPUSH, v_src_list))->output();
    (new MovMInstruction(nullptr, MovMInstruction::MOV, fp, sp))->output();
    int off = AllocSpace(0);
    if (off % 8 != 0) {
        off = AllocSpace(4);
    }
    if (off) {
        auto size = new MachineOperand(MachineOperand::IMM, off);  
        auto temp= new MachineOperand(MachineOperand::REG, 4);
        (new LoadMInstruction(nullptr,LoadMInstruction::LDR, temp, size))->output();
        (new BinaryMInstruction(nullptr, BinaryMInstruction::SUB, sp, sp, temp))->output();
    }
    
    int count = 0;
    for(auto iter : block_list){
        iter->output();
        count += iter->getCount();
        if (count > 160) {
            fprintf(yyout, "\tb .F%d\n", parent->n);
            fprintf(yyout, ".LTORG\n");
            parent->PrintGlobalEnd();
            fprintf(yyout, ".F%d:\n", parent->n - 1);
            count = 0;
        }
    }
}

void MachineFunction::deadinst_mark()
{
    for(auto iter : block_list){
        iter->deadinst_mark();
    }
}

void MachineUnit::PrintGlobalDecl()
{
    // TODO:
    cout<<"-------------------PrintGlobalDecl"<<endl;
    // You need to print global variable/const declarition code;
    if (!global_dst.empty()||!arr_global_dst.empty())
        fprintf(yyout, "\t.data\n");
    vector<SymbolEntry*> const_dst;
    vector<SymbolEntry*> const_src;
    for(int i=0;i<global_dst.size();i++){
        cout<<global_dst[i]->toStr()<<endl;
        if(global_src[i])cout<<global_src[i]->get_value()<<endl;
        else cout<<"nothing"<<endl;
        string name=(const char*)(((IdentifierSymbolEntry*)(global_dst[i]))->toStr().c_str())+1;
        int size=((IntType*)(global_dst[i]->getType()))->getSize()/8;
        
        if(((IntType*)(global_dst[i]->getType()))->isConst()){
            const_dst.push_back(global_dst[i]);
            const_src.push_back(global_src[i]);
            continue;
            //fprintf(yyout, "\t.section .rodata\n");
        }
        //cout<<"name????????"<<name<<endl;???为什么??????????
        fprintf(yyout, "\t.global %s\n", name.c_str());
        fprintf(yyout, "\t.align 4\n");
        fprintf(yyout, "\t.size %s, %d\n", name.c_str(), size);
        fprintf(yyout, "%s:\n", name.c_str());
        
        
        if(global_dst[i]->getType()->isFloat()){
            double val;
            if(global_src[i]&&global_src[i]->isConstant()){
                val=((ConstantSymbolEntry*)(global_src[i]))->getValue();
            }
            else{
                val=0;
            }
            cout<<val<<endl;
            uint32_t temp = reinterpret_cast<uint32_t&>(val);
            fprintf(yyout, "\t.word %u\n", temp);
        }
        else{
            string val;
            if(global_src[i]&&global_src[i]->isConstant()){
                val=global_src[i]->toStr();
            }
            else{
                val="0";
            }
            fprintf(yyout, "\t.word %s\n", val.c_str());
        }
    }

    //fprintf(yyout, "\t.section .rodata\n");
    
    for(int i=0;i<const_dst.size();i++){
        string name=(const char*)(((IdentifierSymbolEntry*)(const_dst[i]))->toStr().c_str())+1;
        int size=((IntType*)(const_dst[i]->getType()))->getSize()/8;
        
        if(((IntType*)(const_dst[i]->getType()))->isConst()){
            fprintf(yyout, "\t.section .rodata\n");
        }
        //cout<<"name????????"<<name<<endl;???为什么??????????
        fprintf(yyout, "\t.global %s\n", name.c_str());
        fprintf(yyout, "\t.align 4\n");
        fprintf(yyout, "\t.size %s, %d\n", name.c_str(), size);
        fprintf(yyout, "%s:\n", name.c_str());
        
        if(const_dst[i]->getType()->isFloat()){
            float val;
            if(const_src[i]&&const_src[i]->isConstant()){
                val=((ConstantSymbolEntry*)(const_src[i]))->getValue();
                cout<<val<<"-----------------------"<<name<<const_src[i]->get_value()<<endl;
            }
            else{
                val=0;
            }
            cout<<val<<endl;
            uint32_t temp = reinterpret_cast<uint32_t&>(val);
            fprintf(yyout, "\t.word %u\n", temp);
        }
        else{
            string val;
            if(const_src[i]&&const_src[i]->isConstant()){
                val=const_src[i]->toStr();
            }
            else{
                val="0";
            }
            fprintf(yyout, "\t.word %s\n", val.c_str());
        }
    }
    for(int i=0;i<(int)(arr_global_dst.size());i++){
        string name=(const char*)(((IdentifierSymbolEntry*)(arr_global_dst[i]))->toStr().c_str())+1;
        vector<int>dims=((ArrayType*)(arr_global_dst[i]->getType()))->get_dims();
        int size=1;
        for(auto &dim:dims){
            size*=dim;
        }
        size*=4;
        
        if(((IntType*)(((ArrayType*)(arr_global_dst[i]->getType()))->gettype()))->isConst()){
            fprintf(yyout, "\t.section .rodata\n");
        }
        //cout<<"name????????"<<name<<endl;???为什么??????????
        
        string val;
        if(arr_global_src[i].empty()){
            
            fprintf(yyout, "\t.comm %s, %d, 4 \n", name.c_str(), size);
            continue;
        }
        fprintf(yyout, "\t.global %s\n", name.c_str());
        fprintf(yyout, "\t.align 4\n");
        fprintf(yyout, "\t.size %s, %d\n", name.c_str(), size);
        fprintf(yyout, "%s:\n", name.c_str());
        for(auto& arr_gs:arr_global_src[i]){
            if(arr_gs&&arr_gs->isConstant()){
                val=arr_gs->toStr();
            }
            else{
                val="0";
            }
            fprintf(yyout, "\t.word %s\n", val.c_str());
        }

        
    }
}
void MachineUnit::PrintGlobalEnd()
{
    for(int i=0;i<(int)(global_dst.size());i++){
        const char * name=((IdentifierSymbolEntry*)(global_dst[i]))->toStr().c_str()+1;
        
        fprintf(yyout, "addr_%s%d:\n", name, n);
        fprintf(yyout, "\t.word %s\n", name);
    }
    for(int i=0;i<(int)(arr_global_dst.size());i++){
        const char * name=((IdentifierSymbolEntry*)(arr_global_dst[i]))->toStr().c_str()+1;
        
        fprintf(yyout, "addr_%s%d:\n", name, n);
        fprintf(yyout, "\t.word %s\n", name);
    }
    n++;
}

void MachineUnit::output()
{
    n=0;
    // TODO
    /* Hint:
    * 1. You need to print global variable/const declarition code;
    * 2. Traverse all the function in func_list to print assembly code;
    * 3. Don't forget print bridge label at the end of assembly code!! */
    fprintf(yyout, "\t.arch armv8-a\n");
    fprintf(yyout, "\t.arch_extension crc\n");
    fprintf(yyout, "\t.arm\n");
    PrintGlobalDecl();
    fprintf(yyout, "\t.text\n");
    
    int count=0;
    for(auto iter : func_list){
        iter->output();
        count += iter->getCount();
        if (count > 600) {
            fprintf(yyout, "\tb .F%d\n", n);
            fprintf(yyout, ".LTORG\n");
            PrintGlobalEnd();
            fprintf(yyout, ".F%d:\n", n - 1);
            count = 0;
        }
    }
    fprintf(yyout, "\n");
    PrintGlobalEnd();
}

void MachineUnit::deadinst_mark()
{
    for(auto iter : func_list){
        iter->deadinst_mark();
    }
}
