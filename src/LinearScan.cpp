#include <algorithm>
#include "LinearScan.h"
#include "MachineCode.h"
#include "LiveVariableAnalysis.h"

LinearScan::LinearScan(MachineUnit *unit)
{
    this->unit = unit;
    for (int i = 4; i < 11; i++)
        regs.push_back(i);
}

void LinearScan::allocateRegisters()
{
    for (auto &f : unit->getFuncs())
    {
        func = f;
        bool success;
        success = false;
        while (!success)        // repeat until all vregs can be mapped
        {
            computeLiveIntervals();
            success = linearScanRegisterAllocation();
            if (success)        // all vregs can be mapped to real regs
                modifyCode();
            else{                // spill vregs that can't be mapped to real regs
                genSpillCode();
            }
        }
        cout<<"success??---- "<<success<<endl;
    }
}

void LinearScan::makeDuChains()
{
    LiveVariableAnalysis lva;
    lva.pass(func);
    du_chains.clear();
    int i = 0;
    std::map<MachineOperand, std::set<MachineOperand *>> liveVar;
    for (auto &bb : func->getBlocks())
    {
        liveVar.clear();
        for (auto &t : bb->getLiveOut())
            liveVar[*t].insert(t);
        int no;
        no = i = bb->getInsts().size() + i;
        for (auto inst = bb->getInsts().rbegin(); inst != bb->getInsts().rend(); inst++)
        {
            (*inst)->setNo(no--);
            for (auto &def : (*inst)->getDef())
            {
                if (def->isVReg())
                {
                    auto &uses = liveVar[*def];
                    du_chains[def].insert(uses.begin(), uses.end());
                    auto &kill = lva.getAllUses()[*def];
                    std::set<MachineOperand *> res;
                    set_difference(uses.begin(), uses.end(), kill.begin(), kill.end(), inserter(res, res.end()));
                    liveVar[*def] = res;
                }
            }
            for (auto &use : (*inst)->getUse())
            {
                if (use->isVReg())
                    liveVar[*use].insert(use);
            }
        }
    }
}

void LinearScan::computeLiveIntervals()
{
    makeDuChains();
    intervals.clear();
    for (auto &du_chain : du_chains)
    {
        int t = -1;
        for (auto &use : du_chain.second)
            t = std::max(t, use->getParent()->getNo());
        Interval *interval = new Interval({du_chain.first->getParent()->getNo(), t, false, 0, 0, 
                                            du_chain.first->isFloat(),
                                            {du_chain.first}, du_chain.second});
        intervals.push_back(interval);
    }
    for (auto& interval : intervals) {
        auto uses = interval->uses;
        auto begin = interval->start;
        auto end = interval->end;
        for (auto block : func->getBlocks()) {
            auto liveIn = block->getLiveIn();
            auto liveOut = block->getLiveOut();
            bool in = false;
            bool out = false;
            for (auto use : uses)
                if (liveIn.count(use)) {
                    in = true;
                    break;
                }
            for (auto use : uses)
                if (liveOut.count(use)) {
                    out = true;
                    break;
                }
            if (in && out) {
                begin = std::min(begin, (*(block->begin()))->getNo());
                end = std::max(end, (*(block->rbegin()))->getNo());
            } else if (!in && out) {
                for (auto i : block->getInsts())
                    if (i->getDef().size() > 0 &&
                        i->getDef()[0] == *(uses.begin())) {
                        begin = std::min(begin, i->getNo());
                        break;
                    }
                end = std::max(end, (*(block->rbegin()))->getNo());
            } else if (in && !out) {
                begin = std::min(begin, (*(block->begin()))->getNo());
                int temp = 0;
                for (auto use : uses)
                    if (use->getParent()->getParent() == block)
                        temp = std::max(temp, use->getParent()->getNo());
                end = std::max(temp, end);
            }
        }
        interval->start = begin;
        interval->end = end;
    }
    bool change;
    change = true;
    while (change)
    {
        change = false;
        std::vector<Interval *> t(intervals.begin(), intervals.end());
        for (size_t i = 0; i < t.size(); i++)
            for (size_t j = i + 1; j < t.size(); j++)
            {
                Interval *w1 = t[i];
                Interval *w2 = t[j];
                if (**w1->defs.begin() == **w2->defs.begin())
                {
                    std::set<MachineOperand *> temp;
                    set_intersection(w1->uses.begin(), w1->uses.end(), w2->uses.begin(), w2->uses.end(), inserter(temp, temp.end()));
                    if (!temp.empty())
                    {
                        change = true;
                        w1->defs.insert(w2->defs.begin(), w2->defs.end());
                        w1->uses.insert(w2->uses.begin(), w2->uses.end());
                        // w1->start = std::min(w1->start, w2->start);
                        // w1->end = std::max(w1->end, w2->end);
                        auto w1Min = std::min(w1->start, w1->end);
                        auto w1Max = std::max(w1->start, w1->end);
                        auto w2Min = std::min(w2->start, w2->end);
                        auto w2Max = std::max(w2->start, w2->end);
                        w1->start = std::min(w1Min, w2Min);
                        w1->end = std::max(w1Max, w2Max);
                        auto it = std::find(intervals.begin(), intervals.end(), w2);
                        if (it != intervals.end())
                            intervals.erase(it);
                    }
                }
            }
    }
    sort(intervals.begin(), intervals.end(), compareStart);
}

