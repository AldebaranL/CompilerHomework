#include "Ast.h"
#include "SymbolTable.h"
#include "Unit.h"
#include "Instruction.h"
#include "IRBuilder.h"
#include <string>
#include "Type.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <deque>
using namespace std;

extern FILE *yyout;
int Node::counter = 0;
IRBuilder* Node::builder = nullptr;

int ret_label;
Operand *ret_ope;
Operand *ret_dst;
int times=0;

bool ret_alarm=false;

Operand *const_1 = new Operand(new ConstantSymbolEntry(TypeSystem::intType, 1));
Operand *float_const_1 = new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 1));
Operand *const_0=new Operand(new ConstantSymbolEntry(TypeSystem::intType, 0));
Operand *float_const_0=new Operand(new ConstantSymbolEntry(TypeSystem::floatType, 0));

Node::Node()
{
    seq = counter++;
}

void Node::backPatch(std::vector<Instruction*> &list, BasicBlock*bb, bool branch)
{
    for(auto &inst:list)
    {
        if(inst->isCond())
        //dynamic_cast 强制类型转换操作符
            if(branch)
                dynamic_cast<CondBrInstruction*>(inst)->setTrueBranch(bb);
            else
                dynamic_cast<CondBrInstruction*>(inst)->setFalseBranch(bb);
        else if(inst->isUncond())
            dynamic_cast<UncondBrInstruction*>(inst)->setBranch(bb);
    }
}

std::vector<Instruction*> Node::merge(std::vector<Instruction*> &list1, std::vector<Instruction*> &list2)
{
    std::vector<Instruction*> res(list1);
    res.insert(res.end(), list2.begin(), list2.end());
    return res;
}

void Ast::genCode(Unit *unit)
{
    IRBuilder *builder = new IRBuilder(unit);
    Node::setIRBuilder(builder);
    root->genCode();
}

void FunctionDef::genCode()
{
    cout<<"functiondef~!"<<endl;
    cout<<se->toStr()<<endl;
    Unit *unit = builder->getUnit();
    Function *func = new Function(unit, se); 

    // BasicBlock* ret_bb = new BasicBlock(func);
    // std::vector<BasicBlock*>::iterator endblock=func->end();
    // new UncondBrInstruction((BasicBlock*)(*endblock), ret_bb);
    // cout<<"bb_num: "<<ret_bb->getNo()<<endl;
    // new StoreInstruction(ret_ope, ret_dst, ret_bb);
    // new RetInstruction(ret_ope,ret_bb);
    
    //???
    func->ret_bb=new BasicBlock(func);
    
    BasicBlock *entry = func->getEntry();
    // set the insert point to the entry basicblock of this function.
    builder->setInsertBB(entry);

    //处理返回值 为返回值分配空间
    SymbolEntry *addr_se;
    Type *type, *ret_type;
    
    ret_type=((FunctionType*)(se->getType()))->getRetType();
    bool isvoid=(ret_type->isVoid());
    type = new PointerType(ret_type);

    //设置返回值label
    ret_label=SymbolTable::getLabel();
    addr_se = new TemporarySymbolEntry(type, ret_label);//新建地址
    TemporarySymbolEntry* ret_se=new TemporarySymbolEntry(ret_type, ret_label);//为返回值新建地址
    ret_dst = new Operand(addr_se);

    //cout<<"ret_dst:"<<ret_dst->toStr()<<endl;
    //cout<<"addr_se:"<<addr_se->getType()->toStr()<<endl;
    if(!isvoid)
        entry->insertFront(new AllocaInstruction(ret_dst,ret_se)); 

    // 增加：处理参数列表
    if(!paramlist.empty()){
       for(unsigned int i = 0;i < paramlist.size();i++)
       {
            //cout<<"paramlist~!"<<endl;
            IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>(paramlist[i]->getSymPtr());
            Instruction *alloca;
            Operand *addr;
            SymbolEntry *addr_se;
            Type *type;
            type = new PointerType(se->getType());
            addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());//新建地址
            addr = new Operand(addr_se);
            
            //为形参分配空间
            alloca = new AllocaInstruction(addr, se); 
            entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block. 
            Operand *ope=paramlist[i]->getOperand();
            func->op.push_back(ope);//存储每个形参的地址
            BasicBlock* bb = builder->getInsertBB();
            new StoreInstruction( addr,ope, bb);//存储形参
            se->setAddr(addr);
            //cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~setAddr~~ "<<se->toStr()<<endl;
            //cout<<se<<endl;
            //cout<<"addr:"<<se->getAddr()->toStr()<<endl;
       }
    }

    stmt->genCode();
    
    /**
     * Construct control flow graph. You need do set successors and predecessors for each basic block.
     * Todo ok
    */
   //建立控制流图
    //cout<<"blocknum: "<<func->getBlockList().size()<<endl;
    //遍历当前function中的每个block
    //cout<<"-----------------------size="<<func->begin()-func->end()<<endl;
    int no;
    bool f=false;
    BasicBlock* bf1=nullptr;
    BasicBlock* bf2=nullptr;
    // BasicBlock* bf=nullptr;

    for(std::vector<BasicBlock*>::iterator block=func->begin();block!=func->end();block++)
    {
        //cout<<"block"<<(*block)->getNo()<<endl;
        if((*block)->empty()){
            fprintf(stderr,"%d\n",(*block)->getNo());
        }
        Instruction * inst = (*block)->rbegin();
        //将每个块的最后一个指令（分支跳转指令）的后续块与当前的块之间的边连接好
        if(inst->isCond())
        {
            //条件跳转指令，后续有两个块
            BasicBlock * trueBB = ((CondBrInstruction*) inst)->getTrueBranch();
            //trueBB->output();
            //cout<<trueBB->getNo()<<endl;
            BasicBlock * falseBB = ((CondBrInstruction*) inst)->getFalseBranch();
            //cout<<falseBB->getNo()<<endl;
            //todo!!!
            //new CondBrInstruction(trueBB,falseBB,,(*block));
            if(!trueBB->empty()){
                (*block)->addSucc(trueBB);
                trueBB->addPred(*block);
            }
            if(!falseBB->empty()){
                (*block)->addSucc(falseBB);
                falseBB->addPred(*block);
            }
        }
        else if(inst->isUncond()){
            //无条件跳转指令，后续有1个块
            BasicBlock * next = ((UncondBrInstruction*) inst)->getBranch();
            ////todo!!!new UncondBrInstruction(next,(*block));
            //next->output();
            //cout<<next->getNo()<<endl;
            //cout<<next->empty()<<endl;
            if(!next->empty()){
                (*block)->addSucc(next);
                next->addPred(*block);
            }
        }
        else{
            // cout<<func->getSymPtr()->toStr()<<endl;
            // cout<<"block:"<<(*block)->getNo()<<endl;
            // cout<<"empty?"<<((*block)->empty())<<endl;
            // if(isvoid){
            //     //(*block)->insertBack(new UncondBrInstruction(func->getRet(), (*block)));
            // }
            //是否为空
            if((*block)->empty()){//为空 ret_bb的no设为此
                f=true;
                no=(*block)->getNo();
                bf2=(*block);
            }
            else{
                f=false;
                bf1=(*block);
                // bf=(*block);
            }
            // cout<<"neither!"<<endl;

        }
        //cout<<"getNumOfPred"<<(*block)->getNumOfPred()<<endl;
        //cout<<"getNumOfSucc"<<(*block)->getNumOfSucc()<<endl;
    }
    
    type = new PointerType(ret_type);
    int ld_label=SymbolTable::getLabel();
    addr_se = new TemporarySymbolEntry(type, ld_label);//新建地址
    TemporarySymbolEntry* ld_se = new TemporarySymbolEntry(ret_type, ld_label);
    Operand* ld_dst = new Operand(ld_se);
    // cout<<"ld_dst:"<<ld_dst->toStr()<<endl;
    Operand* dst = new Operand(ld_se);
    //f=true 说明end_bb为空
    //直接把ret_bb的blocknum改掉
    
    if(!isvoid){
        if(f){
            //cout<<"ld_dst:"<<ld_dst->toStr()<<endl;
            // func->ret_bb=new BasicBlock(func);
            //func->ret_bb->setNo(no);//必须要setNo，因为这条指令是和ReturnStmt里的UncondBr耦合的
            //new UncondBrInstruction(func->ret_bb,bf2);
            if(bf2!=func->getRet()){
                new UncondBrInstruction(func->ret_bb,bf2);
                // func->getRet()->setNo(bf2->getNo());

            }
            //func->ret_bb->setNo(no);//必须要setNo，因为这条指令是和ReturnStmt里的UncondBr耦合的
            LoadInstruction* loca=new LoadInstruction(ld_dst, ret_dst, func->getRet());
            RetInstruction* ret=new RetInstruction(dst,func->getRet());  
        }
        else{
            //cout<<"ld_dst:"<<ld_dst->toStr()<<endl;
            //end_bb不为空
            //直接把两条instruction（load&ret）加在end_bb块后面
            //UncondBrInstruction* toret = new UncondBrInstruction(func->ret_bb,bf1);
            new UncondBrInstruction(func->ret_bb,bf1);
            // new UncondBrInstruction(func->ret_bb,bf);
            new LoadInstruction(ld_dst, ret_dst, func->getRet());
            new RetInstruction(dst,func->getRet());  
        }
    }
    else{
        BasicBlock* end=*(func->end()-1);
        if(end->getNo()==func->getRet()->getNo()){
            new RetInstruction(nullptr,func->getEntry());  
        }
        //else{
            if(f){
                //func->ret_bb->setNo(no);
                if(bf2!=func->getRet()){
                    new UncondBrInstruction(func->ret_bb,bf2);
                    // func->getRet()->setNo(bf2->getNo());
                }
                RetInstruction* ret=new RetInstruction(nullptr,func->getRet());  
            }
            else{
                new UncondBrInstruction(func->ret_bb,bf1);
                new RetInstruction(nullptr,func->getRet());  
            }
        //}
    }
    cout<<"defEnd~!"<<endl;
    // loca->output();
    // ret->output();
    // ret_bb->output();   
}

