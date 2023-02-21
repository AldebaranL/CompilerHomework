#include "Unit.h"
#include "Type.h"
#include "Ast.h"
#include <iostream>

using namespace std;

extern FILE* yyout;

void Unit::insertFunc(Function *f)
{
    func_list.push_back(f);
}

void Unit::removeFunc(Function *func)
{
    func_list.erase(std::find(func_list.begin(), func_list.end(), func));
}

void Unit::output() const
{
    for(auto &inst : global_defs){
        cout<<"!!global"<<endl;
        inst->output();
    }
    for (auto &func : func_list)
        func->output();
    for (auto se : sysy_list) {
        cout<<"hi??"<<endl;
        FunctionType* type = (FunctionType*)(se->getType());
        std::string str = type->toStr();
        if(se->toStr()=="@getint"||se->toStr()=="@getch"||se->toStr()=="@getfloat"){
            fprintf(yyout, "declare %s %s()\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else if(se->toStr()=="@putfloat"){
            fprintf(yyout, "declare %s %s(float)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else if(se->toStr()=="@getarray"){
            fprintf(yyout, "declare %s %s(i32*)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else if(se->toStr()=="@putarray"){
            fprintf(yyout, "declare %s %s(i32, i32*)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else if(se->toStr()=="@getfarray"){
            fprintf(yyout, "declare %s %s(float*)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else if(se->toStr()=="@putfarray"){
            fprintf(yyout, "declare %s %s(i32, float*)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
        else{
            fprintf(yyout, "declare %s %s(i32)\n", type->getRetType()->toStr().c_str(),
                se->toStr().c_str());
        }
    }
    
}

void Unit::deadinst_mark()
{
    for (auto &func : func_list)
        func->deadinst_mark();
    
}


void Unit::genMachineCode(MachineUnit* munit) 
{
    AsmBuilder* builder = new AsmBuilder();
    builder->setUnit(munit);
    for(auto &inst : global_defs){
        inst->genMachineCode(builder);
    }
    for (auto &func : func_list){
        func->genMachineCode(builder);
    }
}

Unit::~Unit()
{
    for(auto &func:func_list)
        delete func;
}


