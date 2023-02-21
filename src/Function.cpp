#include "Function.h"
#include "Unit.h"
#include "Type.h"
#include <list>
#include<string.h>

using namespace std;

extern FILE* yyout;

Function::Function(Unit *u, SymbolEntry *s)
{
    u->insertFunc(this);
    entry = new BasicBlock(this);
    sym_ptr = s;
    parent = u;
}

Function::~Function()
{
    // auto delete_list = block_list;
    // for (auto &i : delete_list)
    //     delete i;
    // parent->removeFunc(this);
}

// remove the basicblock bb from its block_list.
void Function::remove(BasicBlock *bb)
{
    block_list.erase(std::find(block_list.begin(), block_list.end(), bb));
}

void Function::output() const
{
    //构造参数列表
    string params;
    int i=0;
    vector<Type*> type=((FunctionType*)(sym_ptr->getType()))->paramsType;
    for(Type* t:type){
        if(i!=0){
            params+=",";
        }
        params+=type[i]->toStr();
        params+=" ";
        params+=op[i]->toStr();
        i++;
    }

    FunctionType* funcType = dynamic_cast<FunctionType*>(sym_ptr->getType());
    Type *retType = funcType->getRetType();
    fprintf(yyout, "define %s %s(%s) {\n", retType->toStr().c_str(), sym_ptr->toStr().c_str(), params.c_str());
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    while (!q.empty())
    {
        //cout<<"qsize:"<<q.size()<<endl;
        auto bb = q.front();
        q.pop_front();
        bb->output();
        //cout<<"bb_num:"<<bb->getNo()<<endl;
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);//没有这个块
                q.push_back(*succ);
            }
        }
    }
    //cout<<"ret_bb==null?"<<(ret_bb==nullptr)<<endl;
    if(ret_bb!=nullptr)
        ret_bb->output();
    fprintf(yyout, "}\n");
}

void Function::deadinst_mark()
{
    std::set<BasicBlock *> v;
    std::list<BasicBlock *> q;
    q.push_back(entry);
    v.insert(entry);
    while (!q.empty())
    {
        //cout<<"qsize:"<<q.size()<<endl;
        auto bb = q.front();
        q.pop_front();
        bb->deadinst_mark();
        //cout<<"bb_num:"<<bb->getNo()<<endl;
        for (auto succ = bb->succ_begin(); succ != bb->succ_end(); succ++)
        {
            if (v.find(*succ) == v.end())
            {
                v.insert(*succ);//没有这个块
                q.push_back(*succ);
            }
        }
    }
    //cout<<"ret_bb==null?"<<(ret_bb==nullptr)<<endl;
}

void Function::genMachineCode(AsmBuilder* builder) 
{
    auto cur_unit = builder->getUnit();
    auto cur_func = new MachineFunction(cur_unit, this->sym_ptr);
    builder->setFunction(cur_func);
    std::map<BasicBlock*, MachineBlock*> map;
    for(auto block : block_list)
    {
        block->genMachineCode(builder);
        map[block] = builder->getBlock();
    }
    // Add pred and succ for every block
    for(auto block : block_list)
    {
        auto mblock = map[block];
        for (auto pred = block->pred_begin(); pred != block->pred_end(); pred++)
            mblock->addPred(map[*pred]);
        for (auto succ = block->succ_begin(); succ != block->succ_end(); succ++)
            mblock->addSucc(map[*succ]);
    }
    cur_unit->InsertFunc(cur_func);
}