void BinaryExpr::genCode()
{
    //cout<<"BinaryExpr~!"<<endl;
    BasicBlock *bb = builder->getInsertBB();
    //cout<<"bb_num:"<<bb->getNo()<<endl;
    Function *func = bb->getParent();
    if(bb)
        func=bb->getParent();

    if (op == AND)
    {
        //cout<<"AND~!"<<endl;
        BasicBlock *trueBB = new BasicBlock(func);  // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();//expr1的代码在bb中
        
        //一定要重新getinsetbb!!因为expr1很可能调用了其他东西改了bb
        BasicBlock *now_bb = builder->getInsertBB();
        
        Instruction * inst;
        //生成bool型的判断dst
        if(expr1->getOperand()->getType()->toStr()=="i32"||expr1->getOperand()->getType()->toStr()=="i32()"){
            Operand * temp_dst = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));        
            new CmpInstruction(CmpInstruction::NE,temp_dst,expr1->getOperand(),const_0,now_bb);
            inst = new CondBrInstruction(trueBB,nullptr,temp_dst,now_bb);//连接bb,trueBB
        }
        else if(expr1->getOperand()->getType()->toStr()=="float"||expr1->getOperand()->getType()->toStr()=="float()"){
            Operand * temp_dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));        
            Operand * temp_dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));        
            new TypefixInstruction(temp_dst1, const_0, now_bb, TypefixInstruction::INT2FLOAT);
            new CmpInstruction(CmpInstruction::NE,temp_dst2,expr1->getOperand(),temp_dst1,now_bb);
            inst = new CondBrInstruction(trueBB,nullptr,temp_dst2,now_bb);//连接bb,trueBB

        }
        else{
            inst = new CondBrInstruction(trueBB,nullptr,expr1->getOperand(),now_bb);//连接bb,trueBB
        }
        //很重要  inst的falselist必须接到false_list里（ifstmt回填）
        expr1->falseList().push_back(inst);//加入list

        backPatch(expr1->trueList(), trueBB);//回填expr1->trueList()
        //and短路，如果expr1正确，才需跳到trueBB继续计算expr2，后续语句在trueBB块生成
        //如果错误后续语句就还在当前块生成
        
        builder->setInsertBB(trueBB);               // set the insert point to the trueBB so that intructions generated by expr2 will be inserted into it.
        expr2->genCode();//expr1的代码在trueBB中
        if(expr2->getOperand()->getType()->toStr()=="i32"||expr2->getOperand()->getType()->toStr()=="i32()"){
            new CmpInstruction(CmpInstruction::NE,dst,expr2->getOperand(),const_0,builder->getInsertBB());//生成bool型的判断dst
        }
        else if(expr2->getOperand()->getType()->toStr()=="float"||expr2->getOperand()->getType()->toStr()=="float()"){
            Operand * temp_dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));        
            new TypefixInstruction(temp_dst1, const_0, builder->getInsertBB(), TypefixInstruction::INT2FLOAT);
            new CmpInstruction(CmpInstruction::NE,dst,expr2->getOperand(),temp_dst1,builder->getInsertBB());
        }
        else{
            // new BinaryInstruction(BinaryInstruction::SAME, dst, expr2->getOperand(), expr2->getOperand(), builder->getInsertBB());
            new CmpInstruction(CmpInstruction::NE, dst, expr2->getOperand(), const_0, builder->getInsertBB());
        }
        // cout<<expr1->getSymPtr()->toStr()<<":"<<expr1->getSymPtr()->getType()->toStr()<<endl;
        // cout<<expr2->getSymPtr()->toStr()<<":"<<expr2->getSymPtr()->getType()->toStr()<<endl;
        // cout<<dst->toStr()<<":"<<dst->getType()->toStr()<<endl;
        true_list = expr2->trueList();//expr2->trueList就是true_list
        false_list = merge(expr1->falseList(), expr2->falseList());//二者的falseList均为整个expr的false_list
    }
    else if(op == OR)
    {
        //cout<<"OR~!"<<endl;
        // Todo ok 与and类似，短路判断
        BasicBlock *falseBB = new BasicBlock(func);  // if the result of lhs is true, jump to the trueBB.
        expr1->genCode();

        BasicBlock *now_bb = builder->getInsertBB();

        //cout<<"now_bb:"<<now_bb->getNo()<<endl;
        //cout<<"falseBB:"<<falseBB->getNo()<<endl;
       
        Instruction * inst;
        Operand * temp_dst = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));        
        if(expr1->getOperand()->getType()->toStr()=="i32"||expr1->getOperand()->getType()->toStr()=="i32()"){
            new CmpInstruction(CmpInstruction::NE,temp_dst,expr1->getOperand(),const_0,now_bb);
            inst = new CondBrInstruction(nullptr,falseBB,temp_dst,now_bb);//连接bb,falseBB
        }
        else if(expr1->getOperand()->getType()->toStr()=="float"||expr1->getOperand()->getType()->toStr()=="float()"){
            Operand * temp_dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));        
            Operand * temp_dst2 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));        
            new TypefixInstruction(temp_dst1, const_0, now_bb, TypefixInstruction::INT2FLOAT);
            new CmpInstruction(CmpInstruction::NE,temp_dst2,expr1->getOperand(),temp_dst1,now_bb);
            inst = new CondBrInstruction(nullptr,falseBB,temp_dst2,now_bb);//连接bb,trueBB

        }
        else{
            inst = new CondBrInstruction(nullptr,falseBB,expr1->getOperand(),now_bb);//连接bb,falseBB
        }
        expr1->trueList().push_back(inst);//加入list

        backPatch(expr1->falseList(), falseBB, false);
        builder->setInsertBB(falseBB);               // set the insert point to the trueBB so that intructions generated by expr2 will be inserted into it.
        expr2->genCode();
        if(expr2->getOperand()->getType()->toStr()=="i32"||expr2->getOperand()->getType()->toStr()=="i32()"){
            new CmpInstruction(CmpInstruction::NE,dst,expr2->getOperand(),const_0,builder->getInsertBB());
        }
        else if(expr2->getOperand()->getType()->toStr()=="float"||expr2->getOperand()->getType()->toStr()=="float()"){
            Operand * temp_dst1 = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));        
            new TypefixInstruction(temp_dst1, const_0, builder->getInsertBB(), TypefixInstruction::INT2FLOAT);
            new CmpInstruction(CmpInstruction::NE,dst,expr2->getOperand(),temp_dst1,builder->getInsertBB());

        }
        else{
            //new BinaryInstruction(BinaryInstruction::SAME, dst, expr2->getOperand(), expr2->getOperand(), builder->getInsertBB());
            new CmpInstruction(CmpInstruction::NE, dst, expr2->getOperand(), const_0, builder->getInsertBB());
        }
        
        
        true_list = merge(expr1->trueList(), expr2->trueList());
        false_list = expr2->falseList();
        //cout<<"end???"<<endl;
    }
    else if(op >= LESS && op <= MORE)
    {
        // Todo
        //cout<<"LESS_MORE~!"<<endl;
        expr1->genCode();
        expr2->genCode();
        Operand * temp_src1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        Operand * temp_src2 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        Operand *src1 = expr1->getOperand();
        Operand *src2 = expr2->getOperand();
        BasicBlock *now_bb=builder->getInsertBB();
        if(expr1->getOperand()->getType()->toStr()=="i1"){
            new TypefixInstruction(temp_src1, expr1->getOperand(), now_bb);
            src1=temp_src1;
        }
        if(expr2->getOperand()->getType()->toStr()=="i1"){
            new TypefixInstruction(temp_src2, expr2->getOperand(), now_bb);
            src2=temp_src2;
        }
                if((src1->getType()->toStr()=="float"||src1->getType()->toStr()=="float()")
            && (src2->getType()->toStr()=="i32"||src2->getType()->toStr()=="i32()")){
                Operand * temp = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
                new TypefixInstruction(temp, src2, now_bb, TypefixInstruction::INT2FLOAT);
                src2=temp;
            }
        else if((src2->getType()->toStr()=="float"||src2->getType()->toStr()=="float()")
            && (src1->getType()->toStr()=="i32"||src1->getType()->toStr()=="i32()")){
                Operand * temp = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
                new TypefixInstruction(temp, src1, now_bb, TypefixInstruction::INT2FLOAT);
                src1=temp;
            }
        // if(src1->getType()->toStr()=="float"&&expr1->getSymPtr()->isConstant()){
        //     Operand * temp = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        //     new BinaryInstruction(BinaryInstruction::ADD, temp, float_const_0, src1, builder->getInsertBB());
        //     src1=temp;
        // }
        // if(src2->getType()->toStr()=="float"&&expr2->getSymPtr()->isConstant()){
        //     Operand * temp = new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
        //     new BinaryInstruction(BinaryInstruction::ADD, temp, float_const_0, src2, builder->getInsertBB());
        //     src2=temp;
        // }
        int opcode;
        switch (op)
        {
        case LESS:
            opcode = CmpInstruction::L;
            break;
        case MORE:
            opcode = CmpInstruction::G;
            break;
        case MORE_E:
            opcode = CmpInstruction::GE;
            break;
        case LESS_E:
            opcode = CmpInstruction::LE;
            break;
        case EQUAL:
            opcode = CmpInstruction::E;
            break;
        case NOT_EQUAL:
            opcode = CmpInstruction::NE;
            break;
        
        }
        now_bb = builder->getInsertBB();//重新getInsertBB!!
        new CmpInstruction(opcode, dst, src1, src2, now_bb);
    }
    else if(op >= ADD && op <= MOD)
    {
        expr1->genCode();
        expr2->genCode();
        Operand *src1 = expr1->getOperand();
        Operand *src2 = expr2->getOperand();

        cout<<src1->getEntry()->toStr()<<"-------------"<<src1->getType()->toStr()<<endl;
        cout<<src2->getEntry()->toStr()<<"-------------"<<src2->getType()->toStr()<<endl;
        cout<<dst->getEntry()->toStr()<<"-------------"<<dst->getType()->toStr()<<endl;

        if(src1->getType()->toStr()!=dst->getType()->toStr()){
            Operand * temp = new Operand(new TemporarySymbolEntry(dst->getType(), SymbolTable::getLabel()));
            if(dst->getType()->toStr()=="i32"){
                new TypefixInstruction(temp, src1, builder->getInsertBB(), TypefixInstruction::FLOAT2INT);
                src1=temp;
            }
            else if(dst->getType()->toStr()=="float"){
                new TypefixInstruction(temp, src1, builder->getInsertBB(), TypefixInstruction::INT2FLOAT);
                src1=temp;
            }
        }
        if(src2->getType()->toStr()!=dst->getType()->toStr()){
            Operand * temp = new Operand(new TemporarySymbolEntry(dst->getType(), SymbolTable::getLabel()));
            if(dst->getType()->toStr()=="i32"){
                new TypefixInstruction(temp, src2, builder->getInsertBB(), TypefixInstruction::FLOAT2INT);
                src2=temp;
            }
            else if(dst->getType()->toStr()=="float"){
                new TypefixInstruction(temp, src2, builder->getInsertBB(), TypefixInstruction::INT2FLOAT);
                src2=temp;
            }
        }


        int opcode;
        switch (op)
        {
        case ADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SUB:
            opcode = BinaryInstruction::SUB;
            break;
        case MUL:
            opcode = BinaryInstruction::MUL;
            break;
        case DIV:
            opcode = BinaryInstruction::DIV;
            break;
        case MOD:
            opcode = BinaryInstruction::MOD;
            break;
        }
        BasicBlock *now_bb = builder->getInsertBB();//重新getInsertBB!!
        new BinaryInstruction(opcode, dst, src1, src2, now_bb);
    }
}

