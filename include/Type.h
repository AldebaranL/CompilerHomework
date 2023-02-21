#ifndef __TYPE_H__
#define __TYPE_H__
#include <vector>
#include <string>

using namespace std;

class Type
{
private:
    int kind;
protected:
    enum {INT, FLOAT, VOID, FUNC, ARRAY, PTR, CONST, BOOL};
public:
    Type(int kind) : kind(kind) {};
    virtual ~Type() {};
    virtual std::string toStr() = 0;
    bool isInt() const {return kind == INT;};
    bool isVoid() const {return kind == VOID;};
    bool isFunc() const {return kind == FUNC;};
    bool isArray() const {return kind == ARRAY;};
    bool isBool() const{return kind == BOOL;};
    bool isFloat() const{return kind==FLOAT;};
    //virtual bool is_const() = 0;
};

class IntType : public Type
{
private:
    int size;
    bool isconst;
public:
       IntType(int size, bool isconst=false) : Type(Type::INT), size(size){
        this->isconst=isconst;
    };
    std::string toStr();
    int getSize(){return size;};
    bool isConst(){return isconst;};
};

class FloatType : public Type
{
private:
    int size;
    bool isconst;
public:
       FloatType(int size, bool isconst=false) : Type(Type::FLOAT), size(size){
        this->isconst=isconst;
    };
    std::string toStr();
    bool isConst(){return isconst;};
};

class VoidType : public Type
{
    bool isconst;
public:
    VoidType() : Type(Type::VOID){};
    std::string toStr();
    //bool is_const();
};

// class ConstType : public Type
// {
// private:
//     int size;
// public:
//     ConstType(int size) : Type(Type::CONST), size(size){};
//     std::string toStr();
// };

class FunctionType : public Type
{
private:
    Type *returnType;
    bool isconst;
public:
    std::vector<Type*> paramsType;
public:
    FunctionType(Type* returnType, std::vector<Type*> paramsType) : 
    Type(Type::FUNC), returnType(returnType), paramsType(paramsType){};
    int num_params(){return paramsType.size();}
    Type* getRetType() {return returnType;};
    std::string toStr();
    //bool is_const();
};

class ArrayType : public Type
{
private:
    Type *type;
public:
    vector<int> dims;
public:
    ArrayType(Type* type, vector<int> dims) : Type(Type::ARRAY), type(type), dims(dims){};
    Type* gettype(){return type;};
    void add_dim(int dim){dims.push_back(dim);};
    vector<int> get_dims(){return dims;};
    //int getsize(){return size;}
    std::string toStr();
    bool isInt(){return this->type->isInt();};
    //bool is_const();
};


class PointerType : public Type
{
private:
    Type *valueType;
    bool isconst;
public:
    PointerType(Type* valueType) : Type(Type::PTR) {this->valueType = valueType;};
    Type* get_valueType(){
        return valueType;
    }
    std::string toStr();
    //bool is_const();
};

class TypeSystem
{
private:
    static IntType commonInt;
    static IntType commonConstInt;
    static IntType commonBool;
    static VoidType commonVoid;
    static FloatType commonFloat;
    static FloatType commonConstFloat;
public:
    static Type *intType;
    static Type *intType_const;
    static Type *voidType;
    static Type *boolType;
    static Type *floatType;
    static Type *floatType_const;
};

#endif
