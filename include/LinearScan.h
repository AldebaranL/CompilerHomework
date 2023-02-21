/**
 * linear scan register allocation
 */

#ifndef _LINEARSCAN_H__
#define _LINEARSCAN_H__
#include <set>
#include <map>
#include <vector>
#include <list>

#include <iostream>
using namespace std;

class MachineUnit;
class MachineOperand;
class MachineFunction;


class LinearScan
{
private:
    struct Interval
    {
        int start; //开始时间
        int end; //结束时间
        bool spill; // 是否溢出 whether this vreg should be spilled to memory
        int disp;   // 保存溢出后在栈中的偏移displacement in stack
        int rreg;   // 寄存器号 the real register mapped from virtual register if the vreg is not spilled to memory
        bool fp_tag; // 是否为浮点
        std::set<MachineOperand *> defs;
        std::set<MachineOperand *> uses;
    };
    MachineUnit *unit;
    MachineFunction *func;
    std::vector<int> regs;//记录可用real regs（从active释放）
    std::vector<int> fpregs;
    std::map<MachineOperand *, std::set<MachineOperand *>> du_chains;
    std::vector<Interval*> intervals;//unhandled intervals
    std::vector<Interval*> activelist;
    static bool compareStart(Interval*a, Interval*b);
    static bool compareEnd(Interval*a, Interval*b);
    static bool up(int a, int b);
    void expireOldIntervals(Interval *interval);
    void spillAtInterval(Interval *interval);
    void makeDuChains();
    void computeLiveIntervals();
    bool linearScanRegisterAllocation();
    void modifyCode();
    void genSpillCode();
public:
    LinearScan(MachineUnit *unit);
    void allocateRegisters();
};



#endif