void Constant::genCode()
{
    //cout<<"Constant~!"<<endl;
    // we don't need to generate code.
}

// void CondNode::genCode(){
//     //todo
//     cout<<"CondNode~!"<<endl;
//     //if(!expr)printf("$$$$$$$$$$");
//     expr->genCode();
//     true_list = expr->trueList();
//     false_list = expr->falseList();
//     //BasicBlock *bb = builder->getInsertBB();
//     //printf("expr1->getOperand():%s\n",expr->getOperand()->toStr().c_str());
//     //new CmpInstruction(CmpInstruction::E,dst,expr->getOperand(),nullptr,bb);
// }
void Id::genCode()
{
    //cout<<"Id~!"<<endl;
    BasicBlock *bb = builder->getInsertBB();
    Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getAddr();
    //cout<<"addr:"<<addr->toStr()<<endl;
    //cout<<"dst:"<<dst->toStr()<<endl;
    new LoadInstruction(dst, addr, bb);
}

void ArrayItem::genCode()
{
    cout<<"ArrayItem~!"<<endl;
    BasicBlock *bb = builder->getInsertBB();
    
    if(!offsets.empty()){
        for(auto & offset : offsets){
            // cout<<offset->getOperand()->toStr()<<endl;
            //offset->genCode();
        }
    }
    Operand *addr = (dynamic_cast<IdentifierSymbolEntry*>(symbolEntry))->getAddr();
    cout<<"symbolEntry:"<<symbolEntry->toStr()<<endl;
    vector<int>dims=((ArrayType*)(symbolEntry->getType()))->get_dims();
    vector<int>dims_copy=dims;
    cout<<"dims:";
    for(auto& dim:dims){
        cout<<dim<<", ";
    }
    cout<<endl;
    if(dims[0]==0){//说明是参数
        cout<<"empty!"<<endl;
        bb = builder->getInsertBB();

        Type* array_type = (dynamic_cast<IdentifierSymbolEntry*>(symbolEntry))->getType();

        SymbolEntry *tmp_se = new TemporarySymbolEntry(array_type, SymbolTable::getLabel());
        Operand* temp_addr=new Operand(tmp_se);
        cout<<"offsets_size:"<<offsets.size()<<endl;

        Operand* offset_addr;
        if(offsets.empty()){
            cout<<"empty()"<<endl;
            offset_addr=const_0;

            new LoadInstruction(temp_addr, addr, bb);
            heads.push_back(temp_addr);
            // new ArrayItemFetchInstruction(array_type, heads[0], temp_addr, offset_addr, bb, true);
            // new LoadInstruction(dst, heads[0], bb);
        }
        else{
            cout<<"not empty()"<<endl;
            new LoadInstruction(temp_addr, addr, bb);
            dims.erase(dims.begin());
            Type* itype=((ArrayType*)array_type)->gettype();
            //here
            offsets[0]->genCode();
            offset_addr=offsets[0]->getOperand();

            ArrayItemFetchInstruction* special=nullptr;
            
            if(dims.empty()){
                
                heads.push_back(new Operand(new TemporarySymbolEntry(new PointerType(itype), SymbolTable::getLabel())));
                special=new ArrayItemFetchInstruction(dst, itype, heads[0], temp_addr, offset_addr, bb, true);
                special->set_paramFlag(true);
                new LoadInstruction(dst, heads.back(), bb);
                return;
            }

            array_type=new ArrayType(itype, dims);
            
            heads.push_back(new Operand(new TemporarySymbolEntry(new PointerType(array_type), SymbolTable::getLabel())));
            cout<<array_type->toStr()<<endl;
            special=new ArrayItemFetchInstruction(dst, array_type, heads[0], temp_addr, offset_addr, bb, true);
            special->set_paramFlag(true);
            int i=1;
            vector<ExprNode*> offsets_copy=offsets;
            offsets_copy.erase(offsets_copy.begin());
            if(!offsets_copy.empty()){
                for(auto& offset:offsets_copy){
                    //here
                    offset->genCode();
                    offset_addr=offset->getOperand();  
                    Type* type;
                    dims.erase(dims.begin()); 
                    if(!dims.empty()){                    
                        type=new ArrayType(itype, dims);
                    }
                    else{
                        type=itype;
                    }
                    heads.push_back(new Operand(new TemporarySymbolEntry(new PointerType(type), SymbolTable::getLabel())));
                    special=new ArrayItemFetchInstruction(dst, array_type, heads[i], heads[i-1], offset_addr, bb);
                    i++;
                    if(dims.empty()){
                        break;
                    }
                }
            }
            special->set_paramFlag(true);
            new LoadInstruction(dst, heads.back(), bb);
        }
    }
    else{
        Type* itype=((ArrayType*)(symbolEntry->getType()))->gettype();
        //vector<int> dims=((ArrayType*)(symbolEntry->getType()))->get_dims();
        // Operand* temp1 = new Operand(new TemporarySymbolEntry(new PointerType(((ArrayType*)(symbolEntry->getType()))->gettype()), SymbolTable::getLabel()));
        // new LoadInstruction(temp1, addr, bb);
        int i=0;
        heads.clear();
        if(offsets.empty()){
            
            Type *array_type=new ArrayType(itype, dims);
            cout<<array_type->toStr()<<endl;
            
            // dims.erase(dims.begin());
            // Type *sub_type;           
            // sub_type=((ArrayType*)(symbolEntry->getType()))->gettype();
            Type* itype=((ArrayType*)(array_type))->gettype();
            Type* type;
            if(dims.size()>1){
                dims.erase(dims.begin());
                type=new ArrayType(itype, dims);
            }
            else{
                type=itype;
            }
            
            Type* ptype = new PointerType(type);//数组元素的类型
            SymbolEntry *tmp_se = new TemporarySymbolEntry(ptype, SymbolTable::getLabel());
            Operand* temp_addr=new Operand(tmp_se);
            heads.push_back(temp_addr);
            cout<<"addr==nullptr??"<<(addr==nullptr)<<endl;
            new ArrayItemFetchInstruction(dst, array_type, temp_addr, addr, const_0, bb);
            //new LoadInstruction(dst, heads[0], bb);
            return;
        }
        cout<<"offsets size:"<<offsets.size()<<endl;
        if(offsets.size()<5){
            for(auto & offset : offsets){
                // cout<<offset->getOperand()->toStr()<<endl;
                offset->genCode();
            }
        }
        for(auto & offset : offsets){
            cout<<"???"<<endl;
            //here
            if(offsets.size()>5){
                offset->genCode();
            }
            Operand *offset_addr = offset->getOperand();
            // Type *array_type = ((ArrayType*)(symbolEntry->getType()))->gettype();
            Type *array_type=new ArrayType(itype, dims);
            // cout<<array_type->toStr()<<endl;
            dims.erase(dims.begin());
            Type *sub_type;
            if(dims.empty()){
                sub_type=((ArrayType*)(symbolEntry->getType()))->gettype();
            }
            else{
                sub_type=new ArrayType(itype, dims);
            }
            Type* type = new PointerType(sub_type);//数组元素的类型
            SymbolEntry *tmp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            Operand* temp_addr=new Operand(tmp_se);

            if(f)offset_addr=const_0;
        
            new ArrayItemFetchInstruction(dst, array_type, temp_addr, addr, offset_addr, bb);
            cout<<"??"<<endl;
            addr=temp_addr;
            heads.push_back(temp_addr);
            
            if(dims.empty())
                break;
            i++;
        }
        while(!dims.empty()){
            Type *array_type=new ArrayType(itype, dims);
            dims.erase(dims.begin());
            Type *sub_type;
            if(dims.empty()){
                sub_type=((ArrayType*)(symbolEntry->getType()))->gettype();
            }
            else{
                sub_type=new ArrayType(itype, dims);
            }
            Type* type = new PointerType(sub_type);//数组元素的类型
            SymbolEntry *tmp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            Operand* temp_addr=new Operand(tmp_se);

            new ArrayItemFetchInstruction(dst, array_type, temp_addr, addr, const_0, bb);
            addr=temp_addr;
            heads.push_back(temp_addr);
        }
        cout<<"??"<<endl; 
        new LoadInstruction(dst, heads.back(), bb);
    }
}

