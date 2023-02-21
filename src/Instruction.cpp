#include "Instruction.h"
#include "BasicBlock.h"
#include <iostream>
#include <vector>
#include "Function.h"
#include "Type.h"
using namespace std;

extern FILE* yyout;

Operand* last_hit=nullptr;
MachineOperand* last_loc=nullptr;

map<MachineFunction*,MachineOperand*> tmp_space;

vector<MachineInstruction*> addition_list;

Instruction::Instruction(unsigned instType, BasicBlock *insert_bb)
{
    prev = next = this;
    opcode = -1;
    this->instType = instType;
    if (insert_bb != nullptr)
    {
        insert_bb->insertBack(this);
        parent = insert_bb;
    }
}

Instruction::~Instruction()
{
    parent->remove(this);
}

BasicBlock *Instruction::getParent()
{
    return parent;
}

void Instruction::setParent(BasicBlock *bb)
{
    parent = bb;
}

void Instruction::setNext(Instruction *inst)
{
    next = inst;
}

void Instruction::setPrev(Instruction *inst)
{
    prev = inst;
}

Instruction *Instruction::getNext()
{
    return next;
}

Instruction *Instruction::getPrev()
{
    return prev;
}

BinaryInstruction::BinaryInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb) : Instruction(BINARY, insert_bb)
{
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    src1->addUse(this);
    src2->addUse(this);
}

BinaryInstruction::~BinaryInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void BinaryInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    s2 = operands[1]->toStr();
    s3 = operands[2]->toStr();
    type = operands[0]->getType()->toStr();
    switch (opcode)
    {
    case ADD:
        if (type == "float"||type == "float()") {
            op = "fadd";
        } else {
            op = "add";
        }
        break;
    case SUB:
        if (type == "float"||type == "float()") {
            op = "fsub";
        } else {
            op = "sub";
        }
        break;
    case MUL:
        if (type == "float"||type == "float()") {
            op = "fmul";
        } else {
            op = "mul";
        }
        break;
    case DIV:
        if (type == "float"||type == "float()") {
            op = "fdiv";
        } else {
            op = "sdiv";
        }
        break;
    case MOD:
        op = "srem";
        break;
    case NOT:
        op = "xor";
        fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), "true");
        return;
        break;
    case SAME:
        op = "xor";
        fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), "false");
        return;
        break;
    default:
        break;
    }
    fprintf(yyout, "  %s = %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
}

CmpInstruction::CmpInstruction(unsigned opcode, Operand *dst, Operand *src1, Operand *src2, BasicBlock *insert_bb): Instruction(CMP, insert_bb){
    this->opcode = opcode;
    operands.push_back(dst);
    operands.push_back(src1);
    operands.push_back(src2);
    dst->setDef(this);
    if(src1) src1->addUse(this);
    if(src2) src2->addUse(this);
}

CmpInstruction::~CmpInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    if(operands[1])operands[1]->removeUse(this);
    if(operands[2])operands[2]->removeUse(this);
}

void CmpInstruction::output() const
{
    std::string s1, s2, s3, op, type;
    s1 = operands[0]->toStr();
    if(operands[1])s2 = operands[1]->toStr();
    else s2="0";
    if(operands[2])s3 = operands[2]->toStr();
    else s3="0";
    type = operands[1]->getType()->toStr();
        if(type=="float"){
        switch (opcode)
        {
        case E:
            op = "oeq";
            break;
        case NE:
            op = "one";
            break;
        case L:
            op = "olt";
            break;
        case LE:
            op = "ole";
            break;
        case G:
            op = "ogt";
            break;
        case GE:
            op = "oge";
            break;
        default:
            op = "";
            break;
        }
        fprintf(yyout, "  %s = fcmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
    else{
        switch (opcode)
        {
        case E:
            op = "eq";
            break;
        case NE:
            op = "ne";
            break;
        case L:
            op = "slt";
            break;
        case LE:
            op = "sle";
            break;
        case G:
            op = "sgt";
            break;
        case GE:
            op = "sge";
            break;
        default:
            op = "";
            break;
        }

        fprintf(yyout, "  %s = icmp %s %s %s, %s\n", s1.c_str(), op.c_str(), type.c_str(), s2.c_str(), s3.c_str());
    }
}

UncondBrInstruction::UncondBrInstruction(BasicBlock *to, BasicBlock *insert_bb) : Instruction(UNCOND, insert_bb)
{
    branch = to;
}

void UncondBrInstruction::output() const
{
    fprintf(yyout, "  br label %%B%d\n", branch->getNo());
}

void UncondBrInstruction::setBranch(BasicBlock *bb)
{
    branch = bb;
}

BasicBlock *UncondBrInstruction::getBranch()
{
    return branch;
}

CondBrInstruction::CondBrInstruction(BasicBlock*true_branch, BasicBlock*false_branch, Operand *cond, BasicBlock *insert_bb) : Instruction(COND, insert_bb){
    this->true_branch = true_branch;
    this->false_branch = false_branch;
    cond->addUse(this);
    operands.push_back(cond);
}

CondBrInstruction::~CondBrInstruction()
{
    operands[0]->removeUse(this);
}

void CondBrInstruction::output() const
{
    std::string cond, type;
    cond = operands[0]->toStr();
    type = operands[0]->getType()->toStr();
    int true_label = true_branch->getNo();
    int false_label = false_branch->getNo();
    fprintf(yyout, "  br %s %s, label %%B%d, label %%B%d\n", type.c_str(), cond.c_str(), true_label, false_label);
}

void CondBrInstruction::setFalseBranch(BasicBlock *bb)
{
    false_branch = bb;
}

BasicBlock *CondBrInstruction::getFalseBranch()
{
    return false_branch;
}

void CondBrInstruction::setTrueBranch(BasicBlock *bb)
{
    true_branch = bb;
}

BasicBlock *CondBrInstruction::getTrueBranch()
{
    return true_branch;
}

RetInstruction::RetInstruction(Operand *src, BasicBlock *insert_bb) : Instruction(RET, insert_bb)
{
    if(src != nullptr)
    {
        operands.push_back(src);
        src->addUse(this);
    }
}

RetInstruction::~RetInstruction()
{
    if(!operands.empty())
        operands[0]->removeUse(this);
}

void RetInstruction::output() const
{
    if(operands.empty())
    {
        fprintf(yyout, "  ret void\n");
    }
    else
    {
        std::string ret, type;
        ret = operands[0]->toStr();
        type = operands[0]->getType()->toStr();
        fprintf(yyout, "  ret %s %s\n", type.c_str(), ret.c_str());
    }
}

AllocaInstruction::AllocaInstruction(Operand *dst, SymbolEntry *se, BasicBlock *insert_bb) : Instruction(ALLOCA, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);
    this->se = se;
}

AllocaInstruction::~AllocaInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
}

void AllocaInstruction::output() const
{
    std::string dst, type;
    dst = operands[0]->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "  %s = alloca %s, align 4\n", dst.c_str(), type.c_str());
}

LoadInstruction::LoadInstruction(Operand *dst, Operand *src_addr, BasicBlock *insert_bb) : Instruction(LOAD, insert_bb)
{
    operands.push_back(dst);
    operands.push_back(src_addr);
    dst->setDef(this);
    src_addr->addUse(this);
}

LoadInstruction::~LoadInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void LoadInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string src_type;
    std::string dst_type;
    dst_type = operands[0]->getType()->toStr();
    src_type = operands[1]->getType()->toStr();
    fprintf(yyout, "  %s = load %s, %s %s, align 4\n", dst.c_str(), dst_type.c_str(), src_type.c_str(), src.c_str());
}

