#ifndef __UNIT_H__
#define __UNIT_H__

#include <vector>
#include "Function.h"
#include "AsmBuilder.h"

class Unit
{
    typedef std::vector<Function *>::iterator iterator;
    typedef std::vector<Function *>::reverse_iterator reverse_iterator;

private:
    std::vector<Function *> func_list;
public:
    std::vector<SymbolEntry *> sysy_list;
    std::vector<GlobalInstruction*> global_defs;

    std::vector<SymbolEntry*> global_dst;
    std::vector<SymbolEntry*> global_src;

    std::vector<SymbolEntry*> arr_global_dst;
    std::vector<vector<SymbolEntry*>> arr_global_src;
public:
    Unit() = default;
    ~Unit() ;
    void insertFunc(Function *);
    void removeFunc(Function *);
    void output() const;
    iterator begin() { return func_list.begin(); };
    iterator end() { return func_list.end(); };
    reverse_iterator rbegin() { return func_list.rbegin(); };
    reverse_iterator rend() { return func_list.rend(); };
    void genMachineCode(MachineUnit* munit);
    void deadinst_mark();
};

#endif