void IfStmt::genCode()
{
    //cout<<"ifstmt~!"<<endl;
    Function *func;
    BasicBlock *then_bb, *end_bb;

    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);

    //条件判断block
    cond->genCode();//生成条件判断语句的中间代码
    backPatch(cond->trueList(), then_bb);//回填trueList的后续block为then_bb
    backPatch(cond->falseList(), end_bb,false);//回填falseList的后续block为end_bb
    BasicBlock * bb = builder->getInsertBB();
    //cout<<cond->getOperand()->toStr()<<":"<<cond->getOperand()->getType()->toStr()<<endl;
    if(cond->getOperand()->getType()->toStr()=="i32"||cond->getOperand()->getType()->toStr()=="i32()"){        
        Operand *temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        new CmpInstruction(CmpInstruction::NE,temp,cond->getOperand(),const_0,bb);//int转bool
        new CondBrInstruction(then_bb,end_bb,temp,bb);
    }
    else if(cond->getOperand()->getType()->toStr()=="float"||cond->getOperand()->getType()->toStr()=="float()"){
        Operand* temp1=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        Operand* temp2=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));

        new TypefixInstruction(temp1, cond->getOperand(), bb, TypefixInstruction::FLOAT2INT);
        new CmpInstruction(CmpInstruction::NE,temp2,temp1,const_0,bb);//int转bool
        new CondBrInstruction(then_bb,end_bb,temp2,bb);

    }

    else
        new CondBrInstruction(then_bb,end_bb,cond->getOperand(),bb);

    //then block
    builder->setInsertBB(then_bb);
    thenStmt->genCode();


    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);//在thenblock最后加上无条件跳转语句
    //then_bb->output();
    
    //后续block为end_bb
    builder->setInsertBB(end_bb);
}

void IfElseStmt::genCode()
{
    //cout<<"ifelsestmt~!"<<endl;
    // Todo ok
    Function *func;
    BasicBlock *then_bb, *else_bb, *end_bb;

    func = builder->getInsertBB()->getParent();
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);
    else_bb = new BasicBlock(func);

    //条件判断block
    cond->genCode();
    backPatch(cond->trueList(), then_bb);
    backPatch(cond->falseList(), else_bb,false);
    BasicBlock * bb = builder->getInsertBB();
    //判断cond是否为布尔类型
    if(cond->getOperand()->getType()->toStr()=="i32"||cond->getOperand()->getType()->toStr()=="i32()"){
        Operand *temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        new CmpInstruction(CmpInstruction::NE,temp,cond->getOperand(),const_0,bb);//int转bool
        new CondBrInstruction(then_bb,else_bb,temp,bb);
    }
        else if(cond->getOperand()->getType()->toStr()=="float"||cond->getOperand()->getType()->toStr()=="float()"){
        Operand* temp1=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        Operand* temp2=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));

        new TypefixInstruction(temp1, cond->getOperand(), bb, TypefixInstruction::FLOAT2INT);
        new CmpInstruction(CmpInstruction::NE,temp2,temp1,const_0,bb);//int转bool
        new CondBrInstruction(then_bb,else_bb,temp2,bb);
    }


    else
        new CondBrInstruction(then_bb,else_bb,cond->getOperand(),bb);

    //then block
    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, then_bb);

    //else block
    builder->setInsertBB(else_bb);
    elseStmt->genCode();
    else_bb = builder->getInsertBB();
    new UncondBrInstruction(end_bb, else_bb);

    //后续block为end_bb
    builder->setInsertBB(end_bb);
}


void CompoundStmt::genCode()
{
    //cout<<"CompoundStmt~!"<<endl;
    // Todo ok
    stmt->genCode();
}

void SeqNode::genCode()
{
    //cout<<"seqnode~!"<<endl;
    // Todo ok
    stmt1->genCode();
    stmt2->genCode();
}