StoreInstruction::StoreInstruction(Operand *dst_addr, Operand *src, BasicBlock *insert_bb) : Instruction(STORE, insert_bb)
{
    operands.push_back(dst_addr);
    operands.push_back(src);
    dst_addr->addUse(this);
    src->addUse(this);
}

StoreInstruction::~StoreInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
}

void StoreInstruction::output() const
{
    std::string dst = operands[0]->toStr();
    std::string src = operands[1]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string src_type = operands[1]->getType()->toStr();

    fprintf(yyout, "  store %s %s, %s %s, align 4\n", src_type.c_str(), src.c_str(), dst_type.c_str(), dst.c_str());
}

//函数调用 call
CallInstruction::CallInstruction(SymbolEntry *symbolentry,Operand *dst,vector<Operand*> vo, BasicBlock *insert_bb):Instruction(CALL,insert_bb){
    operands.push_back(dst);
    dst->addUse(this);
    this->vo=vo;
    for(auto &o:vo){
        o->addUse(this);
    }
    names=symbolentry->toStr();
    types=((FunctionType*)(symbolentry->getType()))->paramsType;
    if(((FunctionType*)(symbolentry->getType()))->getRetType()->isInt()==1){
        isvoid=0;
        ret_type="i32";
    }
    else if(((FunctionType*)(symbolentry->getType()))->getRetType()->isFloat()==1){
        isvoid=0;
        ret_type="float";
    } 
    else if(((FunctionType*)(symbolentry->getType()))->getRetType()->isVoid()==1)
        isvoid=1;
    fprintf(stderr,"1\n");
};

CallInstruction::~CallInstruction(){
     operands[0]->removeUse(this);
};

void CallInstruction::output() const{
    //%18 = call i32 @funb(i32 1, i32 1)
    std::string str;
    std::string s1="  "+operands[0]->toStr();
    if(isvoid==0){
        str+=s1;
        str+=" = call ";
        str+=ret_type;
        str+=" ";
    }
    else
        str+="  call void ";
    str+=names;
    str+="(";
    int i=0;
    for(Operand* o:vo){
        if(i!=0){
            str+=",";
        }
        if(!o->getType()->isArray()){
            str+=o->getType()->toStr();
        }
        else{
            str+=((ArrayType*)(o->getType()))->gettype()->toStr();
            str+="*";
        }
        str+=" ";
        str+=o->toStr();
        i++;
    }
    str+=")\n";
    fprintf(yyout,"%s",str.c_str());
};

//global
GlobalInstruction::GlobalInstruction(Operand *dst, Operand *src, BasicBlock *insert_bb, string global_arr) : Instruction(GLOBAL, insert_bb)
{
    operands.push_back(dst);
    dst->setDef(this);   

    if(src){
        operands.push_back(src);
        src->addUse(this);
    }
    this->global_arr=global_arr;
}

GlobalInstruction::~GlobalInstruction()
{
    // operands[0]->setDef(nullptr);
    // if(operands[0]->usersNum() == 0)
    //     delete operands[0];
}

void GlobalInstruction::output() const
{
    string dst = operands[0]->toStr();
    string type = ((operands[1]->getType()))->toStr();

    double temp=0;
    if(operands[1]->getEntry()->isConstant()){
        temp=((ConstantSymbolEntry*)(operands[1]->getEntry()))->getValue();
    }
    uint64_t val = reinterpret_cast<uint64_t&>(temp);

    cout<<"GlobalInstruction"<<endl;
    if(global_arr=="zero"){
        type = ((PointerType*)((operands[0]->getType())))->get_valueType()->toStr();
        fprintf(yyout, "%s = global %s zeroinitializer, align 4\n", dst.c_str(), type.c_str());
    }
    else if(global_arr!=""){
        fprintf(yyout, "%s = global %s, align 4\n", dst.c_str(), global_arr.c_str());
    }
    else{
        if(operands.size()>1){
            cout<<"?"<<operands[0]->getType()->toStr()<<endl;
            
            if(operands[0]->getType()->toStr()=="float*"){
                fprintf(yyout, "%s = global %s 0x%lX, align 4\n", dst.c_str(), type.c_str(), val);
            }
            else{
                string src=operands[1]->toStr();
                //type = operands[1]->getType()->toStr();
                fprintf(yyout, "%s = global %s %s, align 4\n", dst.c_str(), type.c_str(), src.c_str());
            }
        }
        else{
            fprintf(yyout, "%s = global %s, align 4\n", dst.c_str(), type.c_str());
        }
    }
    
}

//强制转换
TypefixInstruction::TypefixInstruction(Operand* dst, Operand *src, BasicBlock *insert_bb, int kind) : Instruction(TYPEFIX, insert_bb)
{
    this->kind=kind;
    operands.push_back(dst);
    operands.push_back(src);
    dst->setDef(this);
    src->addUse(this);
}

TypefixInstruction::~TypefixInstruction()
{
    operands[0]->setDef(nullptr);
    if(operands[0]->usersNum() == 0)
        delete operands[0];
    operands[1]->removeUse(this);
}

void TypefixInstruction::output() const
{
    std::string dst, src, itype, btype;
    dst = operands[0]->toStr();
    src = operands[1]->toStr();
    if(kind==BOOL2INT){
        itype=TypeSystem::intType->toStr();
        btype=TypeSystem::boolType->toStr();
        //%6 = zext i1 %5 to i32
        fprintf(yyout, "  %s = zext %s %s to %s\n", dst.c_str(), btype.c_str(), src.c_str(), itype.c_str());
    }
    else if(kind==INT2FLOAT){
        itype=TypeSystem::intType->toStr();
        //%6 = zext i1 %5 to i32
        fprintf(yyout, "  %s = sitofp %s %s to float\n", dst.c_str(), itype.c_str(), src.c_str());
    }
    else if(kind==FLOAT2INT){
        itype=TypeSystem::intType->toStr();
        fprintf(yyout, "  %s = fptosi float %s to %s\n", dst.c_str(), src.c_str(), itype.c_str());
    }
}

//数组
ArrayItemFetchInstruction::ArrayItemFetchInstruction(Operand* tag, Type* type, Operand *dst_addr, Operand* item_addr, Operand *offset, BasicBlock *insert_bb, bool f, Operand* addr) : Instruction(ARRAYITEMFETCH, insert_bb)
{
    this->size = size;
    this->type = type;
    this->f = f;
    this->tag=tag;
    this->param_flag=false;
    this->param_addr=nullptr;
    this->memset_flag=false;
    operands.push_back(dst_addr);//一个temp
    operands.push_back(item_addr);
    operands.push_back(offset);
    //if(dst_addr)
        dst_addr->setDef(this);
    //if(item_addr)
        item_addr->addUse(this);
    //if(offset)
        offset->addUse(this);
}

ArrayItemFetchInstruction::~ArrayItemFetchInstruction()
{
    operands[0]->removeUse(this);
    operands[1]->removeUse(this);
    operands[2]->removeUse(this);
}