bool LinearScan::linearScanRegisterAllocation()
{
    // Todo
    bool success=true;
    activelist.clear();
    regs.clear();
    fpregs.clear();
    for (int i = 4; i < 11; i++)
        regs.push_back(i);
    for (int i = 5; i < 32; i++)
        fpregs.push_back(i+16);
    for(auto& interval:intervals){
        // cout<<"--------------------------"<<endl;
        // for(int x=0;x<regs.size();x++){
        //     cout<<regs[x]<<" ";
        // }
        // cout<<endl;
        //寻找可用空间
        expireOldIntervals(interval);
        if((!interval->fp_tag&&regs.empty())||(interval->fp_tag&&fpregs.empty())){
            //return false;
            spillAtInterval(interval);
            success=false;
            // break;//不能break，还要继续
        }
        else{
            if(!interval->fp_tag){
                interval->rreg = regs.front();//分配寄存器
                regs.erase(regs.begin());
            }
            else{
                interval->rreg = fpregs.front();//分配寄存器
                fpregs.erase(fpregs.begin());
            }
            activelist.push_back(interval);
            //cout<<"interval->rreg:"<<interval->rreg<<endl;
            
            //必须要在里面sort，因为每次循环expireOldIntervals会默认activelist已排序
            sort(activelist.begin(), activelist.end(), compareEnd);
        }
    }
    cout<<"success?"<<success<<endl;
    return success;
}

void LinearScan::modifyCode()
{
    for (auto &interval : intervals)
    {
        func->addSavedRegs(interval->rreg);
        for (auto def : interval->defs)
            def->setReg(interval->rreg);
        for (auto use : interval->uses)
            use->setReg(interval->rreg);
    }
}