void DeclStmt::genCode()
{
    cout<<"DeclStmt~!"<<endl;
    //TODO ok
    map<Id*, ExprNode*>::iterator iter;
    for(iter = idlist.begin();iter!=idlist.end();iter++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>((iter->first)->getSymPtr());
        if(se->isGlobal())//scope==GLOBAL（就是0）
        {
            cout<<"Global~!"<<endl;
            //builder->getUnit()->global_dst.push_back(se);
            Operand *addr;
            SymbolEntry *addr_se;
            addr_se = new IdentifierSymbolEntry(*se);
            addr_se->setType(new PointerType(se->getType()));
            addr = new Operand(addr_se);
            se->setAddr(addr);
            
        }
        else if(se->isLocal())
        {
            //根据符号表项se分配地址（operand以地址方式存在）
            Function *func = builder->getInsertBB()->getParent();
            BasicBlock *entry = func->getEntry();
            Instruction *alloca;
            Operand *addr;
            SymbolEntry *addr_se;
            Type *type;
            type = new PointerType(se->getType());
            addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            //addr = iter->first->getOperand();//new Operand(addr_se);
            addr = new Operand(addr_se);
            cout<<"setAddr~~ "<<se->toStr()<<endl;
            se->setAddr(addr);  
            // entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
        }
    }
    for(iter = idlist.begin();iter!=idlist.end();iter++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>((iter->first)->getSymPtr());
        if(se->isGlobal())//scope==GLOBAL（就是0）
        {
            cout<<"**************************Global~!"<<endl;
            
            builder->getUnit()->global_dst.push_back(se);
            cout<<"##"<<se->toStr()<<endl;
            Operand *addr = se->getAddr();
            se->setAddr(addr);
            //cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~setAddr~~ "<<se->toStr()<<endl;

            std::string dst, type;
            dst = addr->toStr();
            type = se->getType()->toStr();

            BasicBlock* bb=builder->getInsertBB();
            
            //new GlobalInstruction(addr, se, bb); 
            if(iter->second){
                cout<<se->toStr()<<se->get_value()<<endl;
                if(se->getType()->isInt()){
                    cout<<"isInt"<<endl;
                    //cout<<"constant?"<<iter->second->getSymPtr()->toStr()<<endl;
                    builder->getUnit()->global_src.push_back(iter->second->getSymPtr());
                    cout<<"##"<<iter->second->getSymPtr()->get_value()<<endl;
                    BasicBlock *bb = builder->getInsertBB();
                    iter->second->genCode();
                    Operand *src = iter->second->getOperand();
                    new StoreInstruction(addr, src, bb);
                    string dst1 = addr->toStr();
                    string src1 = src->toStr();
                    string dst_type1 = addr->getType()->toStr();
                    string src_type1 = src->getType()->toStr();
                    //printf("%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),src1.c_str());
                    builder->getUnit()->global_defs.push_back(new GlobalInstruction(addr, src, bb));
                    // fprintf(yyout, "%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),src1.c_str());
                }
                else{
                    cout<<"isFloat"<<endl;
                    builder->getUnit()->global_src.push_back(iter->second->getSymPtr());
                    cout<<"##"<<iter->second->getSymPtr()->get_value()<<endl;
                    BasicBlock *bb = builder->getInsertBB();
                    iter->second->genCode();
                    Operand *src = iter->second->getOperand();
                    new StoreInstruction(addr, src, bb);
                    string dst1 = addr->toStr();
                    string src1 = src->toStr();
                    string dst_type1 = addr->getType()->toStr();
                    string src_type1 = src->getType()->toStr();
                    //printf("%s = global %s %s, align 4\n", dst.c_str(), type.c_str(),src1.c_str());
                    //new GlobalInstruction(addr,se);
                    
                    // double temp=0;
                    // if(iter->second->getSymPtr()->isConstant()){
                    //     temp=((ConstantSymbolEntry*)(iter->second->getSymPtr()))->getValue();
                    // }
                    // uint64_t val = reinterpret_cast<uint64_t&>(temp);

                    // fprintf(yyout, "%s = global %s 0x%lX, align 4\n",
                    //          dst.c_str(), type.c_str(), val);
                    builder->getUnit()->global_defs.push_back(new GlobalInstruction(addr, src, bb));
                    // format floating point to llvm format.
                }
            }
            else{
                builder->getUnit()->global_src.push_back(nullptr);
                cout<<"##nothing!"<<endl;
                builder->getUnit()->global_defs.push_back(new GlobalInstruction(addr, const_0, bb));
                // fprintf(yyout, "%s = global %s 0, align 4\n", dst.c_str(), type.c_str());
            }
        }
        else if(se->isLocal())
        {
            //根据符号表项se分配地址（operand以地址方式存在）
            Function *func = builder->getInsertBB()->getParent();
            BasicBlock *entry = func->getEntry();
            Instruction *alloca;
            Operand *addr=se->getAddr();
            // cout<<"addr:"<<addr->getType()->toStr()<<endl;
            // cout<<"se:"<<se->getType()->toStr()<<endl;
                                                    // set the addr operand in symbol entry so that we can use it in subsequent code generation.
            alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
            //如果声明的同时赋值，需要跟Store指令
            if(iter->second){
                BasicBlock *bb = builder->getInsertBB();
                iter->second->genCode();
                Operand *src = iter->second->getOperand();
                if(se->getType()->toStr()!=src->getType()->toStr()){
                    Operand* temp=new Operand(new TemporarySymbolEntry(se->getType(), SymbolTable::getLabel()));
                    if(se->getType()->toStr()=="i32"){
                        new TypefixInstruction(temp, src, bb, TypefixInstruction::FLOAT2INT);
                    }
                    else{
                        new TypefixInstruction(temp, src, bb, TypefixInstruction::INT2FLOAT);
                    }
                    src=temp;
                }
                new StoreInstruction(addr, src, bb);
                //printf("19 ");
            }
            entry->insertFront(alloca);                                 // allocate instructions should be inserted into the begin of the entry block.
        }
    }
    map<Id*, vector<ExprNode*>>::iterator array_iter;
    cout<<"arraylist.size():"<<arraylist.size()<<endl;
    for(array_iter = arraylist.begin();array_iter!=arraylist.end();array_iter++){
        IdentifierSymbolEntry *se = dynamic_cast<IdentifierSymbolEntry *>((array_iter->first)->getSymPtr());
        if(se->isGlobal()){
            cout<<"----Global!!"<<endl;
            cout<<se->toStr()<<endl;
            builder->getUnit()->arr_global_dst.push_back(se);
            Operand *addr;
            SymbolEntry *addr_se;
            addr_se = new IdentifierSymbolEntry(*se);
            addr_se->setType(new PointerType(se->getType()));
            addr = new Operand(addr_se);
            se->setAddr(addr);
            //cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~setAddr~~ "<<se->toStr()<<endl;
            std::string dst, type;
            dst = addr->toStr();
            type = se->getType()->toStr();

            BasicBlock* bb=builder->getInsertBB();

            bool f=false;
            if(!array_iter->second.empty()){
                for(auto& sc:array_iter->second){
                    if(sc){
                        f=true;
                        break;
                    }
                }
            }

            if(f&&!array_iter->second.empty()){
                vector<SymbolEntry*> g_s;
                for(auto & exp:array_iter->second){
                    SymbolEntry* item=nullptr;
                    if(exp)
                        item=exp->getSymPtr();    
                    g_s.push_back(item);
                }
                builder->getUnit()->arr_global_src.push_back(g_s);
                vector<ExprNode*> explist=array_iter->second;
                
                BasicBlock *bb = builder->getInsertBB();

                std::ostringstream buffer;
                deque<string> q;
                //buffer<<dst<<" = global "<<type;
                vector<int> dc;//计数
                vector<int> dims=((ArrayType*)(se->getType()))->get_dims();
                vector<int> dims_copy;
                int total=1;
                //cout<<"dims: ";
                for(auto & dim : dims){
                    dc.push_back(0);
                    total*=dim;
                    //cout<<dim<<", ";
                }
                //cout<<endl;
                int maxsize=dims.size();
                Type* itype=((ArrayType*)(se->getType()))->gettype();
                Type *sub_type=((ArrayType*)(se->getType()))->gettype();
                Type *init_type=sub_type;
                int count=0;
                //buffer<<"[";
                bool runFlag=true;
                while(runFlag){
                    cout<<"count:"<<count<<endl;
                    cout<<"dc.back():"<<dc.back()<<endl;
                    if(dc.back()==dims.back()){
                        cout<<"----enter if"<<endl;
                        int i=dc.size()-1;
                        //进位
                        while(i>=0){
                            dc[i]=0;
                            //if(i>0){
                                if(i>0&&dc[i-1]+1==dims[i-1]){
                                    //未找到进位
                                    cout<<"11"<<endl;
                                    i--;
                                    continue;
                                }
                                else{
                                    cout<<"22"<<endl;
                                    //找到进位
                                    //q.pop_back();//弹出最后的逗号
                                    for(int k=dc.size()-1;k>=i;k--){
                                        buffer<<"["<<dims[k]<<" x "<<sub_type->toStr()<<"] ";
                                        //dims_copy.push_back(dims[k]);
                                        dims_copy=dims;
                                        while(dims_copy.size()!=dc.size()-k){
                                            dims_copy.erase(dims_copy.begin());
                                        }
                                        sub_type=new ArrayType(itype, dims_copy);
                                        string temp_str=buffer.str();
                                        buffer.str("");
                                        deque<string> temp_q;
                                        for(int x=0;x<dims[k];x++){
                                            temp_q.push_back(q.back());
                                            q.pop_back();
                                        }
                                        temp_str+="[";
                                        for(int x=0;x<dims[k];x++){
                                            temp_str+=temp_q.back();
                                            temp_q.pop_back();
                                            if(x<dims[k]-1)temp_str+=", ";
                                        }
                                        temp_str+="]";
                                        q.push_back(temp_str);
                                    }
                                    if(i>0){
                                        dc[i-1]++;
                                    }
                                    else{
                                        runFlag=false;
                                    }
                                    break;
                                }
                            //}
                            i--;
                        }
                    }
                    else{
                        cout<<"----enter else"<<endl;
                        int val=0;
                        // cout<<"explist[count]:"<<(explist.size())<<endl;
                        if(explist[count]){
                            //val=((ConstantSymbolEntry*)(explist[count]->getSymPtr()))->getValue();
                            val=(explist[count]->getSymPtr())->get_value();
                        }              
                        //buffer.clear();
                        sub_type=init_type;
                        dims_copy.clear();
                        buffer<<init_type->toStr()<<" "<<val;
                        q.push_back(buffer.str());
                        //q.push_back(",");//先不加逗号
                        buffer.str("");
                        dc.back()++;
                        count++;
                    }                   
                    // for(auto & str:q){
                    //     cout<<str<<"|";
                    // }
                    // cout<<endl;
                    //maxsize--;
                }
                buffer.str("");
                for(auto & str:q){
                    //cout<<str<<"|";
                    buffer<<str;
                    if(str!=q.back())buffer<<", ";
                }
                //cout<<endl;
                builder->getUnit()->global_defs.push_back(new GlobalInstruction(addr, const_0, bb, buffer.str()));
                //fprintf(yyout, "%s = global %s, align 4\n", dst.c_str(), buffer.str().c_str());
                //new GlobalInstruction(addr,se);
                //fprintf(yyout, "%s", buffer.str().c_str());
            }
            else{
                vector<SymbolEntry*> g_s;//是空的
                builder->getUnit()->arr_global_src.push_back(g_s);
                builder->getUnit()->global_defs.push_back(new GlobalInstruction(addr, const_0, bb, "zero"));
                // fprintf(yyout, "%s = global %s zeroinitializer, align 4\n", dst.c_str(), type.c_str());
            }
        }
        else{
            //根据符号表项se分配地址（operand以地址方式存在）
            Function *func = builder->getInsertBB()->getParent();
            BasicBlock *entry = func->getEntry();
            Instruction *alloca;
            Operand *addr;
            Operand *src;
            SymbolEntry *addr_se;
            Type *type;
            type = new PointerType(se->getType());
            addr_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
            //addr = iter->first->getOperand();//new Operand(addr_se);
            addr = new Operand(addr_se);
            // cout<<"addr:"<<addr->getType()->toStr()<<endl;
            cout<<"se:"<<se->getType()->toStr()<<endl;
                                                    // set the addr operand in symbol entry so that we can use it in subsequent code generation.
            se->setAddr(addr);  
            //cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~setAddr~~ "<<se->toStr()<<endl;
            alloca = new AllocaInstruction(addr, se);                   // allocate space for local id in function stack.
            entry->insertFront(alloca);  

            bool f=false;
            if(!array_iter->second.empty()){
                for(auto& sc:array_iter->second){
                    if(sc){
                        f=true;//不全为空
                        break;
                    }
                }
            }
            if(!f){
                //全为空，直接bl memset
                //改了以后生成中间代码可能不对，
                //还是在ArrayItemFetchInstruction里做，比较方便
                new ArrayItemFetchInstruction(se->getType(),addr,true,builder->getInsertBB());
                continue;
            }

            new MemsetInstruction(se->getType(),addr,builder->getInsertBB());
            
            //如果声明的同时赋值，需要跟Store指令
            vector<int> dc;//计数
            vector<int> dims=((ArrayType*)(se->getType()))->get_dims();
            for(auto & dim : dims){
                dc.push_back(0);
            }
            int maxsize=dc.size();
            if(!array_iter->second.empty()){
                // cout<<"--------------explist: ";
                // int v=0;
                // for(auto & exp:array_iter->second){
                //     if(exp)v=exp->getSymPtr()->get_value();
                //     cout<<v<<", ";
                //     v=0;
                // }
                // cout<<endl;
                BasicBlock* bb=builder->getInsertBB();
                cout<<"explist:"<<array_iter->second.size();
                for(auto & exp : array_iter->second){
                    vector<int> dims_copy=dims;
                    Type* itype=((ArrayType*)(se->getType()))->gettype();
                    Operand* addr_copy=addr;
                    Operand* tag=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));

                    //int i=0;
                    if(exp){
                        //不为空
                        cout<<"! nullptr"<<endl;
                        exp->genCode();
                        bb=builder->getInsertBB();
                        src=exp->getOperand();//!!tag不能用这个
                    }
                    else{
                        //为空，赋初值0
                        cout<<"nullptr"<<endl;
                        src=const_0;
                        //生成中间代码就不对了。。先这样
                        // continue;
                        goto jump_to;
                    }


                    for(auto &off:dc){
                        Operand *offset_addr = new Operand(new ConstantSymbolEntry(TypeSystem::intType, off));
                        // Type *array_type = ((ArrayType*)(symbolEntry->getType()))->gettype();
                        Type *array_type=new ArrayType(itype, dims_copy);
                        cout<<array_type->toStr()<<endl;

                        dims_copy.erase(dims_copy.begin());
                        Type *sub_type;
                        if(dims_copy.empty()){
                            sub_type=((ArrayType*)(se->getType()))->gettype();
                        }
                        else{
                            sub_type=new ArrayType(itype, dims_copy);
                        }
                        Type* type = new PointerType(sub_type);//数组元素的类型
                        SymbolEntry *tmp_se = new TemporarySymbolEntry(type, SymbolTable::getLabel());
                        Operand* temp_addr=new Operand(tmp_se);
                        
                        new ArrayItemFetchInstruction(tag, array_type, temp_addr, addr_copy, offset_addr, bb);
                        addr_copy=temp_addr;
                        if(dims_copy.empty())
                            break;
                        //i++;
                    }
                    
                    if(((ArrayType*)(se->getType()))->gettype()->isFloat()&&src->getType()->isInt()){
                        Operand* ot=new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
                        new TypefixInstruction(ot, src, bb, TypefixInstruction::INT2FLOAT);
                        src=ot;
                    }
                    
                    bb=builder->getInsertBB();
                    new StoreInstruction(addr_copy, src, bb);
jump_to:
                    //dc.back()++;
                    if(dc.back()+1==dims.back()){
                        int i=dc.size()-1;
                        //进位
                        while(i>=0){
                            dc[i]=0;
                            if(i>0){
                                if(dc[i-1]+1==dims[i-1]){
                                    i--;
                                    continue;
                                }
                                else{
                                    dc[i-1]++;
                                    break;
                                }
                            }
                            i--;
                        }
                    }
                    else
                        dc.back()++;
                    // cout<<"dc: ";
                    // for(auto &d : dc){
                    //     cout<<d<<", ";
                    // }
                    // cout<<endl;
                    // if(dc[0]+1==dims[0]){
                    //     break;
                    // }
                    
                    
                }
                // new LoadInstruction(dst, heads[i], bb);
                
                //printf("19 ");
            }
            
            
            
        }  
    }
}