void ArrayItemFetchInstruction::output() const
{
    if(memset_flag){
        return;
    }
    std::string dst = operands[0]->toStr();
    std::string offset = operands[2]->toStr();
    std::string dst_type = operands[0]->getType()->toStr();
    std::string offset_type = operands[2]->getType()->toStr();

    //std::string array_type = arrayitem->getSymPtr()->getType()->toStr();
    // std::string item = (dynamic_cast<IdentifierSymbolEntry*>(arrayitem->getSymPtr()))->getAddr()->toStr();
    // std::string item_type = (dynamic_cast<IdentifierSymbolEntry*>(arrayitem->getSymPtr()))->getAddr()->getType()->toStr();
    std::string item = operands[1]->toStr();
    std::string item_type = operands[1]->getType()->toStr();

    std::string array_type=type->toStr();
    if(param_flag){
        return;
    }

    if(!f){
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, i32 0, %s %s\n", dst.c_str(), array_type.c_str(), item_type.c_str(), item.c_str(), offset_type.c_str(), offset.c_str());
    }
    else{
        fprintf(yyout, "  %s = getelementptr inbounds %s, %s %s, %s %s\n", dst.c_str(), array_type.c_str(), item_type.c_str(), item.c_str(), offset_type.c_str(), offset.c_str());
    }
}

MemsetInstruction::MemsetInstruction(Type* array_type, Operand* addr, BasicBlock* insert_bb):Instruction(MEMSET, insert_bb)
{
    this->type=array_type;
    operands.push_back(addr);
}
MemsetInstruction::~MemsetInstruction()
{
    operands[0]->removeUse(this);
}

void MemsetInstruction::output() const
{
    cout<<"------------------MemsetInstruction output-----------------------"<<endl;
}

MachineOperand* Instruction::genMachineOperand(Operand* ope)
{
    auto se = ope->getEntry();
    MachineOperand* mope = nullptr;
    if(se->isConstant())
        mope = new MachineOperand(MachineOperand::IMM, (int)dynamic_cast<ConstantSymbolEntry*>(se)->getValue(), false);
    else if(se->isTemporary())
        mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry*>(se)->getLabel(), false);
    else if(se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry*>(se);
        if(id_se->isGlobal())
            mope = new MachineOperand(id_se->toStr().c_str());
        else
            exit(0);
    }
    return mope;
}

MachineOperand* Instruction::genMachineFPOperand(Operand* ope)
{
    auto se = ope->getEntry();
    if (!se->getType()->isFloat()) {
        // error
        return genMachineOperand(ope);
    }
    MachineOperand* mope = nullptr;
    if(se->isConstant()){
        // cout<<"----------------------------------"<<(float)dynamic_cast<ConstantSymbolEntry*>(se)->getValue()<<endl;
        mope = new MachineOperand(MachineOperand::IMM, (float)dynamic_cast<ConstantSymbolEntry*>(se)->getValue(),true);
        // cout<<mope->getFVal()<<endl;
    }
    else if(se->isTemporary())
        mope = new MachineOperand(MachineOperand::VREG, dynamic_cast<TemporarySymbolEntry*>(se)->getLabel(),true);
    else if(se->isVariable())
    {
        auto id_se = dynamic_cast<IdentifierSymbolEntry*>(se);
        if(id_se->isGlobal())
            mope = new MachineOperand(id_se->toStr().c_str());
        else
            exit(0);
    }
    return mope;
}

MachineOperand* Instruction::genMachineReg(int reg) 
{
    return new MachineOperand(MachineOperand::REG, reg);
}

MachineOperand* Instruction::genMachineFReg(int freg) 
{
    return new MachineOperand(MachineOperand::REG, freg+16, true);
}

MachineOperand* Instruction::genMachineVReg(bool fp) 
{
    return new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel(), fp);
}

MachineOperand* Instruction::genMachineImm(int val) 
{
    return new MachineOperand(MachineOperand::IMM, val);
}

MachineOperand* Instruction::genMachineLabel(int block_no)
{
    std::ostringstream buf;
    buf << ".L" << block_no;
    std::string label = buf.str();
    return new MachineOperand(label);
}

void AllocaInstruction::genMachineCode(AsmBuilder* builder)
{
    /* HINT:
    * Allocate stack space for local variabel
    * Store frame offset in symbol entry */
    cout<<"AllocaInstruction~!"<<endl;
    auto cur_func = builder->getFunction();
    
    int offset;
    //数组
    if(se->getType()->isArray()){
        cout<<"isArray!"<<endl;
        int size=1;
        vector<int>dims=((ArrayType*)(se->getType()))->get_dims();
        cout<<"dims_size:"<<dims.size()<<endl;
        if(dims[0]!=0){
            for(auto &dim:dims){
                size*=dim;
            }
        }
        cout<<"size:"<<size<<endl;
        
        if(dims[0]==0)//说明是指针
            offset=cur_func->AllocSpace(4);
        else
            offset=cur_func->AllocSpace(4*size);
        
    }
    //普通变量
    else{
        offset = cur_func->AllocSpace(4);
    }
    cout<<"offset:"<<offset<<endl;
    dynamic_cast<TemporarySymbolEntry*>(operands[0]->getEntry())->setOffset(-offset);

}

void LoadInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"LoadInstruction~!"<<endl;
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst = nullptr;
    // Load global operand
    if(operands[1]->getEntry()->isVariable()
    && dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry())->isGlobal())
    {
        if (operands[0]->getType()->isFloat()){
            //cout<<1<<endl;
            auto dst = genMachineFPOperand(operands[0]);
            auto internal_reg1 = genMachineVReg();
            auto internal_reg2 = new MachineOperand(*internal_reg1);
            auto src = genMachineOperand(operands[1]);
            // example: load r0, addr_a
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg1, src);
            cur_block->InsertInst(cur_inst);
            // example: load r1, [r0]
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::VLDR, dst, internal_reg2);
            cur_block->InsertInst(cur_inst);
        }
        else{
            //cout<<1<<endl;
            auto dst = genMachineOperand(operands[0]);
            auto internal_reg1 = genMachineVReg();
            auto internal_reg2 = new MachineOperand(*internal_reg1);
            auto src = genMachineOperand(operands[1]);
            // example: load r0, addr_a
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg1, src);
            cur_block->InsertInst(cur_inst);
            // example: load r1, [r0]
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, dst, internal_reg2);
            cur_block->InsertInst(cur_inst);
        }
    }
    // Load local operand
    else if(operands[1]->getEntry()->isTemporary()
    && operands[1]->getDef()
    && operands[1]->getDef()->isAlloc())
    {
        if (operands[0]->getType()->isFloat()){
            auto dst = genMachineFPOperand(operands[0]);
            auto src1 = genMachineReg(11);
            auto src2 = genMachineImm(dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset());
            if(src2->getVal()>500||src2->getVal()<-500){
                auto temp1=genMachineVReg();
                auto temp2=genMachineVReg();
                auto fp=genMachineReg(11);
                cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, src2));
                cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp2, fp, temp1));
                cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::VLDR, dst, temp2));
            }
            else{
                cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::VLDR, dst, src1, src2);
                cur_block->InsertInst(cur_inst);
            }
        }
        else{
            //cout<<2<<endl;
            // example: load r1, [r0, #4]
            auto dst = genMachineOperand(operands[0]);
            auto src1 = genMachineReg(11);
            auto src2 = genMachineImm(dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())->getOffset());
            if(src2->getVal()>500||src2->getVal()<-500){
                auto temp1=genMachineVReg();
                auto temp2=genMachineVReg();
                auto fp=genMachineReg(11);
                cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, src2));
                cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp2, fp, temp1));
                cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, dst, temp2));
            }
            else{
                cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, dst, src1, src2);
                cur_block->InsertInst(cur_inst);
            }
        }
    }
    // Load operand from temporary variable
    else
    {
        // cout<<3<<endl;
        // example: load r1, [r0]
        MachineOperand* dst = nullptr;
        MachineOperand* src = nullptr;
        if (operands[0]->getType()->isFloat()) {
            dst = genMachineFPOperand(operands[0]);
        }
        else{
            dst = genMachineOperand(operands[0]);
        }
        src = genMachineOperand(operands[1]);
        if (operands[0]->getType()->isFloat() || operands[1]->getType()->isFloat()) {
            cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::VLDR, dst, src);
            cur_block->InsertInst(cur_inst);
        } 
        else{
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, dst, src);
            cur_block->InsertInst(cur_inst);
        }
    }
}

void StoreInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"StoreInstruction~!"<<endl;
    // TODO
    auto cur_block = builder->getBlock();
    auto cur_func=builder->getBlock()->getParent();
    MachineInstruction* cur_inst = nullptr;
    MachineOperand* src = nullptr;
    bool sf=operands[1]->getType()->isFloat();
    if(sf){
        src = genMachineFPOperand(operands[1]);
    } 
    else{
        src = genMachineOperand(operands[1]);
    }  

    //识别一下是不是参数
    Function* func=parent->getParent();
    //可以认出！
    auto f=find(func->op.begin(), func->op.end(), operands[1]);
    //cout<<"========isparam??"<<(distance(func->op.begin(), f))<<endl;
    
    //int dt=distance(func->op.begin(), f);
    int dt=0;
    if(find(func->op.begin(), func->op.end(), operands[1])!=func->op.end()){
        //distance是推断第几个参数
        if(operands[1]->getType()->isFloat()){
            cout<<"***********isFloat"<<endl;
            for(auto &o:func->op){
                if(o==operands[1])break;
                if(o->getType()->isFloat()){
                    dt++;
                }
            }
            if(dt<5){
                src=new MachineOperand(MachineOperand::REG, dt+16);
            }
            else{
                src=new MachineOperand(MachineOperand::REG, 20);
                auto fp=genMachineReg(11);
                //stack_off实际上还不能确定。。还是得回填
                auto stack_off=genMachineImm((dt-4)*4+8);
                auto src2 = genMachineImm(((TemporarySymbolEntry*)(operands[0]->getEntry()))->getOffset());
                auto stackinst=new LoadMInstruction(cur_block,LoadMInstruction::VLDR, src, fp, stack_off);
                cur_block->InsertInst(stackinst);
                cur_func->stack_list.push_back(stackinst);
                cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::VSTR, src, fp, src2));
                return;
            }
        }
        else{
            cout<<"**************notFloat"<<endl;
            cout<<operands[1]->getType()->toStr()<<endl;
            for(auto &o:func->op){
                if(o==operands[1])break;
                if(!o->getType()->isFloat()){
                    dt++;
                }
            }
            if(dt<4){
                src=new MachineOperand(MachineOperand::REG, dt);
            }
            else{
                src=new MachineOperand(MachineOperand::REG, 3);
                auto fp=genMachineReg(11);
                //stack_off实际上还不能确定。。还是得回填
                auto stack_off=genMachineImm((dt-4)*4+8);
                auto src2 = genMachineImm(((TemporarySymbolEntry*)(operands[0]->getEntry()))->getOffset());
                auto stackinst=new LoadMInstruction(cur_block,LoadMInstruction::LDR, src, fp, stack_off);
                cur_block->InsertInst(stackinst);
                cur_func->stack_list.push_back(stackinst);
                cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::STR, src, fp, src2));
                return;
            }
        }
    }
    
    // Load global operand
    if(operands[1]->getEntry()->isConstant()){
        cout<<"constant??"<<endl;
        MachineOperand* tmp=genMachineVReg(sf);
        if(sf){
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg, src);
            cur_block->InsertInst(cur_inst);
            internal_reg = new MachineOperand(*internal_reg);
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, tmp, internal_reg);
        }
        else{
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, tmp, src);
        }
        cur_block->InsertInst(cur_inst);
        src=tmp;
    }

    if(operands[0]->getEntry()->isVariable()
    && dynamic_cast<IdentifierSymbolEntry*>(operands[0]->getEntry())->isGlobal())
    {
        cout<<"isVariable"<<endl;
        auto dst = genMachineOperand(operands[0]);
        auto internal_reg1 = genMachineVReg();
        auto internal_reg2 = new MachineOperand(*internal_reg1);//加个方括号
        //auto src = genMachineOperand(operands[1]);
        // example: load r0, addr_a
        if(sf){
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg1, dst);
            cur_block->InsertInst(cur_inst);
            // example: load r1, [r0]
            cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::VSTR, src, internal_reg2);
            cur_block->InsertInst(cur_inst);
        }
        else{
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg1, dst);
            cur_block->InsertInst(cur_inst);
            // example: load r1, [r0]
            cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::STR, src, internal_reg2);
            cur_block->InsertInst(cur_inst);
        }
    }
    // Load local operand
    else if(operands[0]->getEntry()->isTemporary()
    && operands[0]->getDef()
    && operands[0]->getDef()->isAlloc())//栈内变量
    {
        cout<<"isTemporary"<<endl;
        // example: load r1, [r0, #4]
        auto dst = genMachineOperand(operands[0]);
        //auto src = genMachineOperand(operands[1]);
        auto src1 = genMachineReg(11);//fp
        //这条有问题
        //dynamic_cast为什么会有问题？？？
        // cout<<"operands[1]?"<<((TemporarySymbolEntry*)(operands[1]->getEntry())==nullptr)<<endl;
        // cout<<"offset?"<<(dynamic_cast<TemporarySymbolEntry*>(operands[1]->getEntry())==nullptr)<<endl;
        auto src2 = genMachineImm(((TemporarySymbolEntry*)(operands[0]->getEntry()))->getOffset());
        if(src2->getVal()>500||src2->getVal()<-500){
            auto temp1=genMachineVReg();
            auto temp2=genMachineVReg();
            auto fp=genMachineReg(11);
            cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, src2));
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp2, fp, temp1));
            if(sf){
                cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::VSTR, src, temp2));
            }
            else{
                cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::STR, src, temp2));
            }
        }
        else{     
            if(sf){
                cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::VSTR, src, src1, src2);
                cur_block->InsertInst(cur_inst);
            }
            else{  
                cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::STR, src, src1, src2);
                cur_block->InsertInst(cur_inst);
            }
        }
    }
    // Load operand from temporary variable
    else
    {
        cout<<"neither"<<endl;
        // example: load r1, [r0]
        auto dst = genMachineOperand(operands[0]);
        //auto src = genMachineOperand(operands[1]);
        if(sf){
            cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::VSTR, src, dst);
            cur_block->InsertInst(cur_inst);
        }
        else{
            cur_inst = new StoreMInstruction(cur_block,StoreMInstruction::STR, src, dst);
            cur_block->InsertInst(cur_inst);
        }
    }
    cout<<"store end---"<<endl;
}

void BinaryInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"BinaryInstruction~!"<<endl;
    MachineInstruction* cur_inst = nullptr;
    // TODO:
    // complete other instructions
    auto cur_block = builder->getBlock();
    if (operands[0]->getType()->isFloat()){
        bool mov_flag=false;
        auto dst = genMachineFPOperand(operands[0]);
        auto src1 = genMachineFPOperand(operands[1]);
        auto src2 = genMachineFPOperand(operands[2]);
        if(operands[1]->getEntry()->isConstant()){
            //cout<<"constant??"<<endl;
            MachineOperand* tmp1=genMachineVReg(true);
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src1);
            cur_block->InsertInst(cur_inst);
            internal_reg = new MachineOperand(*internal_reg);
            cur_inst = new MovMInstruction(cur_block,MovMInstruction::VMOV, tmp1, internal_reg);
            cur_block->InsertInst(cur_inst);
            src1=new MachineOperand(*tmp1);
        }
        if(operands[2]->getEntry()->isConstant()){
            //cout<<"constant??"<<endl;
            if (src2->getFVal() == 0 && opcode == ADD)
                mov_flag = true;
            else{
                MachineOperand* tmp2=genMachineVReg(true);
                auto internal_reg = genMachineVReg();
                cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src2);
                cur_block->InsertInst(cur_inst);
                internal_reg = new MachineOperand(*internal_reg);
                cur_inst = new MovMInstruction(cur_block,MovMInstruction::VMOV, tmp2, internal_reg);
                cur_block->InsertInst(cur_inst);
                src2=new MachineOperand(*tmp2);
            }
        }
        /* HINT:
        * The source operands of ADD instruction in ir code both can be immediate num.
        * However, it's not allowed in assembly code.
        * So you need to insert LOAD/MOV instrucrion to load immediate num into register.
        * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/

        // if(src1->isImm())
        // {
        //     auto internal_reg = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg, src1);
        //     cur_block->InsertInst(cur_inst);
        //     src1 = new MachineOperand(*internal_reg);
        // }
        // if(src2->isImm())
        // {
        //     auto internal_reg = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
        //     cur_block->InsertInst(cur_inst);
        //     src2 = new MachineOperand(*internal_reg);
        // }
        

        //用于xor
        auto tr=genMachineImm(1);
        auto fl=genMachineImm(0);

        switch (opcode)
        {
        case ADD:
            if (mov_flag){
                cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOVF32, dst, src1);
            }
            else{
                cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::VADD, dst, src1, src2);
            }
            break;
        case SUB:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::VSUB, dst, src1, src2);
            break;
        case MUL:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::VMUL, dst, src1, src2);
            break;
        case DIV:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::VDIV, dst, src1, src2);
            break;
        default:
            break;
        }
        if(cur_inst)
            cur_block->InsertInst(cur_inst);
    }
    else{
        auto dst = genMachineOperand(operands[0]);
        auto src1 = genMachineOperand(operands[1]);
        auto src2 = genMachineOperand(operands[2]);
        if(operands[1]->getEntry()->isConstant()){
            //cout<<"constant??"<<endl;
            MachineOperand* tmp1=genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, tmp1, src1);
            cur_block->InsertInst(cur_inst);
            src1=tmp1;
        }
        if(operands[2]->getEntry()->isConstant()){
            //cout<<"constant??"<<endl;
            MachineOperand* tmp2=genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, tmp2, src2);
            cur_block->InsertInst(cur_inst);
            src2=tmp2;
        }
        /* HINT:
        * The source operands of ADD instruction in ir code both can be immediate num.
        * However, it's not allowed in assembly code.
        * So you need to insert LOAD/MOV instrucrion to load immediate num into register.
        * As to other instructions, such as MUL, CMP, you need to deal with this situation, too.*/

        // if(src1->isImm())
        // {
        //     auto internal_reg = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg, src1);
        //     cur_block->InsertInst(cur_inst);
        //     src1 = new MachineOperand(*internal_reg);
        // }
        // if(src2->isImm())
        // {
        //     auto internal_reg = genMachineVReg();
        //     cur_inst = new LoadMInstruction(cur_block, internal_reg, src2);
        //     cur_block->InsertInst(cur_inst);
        //     src2 = new MachineOperand(*internal_reg);
        // }
        
        MachineOperand* temp1 = genMachineVReg();
        MachineOperand* temp2 = genMachineVReg();

        //用于xor
        auto tr=genMachineImm(1);
        auto fl=genMachineImm(0);

        switch (opcode)
        {
        case ADD:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, src1, src2);
            break;
        case SUB:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, src2);
            break;
        case MUL:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, dst, src1, src2);
            break;
        case DIV:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, dst, src1, src2);
            break;
        case MOD:
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::DIV, temp1, src1, src2);
            cur_block->InsertInst(cur_inst);
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, temp2, temp1, src2);
            cur_block->InsertInst(cur_inst);
            cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::SUB, dst, src1, temp2);
            break;
        case NOT:
            //cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::XOR, dst, src1, fl);
            cur_block->InsertInst(new CmpMInstruction(cur_block,CmpMInstruction::CMP, src1, fl, CmpInstruction::E));
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOVEQ, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOVNE, dst, fl));
            break;
        case SAME:
            //cur_inst = new BinaryMInstruction(cur_block, BinaryMInstruction::XOR, dst, src1, tr);
            break;
        default:
            break;
        }
        if(cur_inst)
            cur_block->InsertInst(cur_inst);
    }
}

void CmpInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"CmpInstruction~!"<<endl;
    // TODO
    auto cur_block = builder->getBlock();
    MachineOperand* src1, *src2;

    MachineInstruction* cur_inst = nullptr;
    if(operands[1]->getType()->isFloat()){
        cout<<"isFloat??"<<endl;
        src1 = genMachineFPOperand(operands[1]);
        src2 = genMachineFPOperand(operands[2]);
        if (src1->isImm()) {
            auto tmp1 = genMachineVReg(true);
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src1);
            cur_block->InsertInst(cur_inst);
            internal_reg = new MachineOperand(*internal_reg);
            cur_inst = new MovMInstruction(cur_block,MovMInstruction::VMOV, tmp1, internal_reg);
            cur_block->InsertInst(cur_inst);
            src1 = new MachineOperand(*tmp1);
        }
        if (src2->isImm()) {
            auto tmp2 = genMachineVReg(true);
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src2);
            cur_block->InsertInst(cur_inst);
            internal_reg = new MachineOperand(*internal_reg);
            cur_inst = new MovMInstruction(cur_block,MovMInstruction::VMOV, tmp2, internal_reg);
            cur_block->InsertInst(cur_inst);
            src2 = new MachineOperand(*tmp2);
        }
        cur_inst = new CmpMInstruction(cur_block,CmpMInstruction::VCMP, src1, src2, opcode);
        cur_block->InsertInst(cur_inst);
        //这有啥用
        cur_inst = new VmrsMInstruction(cur_block);
        cur_block->InsertInst(cur_inst);
    }
    else{
        src1 = genMachineOperand(operands[1]);
        src2 = genMachineOperand(operands[2]);
        if (src1->isImm()) {
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src1);
            cur_block->InsertInst(cur_inst);
            src1 = new MachineOperand(*internal_reg);
        }
        if (src2->isImm()) {
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block,LoadMInstruction::LDR, internal_reg, src2);
            cur_block->InsertInst(cur_inst);
            src2 = new MachineOperand(*internal_reg);
        }
        cur_inst = new CmpMInstruction(cur_block,CmpMInstruction::CMP, src1, src2, opcode);
        cur_block->InsertInst(cur_inst);
    }

    //保存必要的运算结果
    auto dst=genMachineOperand(operands[0]);
    cout<<"***"<<dst->getReg()<<endl;
    cout<<operands[0]->getType()->isFloat()<<endl;
    // auto dst=genMachineVReg();
    auto tr=genMachineImm(1);
    auto fl=genMachineImm(0);
    switch (cur_block->get_op())
    {
        case CmpMInstruction::E:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVEQ, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVNE, dst, fl));
            break;
        case CmpMInstruction::NE:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVNE, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVEQ, dst, fl));
            break;
        case CmpMInstruction::L:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVLT, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVGE, dst, fl));
            break;
        case CmpMInstruction::LE:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVLE, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVGT, dst, fl));
            break;
        case CmpMInstruction::G:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVGT, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVLE, dst, fl));
            break;
        case CmpMInstruction::GE:
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVGE, dst, tr));
            cur_block->InsertInst(new MovMInstruction(cur_block,MovMInstruction::MOVLT, dst, fl));
            break;
        default:
            break;
    }
    // cur_inst=new MovMInstruction(cur_block,op, dst, tr);
    // cur_block->InsertInst(cur_inst);
}

void UncondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"UncondInstruction~!"<<endl;
    // TODO
    auto cur_block = builder->getBlock();
    std::stringstream ss;
    ss << ".L" << branch->getNo();
    MachineOperand* dst = new MachineOperand(ss.str());
    MachineInstruction* cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
    cout<<"end??"<<endl;
}

void CondBrInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"CondInstruction~!"<<endl;
    // TODO
    auto cur_block = builder->getBlock();
    std::stringstream truess, falsess;
    //true 有条件
    truess << ".L" << true_branch->getNo();
    MachineOperand* dst = new MachineOperand(truess.str());
    MachineInstruction* cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst, cur_block->get_op());
    cur_block->InsertInst(cur_inst);
    //false 无条件
    falsess << ".L" << false_branch->getNo();
    dst = new MachineOperand(falsess.str());
    cur_inst = new BranchMInstruction(cur_block, BranchMInstruction::B, dst);
    cur_block->InsertInst(cur_inst);
}

void RetInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"RetInstruction~!"<<endl;
    // TODO
    /* HINT:
    * 1. Generate mov instruction to save return value in r0
    * 2. Restore callee saved registers and sp, fp
    * 3. Generate bx instruction */
    auto cur_block = builder->getBlock();
    MachineInstruction* cur_inst=nullptr;
   //存返回值
    if(operands.size()>0){
        if (operands[0]->getType()->isFloat()) {
            MachineOperand* r0 = new MachineOperand(MachineOperand::REG, 16);//0号寄存器
            MachineOperand* src = genMachineFPOperand(operands[0]);
            if (src->isImm()) {
                auto internal_reg = genMachineVReg();
                cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg, src);
                cur_block->InsertInst(cur_inst);
                src = internal_reg;
            }
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, r0, src);//返回值放入0号寄存器
            cur_block->InsertInst(cur_inst);
        }
        else{
            MachineOperand* r0 = new MachineOperand(MachineOperand::REG, 0);//0号寄存器
            MachineOperand* src = genMachineOperand(operands[0]);
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::MOV, r0, src);//返回值放入0号寄存器
            cur_block->InsertInst(cur_inst);
        }
    }
    //add指令 sp
    MachineFunction* cur_func = builder->getFunction();
    auto sp = new MachineOperand(MachineOperand::REG, 13);//sp为13号寄存器
    cout<<cur_func->AllocSpace(0)<<endl;
    auto size = new MachineOperand(MachineOperand::IMM, cur_func->AllocSpace(0));
    auto temp=genMachineVReg();
    cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp, size, nullptr, MachineInstruction::NONE, true));
    MachineInstruction* add_sp = new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp, sp, temp);//bp置位！
    cur_block->InsertInst(add_sp);

    //对称pop
    auto fp = new MachineOperand(MachineOperand::REG, 11);
    auto lr = new MachineOperand(MachineOperand::REG, 14);
    vector<MachineOperand*> src_list;
    // src_list.push_back(fp);
    // src_list.push_back(lr);
    MachineInstruction* vpop = new StackMInstructon(nullptr, StackMInstructon::VPOP, src_list);//后面填
    cur_block->InsertInst(vpop);
    MachineInstruction* pop = new StackMInstructon(nullptr, StackMInstructon::POP, src_list);//后面填
    cur_block->InsertInst(pop);

    //bx lr
    MachineInstruction* bx_lr = new BranchMInstruction(cur_block, BranchMInstruction::BX, lr);
    cur_block->InsertInst(bx_lr);
    cout<<"Ret end??"<<endl;
}

void CallInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"--------------------------------------------CallInstruction~!"<<endl;
    //Todo
    auto cur_block=builder->getBlock();
    MachineInstruction* cur_inst=nullptr;

    int icount=0, fcount=0;
    int isum=0, fsum=0;
    for(auto& v:vo){
        if(v->getType()->isFloat()){
            fsum++;
        }
        else{
            isum++;
        }
    }
        //放参数
    int c=0;
    int cr=vo.size()-1;
    int push_num=0;
    while(c<vo.size()){
        if(vo[c]->getType()->isFloat()){
            if(fcount<5){
                auto reg=new MachineOperand(MachineOperand::REG, fcount+16);
                auto param=genMachineFPOperand(vo[c]);
                if(param->isImm()){
                    auto internal_reg=genMachineVReg();
                    auto tmp=genMachineVReg(true);
                    cur_inst=new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg, param);
                    cur_block->InsertInst(cur_inst);
                    internal_reg=new MachineOperand(*internal_reg);
                    cur_inst=new MovMInstruction(cur_block, MovMInstruction::VMOV, tmp, internal_reg);
                    cur_block->InsertInst(cur_inst);
                    param=new MachineOperand(*tmp);
                }
                cur_inst=new MovMInstruction(cur_block, MovMInstruction::VMOV, reg, param);
                cur_block->InsertInst(cur_inst);
            }
            else{
                auto param=genMachineFPOperand(vo[cr]);
                if(param->isImm()){
                    auto internal_reg=genMachineVReg();
                    auto tmp=genMachineVReg(true);
                    cur_inst=new LoadMInstruction(cur_block, LoadMInstruction::LDR, internal_reg, param);
                    cur_block->InsertInst(cur_inst);
                    internal_reg=new MachineOperand(*internal_reg);
                    cur_inst=new MovMInstruction(cur_block, MovMInstruction::VMOV, tmp, internal_reg);
                    cur_block->InsertInst(cur_inst);
                    param=new MachineOperand(*tmp);
                }
                vector<MachineOperand*> push_list;
                push_list.push_back(param);
                cur_block->InsertInst(new StackMInstructon(cur_block, StackMInstructon::VPUSH, push_list));
                push_num++;
                cr--;
            }
            fcount++;
        }
        else{
            if(icount<4){
                auto reg=new MachineOperand(MachineOperand::REG, icount);
                auto param=genMachineOperand(vo[c]);
                cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, reg, param);
                cur_block->InsertInst(cur_inst);
            }
            else{
                auto param=genMachineOperand(vo[cr]);
                if(param->isImm()){
                    auto temp=genMachineVReg();
                    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, temp, param));
                    param=temp;
                }
                vector<MachineOperand*> push_list;
                push_list.push_back(param);
                cur_block->InsertInst(new StackMInstructon(cur_block, StackMInstructon::PUSH, push_list));
                push_num++;
                cr--;
            }
            icount++;
        }       
        c++;
        
    }

    
    //bl func
    const char *func_name = names.c_str() + 1;
    auto func_dst=new MachineOperand(func_name, true);
    cur_inst=new BranchMInstruction(cur_block, BranchMInstruction::BL, func_dst);
    cur_block->InsertInst(cur_inst);

    //add sp, sp, #..
    if(push_num){
        auto sp = new MachineOperand(MachineOperand::REG, 13);//sp为13号寄存器
        cur_inst=new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp, sp, genMachineImm(push_num*4));
        cur_block->InsertInst(cur_inst);
    }

    if(ret_type=="float"){
        auto ret_dst=genMachineFPOperand(operands[0]);
        auto s0=new MachineOperand(MachineOperand::REG, 16);
        cur_inst=new MovMInstruction(cur_block, MovMInstruction::VMOV, ret_dst, s0);
        cur_block->InsertInst(cur_inst);
    }
    else if(ret_type=="i32"){
        //mov r0 ..
        auto ret_dst=genMachineOperand(operands[0]);
        auto r0=new MachineOperand(MachineOperand::REG, 0);
        cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, ret_dst, r0);
        cur_block->InsertInst(cur_inst);
    }

    // if(vo.size()<=4){
    //     //放参数
    //     vector<MachineOperand*> mvo;
    //     int c=0;
    //     while(c<vo.size()){
    //         auto reg=new MachineOperand(MachineOperand::REG, c);
    //         auto param=genMachineOperand(vo[c]);
    //         cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, reg, param);
    //         cur_block->InsertInst(cur_inst);
    //         c++;
    //     }
    //     //bl func
    //     const char *func_name = names.c_str() + 1;
    //     auto func_dst=new MachineOperand(func_name, true);
    //     cur_inst=new BranchMInstruction(cur_block, BranchMInstruction::BL, func_dst);
    //     cur_block->InsertInst(cur_inst);
    //     //mov r0 ..
    //     auto ret_dst=genMachineOperand(operands[0]);
    //     auto r0=new MachineOperand(MachineOperand::REG, 0);
    //     cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, ret_dst, r0);
    //     cur_block->InsertInst(cur_inst);
    // }
    // else{
    //     int c=0;
    //     while(c<4){
    //         auto reg=new MachineOperand(MachineOperand::REG, c);
    //         cout<<vo[c]->getEntry()->toStr()<<endl;
    //         auto param=genMachineOperand(vo[c]);
    //         cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, reg, param);
    //         cur_block->InsertInst(cur_inst);
    //         c++;
    //     }

    //     //push...
    //     c=vo.size()-1;
    //     while(c>3){
    //         auto param=genMachineOperand(vo[c]);
    //         if(param->isImm()){
    //             auto temp=genMachineVReg();
    //             cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, temp, param));
    //             param=temp;
    //         }
    //         cout<<vo[c]->getEntry()->toStr()<<endl;
    //         vector<MachineOperand*> push_list;
    //         push_list.push_back(param);
    //         cur_block->InsertInst(new StackMInstructon(cur_block, StackMInstructon::PUSH, push_list));
    //         c--;
    //     }
        
        
    //     //bl func
    //     const char *func_name = names.c_str() + 1;
    //     auto func_dst=new MachineOperand(func_name, true);
    //     cur_inst=new BranchMInstruction(cur_block, BranchMInstruction::BL, func_dst);
    //     cur_block->InsertInst(cur_inst);

    //     //add sp, sp, #..
    //     auto sp = new MachineOperand(MachineOperand::REG, 13);//sp为13号寄存器
    //     auto push_num=genMachineImm((vo.size()-4)*4);
    //     cur_inst=new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, sp, sp, push_num);
    //     cur_block->InsertInst(cur_inst);

    //     //mov r0 ..
    //     auto ret_dst=genMachineOperand(operands[0]);
    //     auto r0=new MachineOperand(MachineOperand::REG, 0);
    //     cur_inst=new MovMInstruction(cur_block, MovMInstruction::MOV, ret_dst, r0);
    //     cur_block->InsertInst(cur_inst);
    // }
    cout<<"call end??"<<endl;
}

void GlobalInstruction::genMachineCode(AsmBuilder* builder)
{
    // cout<<"GlobalInstruction~!"<<endl;
    // auto cur_block = builder->getBlock();
    // MachineOperand* dst=new MachineOperand(operands[0]->toStr());
    // MachineOperand* src=nullptr;
    // if(operands.size()>1){
    //     src=new MachineOperand(operands[1]->toStr());
    // }
    // int size=((IntType*)(operands[0]->getEntry()->getType()))->getSize()/8;    
    // //cur_block->InsertInst(new GlobalMInstruction(cur_block, dst, src, size));
    // builder->getUnit()->globalList.push_back(new GlobalMInstruction(cur_block, dst, src, size));
    // cout<<"hi"<<endl;
}

void TypefixInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"TypefixInstruction~!"<<endl;
    auto cur_block=builder->getBlock();
    MachineInstruction* cur_inst=nullptr;
    
    
    auto tr=genMachineImm(1);
    int op;

    if(kind==FLOAT2INT){
        auto src_operand = genMachineFPOperand(operands[1]);
        auto dst_operand = genMachineOperand(operands[0]);

        if (src_operand->isImm()) {
            auto tmp = genMachineVReg(true);
            auto internal_reg = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR,
                                            internal_reg, src_operand);
            cur_block->InsertInst(cur_inst);
            internal_reg = new MachineOperand(*internal_reg);
            cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, tmp,
                                        internal_reg);
            cur_block->InsertInst(cur_inst);
            src_operand = tmp;
        }
        auto vcvtDst = genMachineVReg(true);
        cur_inst = new VcvtMInstruction(cur_block, VcvtMInstruction::F2S, vcvtDst,
                                        src_operand);
        cur_block->InsertInst(cur_inst);
        auto movUse = new MachineOperand(*vcvtDst);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV,
                                    dst_operand, movUse);

        cur_block->InsertInst(cur_inst);
    }
    else if(kind==INT2FLOAT){
        auto src_operand = genMachineOperand(operands[1]);

        if (src_operand->isImm()) {
            auto tmp = genMachineVReg();
            cur_inst = new LoadMInstruction(cur_block, LoadMInstruction::LDR, tmp,
                                            src_operand);
            cur_block->InsertInst(cur_inst);
            src_operand = new MachineOperand(*tmp);
        }
        auto movDst = genMachineVReg(true);
        cur_inst = new MovMInstruction(cur_block, MovMInstruction::VMOV, movDst,
                                    src_operand);
        cur_block->InsertInst(cur_inst);
        auto vcvtUse = new MachineOperand(*movDst);
        auto dst_operand = genMachineFPOperand(operands[0]);
        cur_inst = new VcvtMInstruction(cur_block, VcvtMInstruction::S2F,
                                        dst_operand, vcvtUse);
        cur_block->InsertInst(cur_inst);
    }
    else{
        auto dst=genMachineOperand(operands[0]);
        auto src=genMachineOperand(operands[1]);

        //{ E, NE, L, LE , G, GE, NONE }
        switch (cur_block->get_op())
        {
        case CmpMInstruction::E:
            op=MovMInstruction::MOVEQ;
            break;
        case CmpMInstruction::NE:
            op=MovMInstruction::MOVNE;
            break;
        case CmpMInstruction::L:
            op=MovMInstruction::MOVLT;
            break;
        case CmpMInstruction::LE:
            op=MovMInstruction::MOVLE;
            break;
        case CmpMInstruction::G:
            op=MovMInstruction::MOVGT;
            break;
        case CmpMInstruction::GE:
            op=MovMInstruction::MOVGE;
            break;
        default:
            break;
        }

        cout<<"op??"<<op<<endl;
        cur_inst=new MovMInstruction(cur_block,MovMInstruction::MOV, dst, src);

        cur_block->InsertInst(cur_inst);
    }
}