// 生成溢出代码
void LinearScan::genSpillCode()
{
    for (auto& interval : intervals) {
        if (!interval->spill)
            continue;
        /* HINT:
         * The vreg should be spilled to memory.
         * 1. insert ldr inst before the use of vreg
         * 2. insert str inst after the def of vreg
         */
        interval->disp = -func->AllocSpace(4);//分配栈空间 记录溢出的栈偏移
        cout<<"interval->disp:"<<interval->disp<<endl;
        auto off = new MachineOperand(MachineOperand::IMM, interval->disp);
        auto fp = new MachineOperand(MachineOperand::REG, 11);

        //扫描uselist 使用前ldr
        for (auto use : interval->uses) {
            auto temp = new MachineOperand(*use);
            MachineOperand* operand = nullptr;

            //ldr operand =<off> 加载偏移立即数
            if (interval->disp > 255 || interval->disp < -255) {
                operand = new MachineOperand(MachineOperand::VREG,SymbolTable::getLabel());
                auto inst1 =new LoadMInstruction(use->getParent()->getParent(),LoadMInstruction::LDR,operand, off);
                use->getParent()->insertBefore(inst1);
            }
            //off是立即数还是在寄存器中
            if (operand) {
                if(!use->isFloat()){
                    //ldr reg [fp, operand]
                    auto inst = new LoadMInstruction(use->getParent()->getParent(),LoadMInstruction::LDR,temp, fp, new MachineOperand(*operand));
                    use->getParent()->insertBefore(inst);
                }
                else{
                    auto reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    //add reg fp operand
                    MachineInstruction* inst = new BinaryMInstruction(use->getParent()->getParent(), BinaryMInstruction::ADD, reg, fp, new MachineOperand(*operand));
                    use->getParent()->insertBefore(inst);
                    //ldr temp [reg]
                    inst = new LoadMInstruction(use->getParent()->getParent(), LoadMInstruction::VLDR, temp, new MachineOperand(*reg));
                    use->getParent()->insertBefore(inst);
                }
            } else {
                if(!use->isFloat()){
                    auto inst = new LoadMInstruction(use->getParent()->getParent(),LoadMInstruction::LDR,temp, fp, off);
                    use->getParent()->insertBefore(inst);
                }
                else{
                    auto inst = new LoadMInstruction( use->getParent()->getParent(), LoadMInstruction::VLDR, temp, fp, off);
                    use->getParent()->insertBefore(inst);
                }
            }
        }

        //扫描deflist 定义时str
        //同理
        for (auto def : interval->defs) {
            auto temp = new MachineOperand(*def);
            MachineOperand* operand = nullptr;
            MachineInstruction *inst1 = nullptr, *inst = nullptr;
            if (interval->disp > 255 || interval->disp < -255) {
                operand = new MachineOperand(MachineOperand::VREG,SymbolTable::getLabel());
                inst1 = new LoadMInstruction(def->getParent()->getParent(),LoadMInstruction::LDR,operand, off);
                def->getParent()->insertAfter(inst1);//
            }
            if (operand) {
                if (!def->isFloat()) {
                    inst = new StoreMInstruction(def->getParent()->getParent(),StoreMInstruction::STR, temp, fp, new MachineOperand(*operand));
                }
                else{
                    auto reg = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                    MachineInstruction* tmp_inst = new BinaryMInstruction(def->getParent()->getParent(), BinaryMInstruction::ADD, reg, fp, new MachineOperand(*operand));
                    inst1->insertAfter(tmp_inst);
                    inst1 = tmp_inst;
                    inst = new StoreMInstruction(def->getParent()->getParent(), StoreMInstruction::VSTR, temp, new MachineOperand(*reg));
                }
            } else {
                if (!def->isFloat()) {
                    inst = new StoreMInstruction(def->getParent()->getParent(),StoreMInstruction::STR,temp,fp, off);
                }
                else {
                    inst = new StoreMInstruction(def->getParent()->getParent(),StoreMInstruction::VSTR, temp, fp, off);
                }
            }
            if (inst1)
                inst1->insertAfter(inst);
            else
                def->getParent()->insertAfter(inst);
        }
    }
}

//根据（unhandled）interval的开始时间和active的结束时间，踢掉一些active占用的寄存器
void LinearScan::expireOldIntervals(Interval *interval)
{
    // Todo
    vector<Interval*>::iterator it = activelist.begin();

    // 从前往后（activelist有序 按结束时间递增）
    // cout<<"---------------------"<<endl;
    while (it != activelist.end()) {
        if ((*it)->end >= interval->start)
            return;
        // r regs
        if ((*it)->rreg < 11) {
            // cout<<"erase!"<<endl;
            // cout<<"rreg:"<<(*it)->rreg<<endl;
            regs.push_back((*it)->rreg);
            //it++;//不能这样！！一边遍历一边删除
            //it迭代放最后！！
            it=activelist.erase(find(activelist.begin(), activelist.end(), *it));//erase返回下一个位置    
        } 
        // fp regs
        else{
            fpregs.push_back((*it)->rreg);
            it = activelist.erase(find(activelist.begin(), activelist.end(), *it));
        }
    }
    //sort(fpregs.begin(), fpregs.end(), up);
}

//寄存器溢出 找结束时间最晚的
void LinearScan::spillAtInterval(Interval *interval)
{
    // Todo
    auto last=activelist.back();
    if(last->end>interval->end){
        //溢出activelist的最后一项
        //需要先把interval加入activelist，再溢出
        last->spill=true;//spill置位
        interval->rreg=last->rreg;//把last的寄存器换给interval用
        activelist.push_back(interval);
        sort(activelist.begin(), activelist.end(), compareEnd);
    }
    else{
        //溢出interval，只需将interval->spill置位
        interval->spill=true;
    }
}

bool LinearScan::compareStart(Interval *a, Interval *b)
{
    return a->start < b->start;
}

bool LinearScan::compareEnd(Interval* a, Interval* b) 
{
    return a->end < b->end;
}

bool LinearScan::up(int a, int b) 
{
    return a < b;
}