void ReturnStmt::genCode()
{
    //cout<<"ReturnStmt~!"<<endl;
    // Todo ok
    
    BasicBlock *bb = builder->getInsertBB();
    Function* func=bb->getParent();
    if(retValue){
        retValue->genCode();
        SymbolEntry *se = retValue->getSymPtr();
        ret_ope=retValue->getOperand();
        bb = builder->getInsertBB();
        //cout<<"ret_dst:"<<ret_dst->toStr()<<endl;
        //cout<<"ret_ope:"<<ret_ope->toStr()<<endl;

        if(ret_ope->getType()->toStr()=="i32"&&ret_dst->getType()->toStr()=="float*"){
            Operand* temp=new Operand(new TemporarySymbolEntry(TypeSystem::floatType, SymbolTable::getLabel()));
            new TypefixInstruction(temp, ret_ope, bb, TypefixInstruction::INT2FLOAT);
            ret_ope=temp;
        }
        else if(ret_ope->getType()->toStr()=="float"&&ret_dst->getType()->toStr()=="i32*"){
            Operand* temp=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            new TypefixInstruction(temp, ret_ope, bb, TypefixInstruction::FLOAT2INT);
            ret_ope=temp;
        }

        new StoreInstruction(ret_dst,ret_ope, bb);//存储形参
        new UncondBrInstruction(func->ret_bb, bb);
    }
    else{
        new UncondBrInstruction(func->ret_bb, bb);
    }
}
void sufSingleExpr::genCode()
{
    // Todo ok
    expr->genCode();

    BasicBlock *bb = builder->getInsertBB();
    Operand *const_1 = new Operand(new ConstantSymbolEntry(dst->getType(), 1));
    Operand *temp = new Operand(new TemporarySymbolEntry(dst->getType(), SymbolTable::getLabel()));
    int opcode;
    switch (op)
    {
    case AADD:
        opcode = BinaryInstruction::ADD;
        break;
    case SSUB:
        opcode = BinaryInstruction::SUB;
        break;
    }
    new BinaryInstruction(opcode, temp, dst, const_1, bb);
    Operand *addr = dynamic_cast<IdentifierSymbolEntry *>(expr->getSymPtr())->getAddr();
    new StoreInstruction(addr, temp, bb);

    true_list = expr->trueList();
    false_list = expr->falseList();
}
void preSingleExpr::genCode()
{
    // Todo ok
    expr->genCode();

    //常数1和常数0

    BasicBlock *bb = builder->getInsertBB();
    Operand *temp = new Operand(new TemporarySymbolEntry(dst->getType(), SymbolTable::getLabel()));
    int opcode;
    
    if(op>=AADD&&op<=SSUB){
        switch (op)
        {
        case AADD:
            opcode = BinaryInstruction::ADD;
            break;
        case SSUB:
            opcode = BinaryInstruction::SUB;
            break;
        default:
            break;
        }
        Operand *addr = dynamic_cast<IdentifierSymbolEntry *>(expr->getSymPtr())->getAddr();
        new LoadInstruction(dst, addr, bb);
        new BinaryInstruction(opcode, temp, dst, const_1, bb);
        new StoreInstruction(addr, temp, bb);
    }
    else if(op==SUB){
        Operand *temp1 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        //cout<<expr->getSymPtr()->getType()->toStr()<<endl;
        if(expr->getOperand()->getType()->toStr()=="i1"){
            new TypefixInstruction(temp1, expr->getOperand(), bb);
            new BinaryInstruction(BinaryInstruction::SUB, dst, const_0, temp1, bb);
        }
        else if(expr->getOperand()->getType()->toStr()=="float"){
            new BinaryInstruction(BinaryInstruction::SUB, dst, float_const_0, expr->getOperand(), bb);
        }
        else{
            new BinaryInstruction(BinaryInstruction::SUB, dst, const_0, expr->getOperand(), bb);
        }
        //else{
        //     new TypefixInstruction(temp1, expr->getOperand(), bb);
        //     Operand *temp2 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        //     new BinaryInstruction(BinaryInstruction::SUB, temp2, const_0, temp1, bb);
        //     new CmpInstruction(CmpInstruction::NE,dst,temp2,const_0,bb);//int转bool
        // }
    }
    else if(op==NOT){
        //cout<<"NOT~!"<<endl;
        opcode=BinaryInstruction::NOT;
        
        //临时变量没有地址！不用load/store直接在寄存器上运算就行
        if(expr->getOperand()->getType()->toStr()=="i32"){
            //Operand *addr = dynamic_cast<IdentifierSymbolEntry *>(expr->getSymPtr())->getAddr();
            Operand *temp1 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            Operand *temp2 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            //new LoadInstruction(temp1, addr, bb);
            new CmpInstruction(CmpInstruction::NE,temp1,expr->getOperand(),const_0,bb);//int转bool
            //new TypefixInstruction(temp2, temp1, bb);
            // new BinaryInstruction(opcode, dst, temp1, temp1, bb);
            new CmpInstruction(CmpInstruction::E,dst,temp1,const_0,bb);

            //Operand *temp3 = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            //
            //new StoreInstruction(addr, temp2, bb);
        }
        else if(expr->getOperand()->getType()->toStr()=="float"){
            Operand *temp = new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
            Operand *temp1 = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
            new TypefixInstruction(temp, expr->getOperand(), bb, TypefixInstruction::FLOAT2INT);
            new CmpInstruction(CmpInstruction::NE,temp1,temp,const_0,bb);//int转bool
            new CmpInstruction(CmpInstruction::E,dst,temp1,const_0,bb);
        }
        else{
            new CmpInstruction(CmpInstruction::E,dst,expr->getOperand(),const_0,bb);
            // new BinaryInstruction(opcode, dst, expr->getOperand(), expr->getOperand(), bb);

        }
    }

    true_list = expr->trueList();
    false_list = expr->falseList();
    
    
}
void FunctionCall::genCode()
{
    // Todo ok
    cout<<"FunctionCall~!"<<endl;
    cout<<se->toStr()<<params.size()<<endl;
    Function *func= builder->getInsertBB()->getParent();
    BasicBlock *nowb = builder->getInsertBB();
    vector<Operand*> vo;
    std::string name;

    std::vector<Type*> types = ((FunctionType*)(se->getType()))->paramsType;
    
    vector<Type*> paramtypes=((FunctionType*)(((IdentifierSymbolEntry*)se)->getType()))->paramsType;
    if(!params.empty()){
        // for(unsigned int i = 0;i < params.size(); i++)
        // {
        //     cout<<"hi1"<<endl;
                        
        //     params[i]->genCode();
        //     Operand *src=params[i]->getOperand();
        //     if(params[i]->isArray()&&paramtypes[i]->isArray()){
        //         src=((ArrayItem*)(params[i]))->heads.back();
        //     }
        //     cout<<"hi2"<<endl;
        //     if(params[i]->isArray()){
        //         cout<<"haha------"<<endl;
        //         if(((ArrayItem*)(params[i]))->getf())
        //             src=((ArrayItem*)(params[i]))->heads[0];
        //     }
        //     vo.push_back(src);
        // }
        for(unsigned int i = 0;i < params.size(); i++)
        {
            params[i]->genCode();
            Operand *src=params[i]->getOperand();
            
            if(src->getType()->toStr()!=types[i]->toStr()){
                Operand* temp=new Operand(new TemporarySymbolEntry(types[i], SymbolTable::getLabel()));
                
                if(types[i]->toStr()=="i32"){
                    if(src->getType()->toStr()=="i1"){
                        new TypefixInstruction(temp, src, nowb, TypefixInstruction::BOOL2INT);
                        src=temp;
                    }
                    else if(src->getType()->toStr()=="float"){
                        new TypefixInstruction(temp, src, nowb, TypefixInstruction::FLOAT2INT);
                        src=temp;
                    }
                    
                }
                else if(types[i]->toStr()=="float"){
                    if(src->getType()->toStr()=="i1"){
                        Operand* temp1=new Operand(new TemporarySymbolEntry(types[i], SymbolTable::getLabel()));
                        new TypefixInstruction(temp1, src, nowb, TypefixInstruction::BOOL2INT);
                        new TypefixInstruction(temp, temp1, nowb, TypefixInstruction::INT2FLOAT);
                        src=temp;
                    }
                    else if(src->getType()->toStr()=="i32"){
                        new TypefixInstruction(temp, src, nowb, TypefixInstruction::INT2FLOAT);
                        src=temp;
                    }
                    
                }
                
                if(params[i]->isArray()){
                //if(((ArrayItem*)(params[i]))->getf())
                    src=((ArrayItem*)(params[i]))->heads.back();
                    cout<<"isArray:"<<src->toStr()<<endl;
                }
                
            }
            vo.push_back(src);
        }
    }
    new CallInstruction(symbolEntry,dst,vo,nowb);
    Operand *dsts=dst;

    if (((IdentifierSymbolEntry*)se)->isSysy()) {
        vector<SymbolEntry*> v=func->getParent()->sysy_list;
        if(std::find(v.begin(), v.end(), se) == v.end()){//去重
            func->getParent()->sysy_list.push_back(se);
        }
    }
}
void EmptyStmt::genCode()
{
    // Todo ok
}
void BreakStmt::genCode()
{
    // cout<<"BreakStmt~!"<<endl;
    cout<<parent<<endl;
    BasicBlock* bb=builder->getInsertBB();
    BasicBlock* end_bb=((WhileStmt*)parent)->get_end_bb();
    new UncondBrInstruction(end_bb, bb);
}
void ContinueStmt::genCode()
{
    //cout<<"ContinueStmt~!"<<endl;
    BasicBlock* bb=builder->getInsertBB();
    BasicBlock* cond_bb=((WhileStmt*)parent)->get_cond_bb();
    new UncondBrInstruction(cond_bb, bb);
}
void ExprStmt::genCode()
{
    // Todo ok
    exp->genCode();
}
void WhileStmt::genCode()
{
    cout<<"WhileStmt~!"<<endl;
    cout<<this<<endl;
    // Todo ok
    // 和ifstmt有区别：cond必须为一个独立的basicblock（cond实际上无条件接在end_bb后面）
    Function *func;
    BasicBlock *then_bb, *end_bb, *cond_bb;

    func = builder->getInsertBB()->getParent();
    //cout<<func->getSymPtr()->toStr()<<endl;
    then_bb = new BasicBlock(func);
    end_bb = new BasicBlock(func);
    cond_bb = new BasicBlock(func);

    this->end_bb=end_bb;
    this->cond_bb=cond_bb;

    BasicBlock* bb = builder->getInsertBB();
    //cout<<"bb:"<<bb->getNo()<<endl;
    // cond无条件接在当前bb后
    
    new UncondBrInstruction(cond_bb, bb);
    builder->setInsertBB(cond_bb);

    //cond block 
    cond->genCode();
    backPatch(cond->trueList(), then_bb);
    //end_bb只能有条件接在cond_bb后面
    backPatch(cond->falseList(), end_bb, false);
    cond_bb=builder->getInsertBB();
    //cout<<"cond_bb:"<<cond_bb->getNo()<<endl;
    if(cond->getOperand()->getType()->toStr()=="i32"||cond->getOperand()->getType()->toStr()=="i32()"){
        Operand *temp = new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));
        new CmpInstruction(CmpInstruction::NE,temp,cond->getOperand(),const_0,cond_bb);//int转bool
        new CondBrInstruction(then_bb,end_bb,temp,cond_bb);
    }
    else if(cond->getOperand()->getType()->toStr()=="float"||cond->getOperand()->getType()->toStr()=="float()"){
        Operand* temp1=new Operand(new TemporarySymbolEntry(TypeSystem::intType, SymbolTable::getLabel()));
        Operand* temp2=new Operand(new TemporarySymbolEntry(TypeSystem::boolType, SymbolTable::getLabel()));

        new TypefixInstruction(temp1, cond->getOperand(), cond_bb, TypefixInstruction::FLOAT2INT);
        new CmpInstruction(CmpInstruction::NE,temp2,temp1,const_0,cond_bb);//int转bool
        new CondBrInstruction(then_bb,end_bb,temp2,cond_bb);

    }
    else
        new CondBrInstruction(then_bb,end_bb,cond->getOperand(),cond_bb);
    
    //then block
    builder->setInsertBB(then_bb);
    thenStmt->genCode();
    then_bb = builder->getInsertBB();
    new UncondBrInstruction(this->cond_bb, then_bb);

    builder->setInsertBB(end_bb);
    cout<<"WhileStmt end"<<endl;
}
void AssignStmt::genCode()
{
    cout<<"AssignStmt~!"<<endl;

    //要检测lval的类型了！！
    expr->genCode();

    if(lval->isId()){
        BasicBlock *bb = builder->getInsertBB();
        IdentifierSymbolEntry* se = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr());
        //cout<<"se:"<<se<<endl;
        Operand *addr = dynamic_cast<IdentifierSymbolEntry*>(lval->getSymPtr())->getAddr();
        Operand *src = expr->getOperand();
        /***
         * We haven't implemented array yet, the lval can only be ID. So we just store the result of the `expr` to the addr of the id.
         * If you want to implement array, you have to caculate the address first and then store the result into it.
         */
        //cout<<"lval?"<<(lval->getOperand()->toStr())<<endl;
        //cout<<"expr?"<<(expr->getOperand()->toStr())<<endl;
        //cout<<"addr?"<<(addr->toStr())<<endl;
        //不能用lval->getOperand()了！！
        Operand* expr_addr=expr->getOperand();
        cout<<"lval:"<<lval->getSymPtr()->getType()->toStr()<<endl;
        cout<<"expr:"<<expr->getSymPtr()->getType()->toStr()<<endl;
        if(lval->getOperand()->getType()->toStr()!=expr->getOperand()->getType()->toStr()){
            Operand* temp=new Operand(new TemporarySymbolEntry(lval->getOperand()->getType(), SymbolTable::getLabel()));
            if(lval->getOperand()->getType()->toStr()=="i32"){
                new TypefixInstruction(temp, expr->getOperand(), bb, TypefixInstruction::FLOAT2INT);
            }
            else if(lval->getOperand()->getType()->toStr()=="float"){
                new TypefixInstruction(temp, expr->getOperand(), bb, TypefixInstruction::INT2FLOAT);
            }
            expr_addr=temp;
        }

        new StoreInstruction(addr, expr_addr, bb);
    }
    else if(lval->isArray()){
        BasicBlock* bb=builder->getInsertBB();
       
        lval->genCode();
        bb=builder->getInsertBB();
        Operand* dst=((ArrayItem*)lval)->heads.back();
        
        Operand* temp=expr->getOperand();
        if(dst->getType()->toStr()!=(expr->getOperand()->getType()->toStr()+"*")){
            temp=new Operand(new TemporarySymbolEntry(lval->getOperand()->getType(), SymbolTable::getLabel()));
            if(dst->getType()->toStr()=="i32*"){
                new TypefixInstruction(temp, expr->getOperand(), bb, TypefixInstruction::FLOAT2INT);
            }
            else if(dst->getType()->toStr()=="float*"){
                new TypefixInstruction(temp, expr->getOperand(), bb, TypefixInstruction::INT2FLOAT);
            }
        }

        new StoreInstruction(dst, temp, bb);
    }
}
void EmptyStmt::typeCheck()
{
    // Todo
}
void BreakStmt::typeCheck()
{
    // Todo
}
void ContinueStmt::typeCheck()
{
    // Todo
}
void Ast::typeCheck()
{
    if(root != nullptr)
        root->typeCheck();
}

