#include "Type.h"
#include <sstream>
#include <iostream>
using namespace std;

IntType TypeSystem::commonInt = IntType(32);
IntType TypeSystem::commonConstInt = IntType(32, true);
IntType TypeSystem::commonBool = IntType(1);
VoidType TypeSystem::commonVoid = VoidType();
FloatType TypeSystem::commonFloat = FloatType(32);
FloatType TypeSystem::commonConstFloat = FloatType(32, true);

Type* TypeSystem::intType = &commonInt;
Type* TypeSystem::intType_const = &commonConstInt;
Type* TypeSystem::voidType = &commonVoid;
Type* TypeSystem::boolType = &commonBool;
Type* TypeSystem::floatType = &commonFloat;
Type* TypeSystem::floatType_const = &commonConstFloat;

// bool IntType::is_const()
// {
//     return false;
// }

// bool VoidType::is_const()
// {
//     return false;
// }
// bool FunctionType::is_const()
// {
//     return false;
// }
// bool PointerType::is_const()
// {
//     return false;
// }
std::string IntType::toStr()
{
    std::ostringstream buffer;
    buffer << "i" << size;
    return buffer.str();
}

std::string FloatType::toStr()
{
    return "float";
}


std::string VoidType::toStr()
{
    return "void";
}

std::string FunctionType::toStr()
{
    std::ostringstream buffer;
    buffer << returnType->toStr() << "()";
    return buffer.str();
}

std::string ArrayType::toStr()
{
    //cout<<"arrayType tostr"<<endl;
    std::ostringstream buffer;
    // cout<<dims.empty()<<endl;
    if(dims.empty()){
        //buffer << type->toStr() <<"*";
        cout<<"----------error------------"<<endl;
        buffer<<"----------error------------"<<endl;
        return buffer.str();
    }
    if(dims[0]!=0){
        //buffer << "[" << dims[1] <<" x " << type->toStr() <<"]";
        for(auto & dim:dims){
            buffer << "[" << dim <<" x " ;
        }
        buffer<<type->toStr();
        for(auto & dim:dims){
            buffer << "]" ;
        }
    }
    else{
        if(dims.size()>1){
            int i=1;
            while(i<dims.size()){
                buffer << "[" << dims[i] <<" x " ;
                i++;
            }
            buffer<<type->toStr();
            i=1;
            while(i<dims.size()){
                buffer << "]" ;
                i++;
            }
            buffer<<"*";
        }
        else{
            buffer << type->toStr() <<"*";
        }
    }
    //cout<<"arrayType tostr end----"<<endl;
    return buffer.str();
}

std::string PointerType::toStr()
{
    std::ostringstream buffer;
    buffer << valueType->toStr() << "*";
    return buffer.str();
}