void ArrayItemFetchInstruction::genMachineCode(AsmBuilder* builder)
{
    cout<<"ArrayItemFetchInstruction~!"<<endl;
    auto cur_block=builder->getBlock();
    auto temp1=genMachineVReg();
    auto fp=new MachineOperand(MachineOperand::REG, 11);
    auto sp=new MachineOperand(MachineOperand::REG, 13);

    if(memset_flag){
        cout<<"memsetflag tested***************"<<endl;
        vector<int>dims;
        int total=1;
        if(type->isArray()){
            dims=((ArrayType*)type)->get_dims();
            while(!dims.empty()){
                total*=dims.front();
                dims.erase(dims.begin());
            }
        }
        cout<<"total:"<<total<<endl;
        total*=4;
        cout<<operands.size()<<endl;
        auto loc=genMachineImm(((TemporarySymbolEntry*)(operands[0]->getEntry()))->getOffset());
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, loc));
        auto r0=genMachineReg(0);
        auto r1=genMachineReg(1);
        auto r2=genMachineReg(2);
        auto init_val=genMachineImm(0);
        auto range=genMachineImm(total);
        cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, r0, fp, temp1));
        cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, r1, init_val));
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, r2, range));
        auto memset_dst=new MachineOperand("memset", true);
        cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::BL, memset_dst));
        return;
    }



    MachineInstruction* cur_inst=nullptr;

    auto dst=genMachineOperand(operands[0]);
    auto item=genMachineOperand(operands[1]);
    auto offset=genMachineOperand(operands[2]);
        MachineOperand* temp2;

    bool flag=false;
    auto loc=genMachineImm(((TemporarySymbolEntry*)(operands[1]->getEntry()))->getOffset());
    
    if(last_hit!=tag){
        cout<<"miss!!"<<endl;

        
        temp2=genMachineVReg();
        
        
        if(operands[1]->getEntry()->isVariable()
        && dynamic_cast<IdentifierSymbolEntry*>(operands[1]->getEntry())->isGlobal()){
            cout<<"global tested------------"<<endl;
            cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp2, item));
        }
        else if(param_flag){
            cout<<"param_flag tested"<<endl;
            temp2=item;
        }
        else{
            flag=true;
            cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, loc));
            cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp2, fp, temp1));
            //temp2存了数组首地址----
        }
    }
    else{
        cout<<"hit!!"<<endl;
        temp2=last_loc;
    }
    //！！把temp2存到内存里，防止被改
    auto curr_func=cur_block->getParent();
    if(tmp_space.empty()||!tmp_space.count(curr_func)){   
        int o=curr_func->AllocSpace(4);
        tmp_space[curr_func]=genMachineImm(-1*o);
    }
    if(tmp_space[curr_func]->getVal()>1000||tmp_space[curr_func]->getVal()<-1000){
        auto t1=genMachineReg(1);
        auto t2=genMachineReg(2);//如果用vreg还是会冲突，直接用reg吧，反正参数都是在后面设的
        auto fp=genMachineReg(11);
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, t1, tmp_space[curr_func]));
        cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, t2, fp, t1));
        cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::STR, temp2, t2));
    }
    else{
        cur_block->InsertInst(new StoreMInstruction(cur_block,StoreMInstruction::STR, temp2, fp, tmp_space[curr_func]));
    }
    //下面计算元素的具体地址
    //一定是从大开始调到小
    vector<int>dims;
    int total=1;
    if(type->isArray()){
        dims=((ArrayType*)type)->get_dims();
        dims.erase(dims.begin());
        while(!dims.empty()){
            total*=dims.front();
            dims.erase(dims.begin());
        }
    }
    auto temp3=genMachineVReg();
    auto off=genMachineImm(total*4);
    if(type->isArray()){
        dims=((ArrayType*)type)->get_dims();
        if(!dims.empty()&&last_hit!=tag&&param_flag){
            off=genMachineImm(dims[0]*4);
        }
    }
    //off目前是Imm，所以要加多一条mov
    auto temp_off=genMachineVReg();
    cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp_off, off));
    auto temp_offset=genMachineVReg();
    if(offset->isImm()){
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp_offset, offset));
    }
    else{
        temp_offset=offset;
    }
    cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, temp3, temp_offset, temp_off));
    
    // if(flag){
    //     cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, temp1, loc));
    //     cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, temp2, fp, temp1));
    // }

    //从内存中加载temp2
    if(tmp_space[curr_func]->getVal()>1000||tmp_space[curr_func]->getVal()<-1000){
        auto t1=genMachineReg(1);
        auto t2=genMachineReg(2);
        auto fp=genMachineReg(11);
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, t1, tmp_space[curr_func]));
        cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, t2, fp, t1));
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp2, t2));
    }
    else{
        cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp2, fp, tmp_space[curr_func]));
    }
    cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, temp2, temp3));
    
    last_hit=tag;
    last_loc=dst;
    // //计算item地址
    // auto size=genMachineImm(4);
    // auto temp2=genMachineVReg();
    // cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, temp2, size));
    // auto item_off=genMachineVReg();
    // auto off=genMachineOperand(operands[2]);//offset
    // if(operands[2]->getEntry()->isConstant()){
    //     //cout<<"constant??"<<endl;
    //     MachineOperand* tmp=genMachineVReg();
    //     cur_inst = new LoadMInstruction(cur_block, tmp, off);
    //     cur_block->InsertInst(cur_inst);
    //     off=tmp;
    // }
    // cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::MUL, item_off, temp2, off));

    // auto item_addr=genMachineVReg();
    // //计算head
    // auto temp1=genMachineVReg();
    // auto head=genMachineVReg();
    // auto fp=new MachineOperand(MachineOperand::REG, 11);
    // if(!f){       
    //     cur_block->InsertInst(new LoadMInstruction(cur_block, temp1, offset));
    //     cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, head, fp, temp1));
    // }
    // else{
    //     offset=genMachineImm(dynamic_cast<TemporarySymbolEntry*>(this->addr->getEntry())->getOffset());
    //     head = genMachineOperand(operands[1]);
    // }
    // cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, dst, head, item_off));

}

void MemsetInstruction::genMachineCode(AsmBuilder* builder){
    cout<<"MemsetInstruction~!"<<endl;
    auto cur_block=builder->getBlock();
    auto temp1=genMachineVReg();
    auto fp=new MachineOperand(MachineOperand::REG, 11);
    auto sp=new MachineOperand(MachineOperand::REG, 13);

    cout<<"memsetflag tested***************"<<endl;
    vector<int>dims;
    int total=1;
    if(type->isArray()){
        dims=((ArrayType*)type)->get_dims();
        while(!dims.empty()){
            total*=dims.front();
            dims.erase(dims.begin());
        }
    }
    cout<<"total:"<<total<<endl;
    total*=4;
    cout<<operands.size()<<endl;
    auto loc=genMachineImm(((TemporarySymbolEntry*)(operands[0]->getEntry()))->getOffset());
    cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, temp1, loc));
    auto r0=genMachineReg(0);
    auto r1=genMachineReg(1);
    auto r2=genMachineReg(2);
    auto init_val=genMachineImm(0);
    auto range=genMachineImm(total);
    cur_block->InsertInst(new BinaryMInstruction(cur_block, BinaryMInstruction::ADD, r0, fp, temp1));
    cur_block->InsertInst(new MovMInstruction(cur_block, MovMInstruction::MOV, r1, init_val));
    cur_block->InsertInst(new LoadMInstruction(cur_block,LoadMInstruction::LDR, r2, range));
    auto memset_dst=new MachineOperand("memset", true);
    cur_block->InsertInst(new BranchMInstruction(cur_block, BranchMInstruction::BL, memset_dst));

}