void FunctionDef::typeCheck()
{
    // Todo
}
// 


void BinaryExpr::typeCheck()
{
    // Todo
}

void Constant::typeCheck()
{
    // Todo
}
void ExprStmt::typeCheck()
{
    // Todo
}
void WhileStmt::typeCheck()
{
    // Todo
}
void Id::typeCheck()
{
    // Todo
}
void ArrayItem::typeCheck()
{
    // Todo
}

void IfStmt::typeCheck()
{
    // Todo
}

void IfElseStmt::typeCheck()
{
    // Todo
}

void CompoundStmt::typeCheck()
{
    // Todo
}

void SeqNode::typeCheck()
{
    // Todo
}

void DeclStmt::typeCheck()
{
    // Todo
}

void ReturnStmt::typeCheck()
{
    // Todo
}

void AssignStmt::typeCheck()
{
    // Todo
}
void FunctionCall::typeCheck()
{
    // Todo
}
void sufSingleExpr::typeCheck()
{
    // Todo
}
void preSingleExpr::typeCheck()
{
    // Todo
}

void Ast::output()
{
    fprintf(yyout, "program\n");
    if(root != nullptr)
        root->output(4);
}

void BinaryExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case ADD:
            op_str = "add";
            break;
        case SUB:
            op_str = "sub";
            break;
        case MUL:
            op_str = "mul";
            break;
        case DIV:
            op_str = "div";
            break;
        case MOD:
            op_str = "mod";
            break;
        case AND:
            op_str = "and";
            break;
        case OR:
            op_str = "or";
            break;
        case LESS:
            op_str = "less";
            break;
        case MORE:
            op_str = "more";
            break;
        case EQUAL:
            op_str = "equal";
            break;
        case MORE_E:
            op_str = "more_than_or_equal";
            break;
        case LESS_E:
            op_str = "less_than_or_equal";
            break;
        case NOT_EQUAL:
            op_str = "not_equal";
            break;
    }
    fprintf(yyout, "%*cBinaryExpr\top: %s\n", level, ' ', op_str.c_str());
    expr1->output(level + 4);
    expr2->output(level + 4);
}

void preSingleExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case NOT:
            op_str = "not";
            break;
        case AADD:
            op_str = "prefix_self_add";
            break;
        case SSUB:
            op_str = "prefix_self_sub";
            break;
    }
    fprintf(yyout, "%*cprefix_SingleExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}

void sufSingleExpr::output(int level)
{
    std::string op_str;
    switch(op)
    {
        case AADD:
            op_str = "suffix_self_add";
            break;
        case SSUB:
            op_str = "suffix_self_sub";
            break;
    }
    fprintf(yyout, "%*csuffix_SingleExpr\top: %s\n", level, ' ', op_str.c_str());
    expr->output(level + 4);
}


void Constant::output(int level)
{
    std::string type, value;
    type = symbolEntry->getType()->toStr();
    value = symbolEntry->toStr();
    fprintf(yyout, "%*cIntegerLiteral\tvalue: %s\ttype: %s\n", level, ' ',
            value.c_str(), type.c_str());
}

void Id::output(int level)
{
    std::string name, type;
    int scope;
    name = symbolEntry->toStr();
    type = symbolEntry->getType()->toStr();
    // bool isconst = symbolEntry->getType()->is_const();
    // scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    // fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\tis_const:%d\n", level, ' ',
    //         name.c_str(), scope, type.c_str(), isconst);
}

void ArrayItem::output(int level)
{
    cout<<"ArrayItem::output"<<endl;
    // std::string name, type;
    // int scope;
    // name = symbolEntry->toStr();
    // type = symbolEntry->getType()->toStr();
    // bool isconst = symbolEntry->getType()->is_const();
    // scope = dynamic_cast<IdentifierSymbolEntry*>(symbolEntry)->getScope();
    // fprintf(yyout, "%*cId\tname: %s\tscope: %d\ttype: %s\tis_const:%d\n", level, ' ',
    //         name.c_str(), scope, type.c_str(), isconst);
}

void CompoundStmt::output(int level)
{
    fprintf(yyout, "%*cCompoundStmt\n", level, ' ');
    stmt->output(level + 4);
}

void SeqNode::output(int level)
{
    fprintf(yyout, "%*cSequence\n", level, ' ');
    stmt1->output(level + 4);
    stmt2->output(level + 4);
}

void DeclStmt::output(int level)
{
    fprintf(yyout, "%*cDeclStmt\n", level, ' ');
    std::map<Id*, ExprNode*>::iterator it=idlist.begin();
    while(it!=idlist.end()){
        it->first->output(level + 4);
        if(it->second!=nullptr){
            it->second->output(level+4);
        }
        it++;
    }
}

void IfStmt::output(int level)
{
    fprintf(yyout, "%*cIfStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}

void IfElseStmt::output(int level)
{
    fprintf(yyout, "%*cIfElseStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
    elseStmt->output(level + 4);
}

void WhileStmt::output(int level)
{
    fprintf(yyout, "%*cWhileStmt\n", level, ' ');
    cond->output(level + 4);
    thenStmt->output(level + 4);
}


void ReturnStmt::output(int level)
{
    fprintf(yyout, "%*cReturnStmt\n", level, ' ');
    if(!retValue){
        retValue->output(level + 4);
    }
}

void ExprStmt::output(int level)
{
    fprintf(yyout, "%*cExprStmt\n", level, ' ');
    if(exp){
        exp->output(level + 4);
    }
}

void AssignStmt::output(int level)
{
    fprintf(yyout, "%*cAssignStmt\n", level, ' ');
    lval->output(level + 4);
    expr->output(level + 4);
}

void FunctionDef::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionDefine function name: %s, type: %s\n", level, ' ', 
            name.c_str(), type.c_str());
    stmt->output(level + 4);
}

void FunctionCall::output(int level)
{
    std::string name, type;
    name = se->toStr();
    type = se->getType()->toStr();
    fprintf(yyout, "%*cFunctionCall function name: %s, type: %s, params_num: %d\n", level, ' ', 
            name.c_str(), type.c_str(), int(params.size()));
    //vector<SymbolEntry*>::iterator it=params.begin();
    if(params.empty()){
        return;
    }
}

void EmptyStmt::output(int level)
{
    fprintf(yyout, "%*cEmptyStmt\n", level, ' ');
}
void BreakStmt::output(int level)
{
    fprintf(yyout, "%*cBreakStmt\n", level, ' ');
}
void ContinueStmt::output(int level)
{
    fprintf(yyout, "%*cContinueStmt\n", level, ' ');
}
