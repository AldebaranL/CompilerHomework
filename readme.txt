expr1.y对应1、2问所述的基本表达式运算的实现
expr2.y对应中、后缀表达式转换的实现
expr3.y对应赋值语句和符号表的实现

编译运行：
bison -d expr2.y
gcc expr2.tab.c -o expr2
./expr2

或：
make
./expr2
