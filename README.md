# 2022Fall NKUCS Course - Principle of Compilers

编译器的实现，包括词法分析、语法分析、中间代码生成、目标代码生成、类型检查等。

待优化，另见https://gitlab.eduxiji.net/nku_hl/

项目实现报告见document.pdf

## 项目结构

```
./
include/
	Ast.h					抽象语法树相关类
	SymbolTable.h			符号表相关类
	Type.h					类型系统相关类
	IRBuilder.h				中间代码构造辅助类
	Unit.h					编译单元
	Function.h				函数
	BasicBlock.h			基本块
	Instruction.h			指令
	Operand.h				指令操作数
	AsmBuilder.h			汇编代码构造辅助类
    MachineCode.h			汇编代码构造相关类
	LinearScan.h			线性扫描寄存器分配相关类
	LiveVariableAnalysis.h	活跃变量分析相关类
src/
	Ast.cpp
	BasicBlock.cpp
	Function.cpp
	Instruction.cpp
	lexer.l					词法分析器
	main.cpp
	Operand.cpp
	parser.y				语法分析器
	SymbolTable.cpp
	Type.cpp
	Unit.cpp
	MachineCode.cpp
	LinearScan.cpp
	LiveVariableAnalysis.cpp
	sysyruntimelibrary
test/						测试样例
sysyruntimelibrary/	 		SysY 运行时库
.gitignore
example.sy
Makefile
```



## 编译器命令

```
Usage：build/compiler [options] infile
Options:
    -o <file>   Place the output into <file>.
    -t          Print tokens.
    -a          Print abstract syntax tree.
    -i          Print intermediate code
    -S          Print assembly code
```

## VSCode调试

提供了VSCode调试所需的`json`文件，使用前需正确设置`launch.json`中`miDebuggerPath`中`gdb`的路径。`launch.json`中`args`值即为编译器的参数，可自行调整。

## Makefile使用

* 修改测试路径：

默认测试路径为test，你可以修改为任意要测试的路径。我们已将最终所有测试样例分级上传。

如：要测试level1-1下所有sy文件，可以将makefile中的

```
TEST_PATH ?= test
```

修改为

```
TEST_PATH ?= test/level1-1
```

* 编译：

```
    make
```
编译出我们的编译器。

* 运行：
```
    make run
```
以example.sy文件为输入，输出相应的汇编代码到example.s文件中。

* 测试：
```
    make testlab7
```
该命令会搜索TEST_PATH目录下所有的.sy文件，逐个输入到编译器中，生成相应的汇编代码.s文件。你还可以指定测试目录：
```
    make testlab7 TEST_PATH=dirpath
```
* 批量测试：
```
    make test
```
对TEST_PATH目录下的每个.sy文件，编译器将其编译成汇编代码.s文件， 再使用gcc将.s文件汇编成二进制文件后执行， 将得到的输出与标准输出对比， 验证编译器实现的正确性。错误信息描述如下：
|  错误信息   | 描述  |
|  ----  | ----  |
| Compile Timeout  | 编译超时， 可能是编译器实现错误导致， 也可能是源程序过于庞大导致(可调整超时时间) |
| Compile Error  | 编译错误， 源程序有错误或编译器实现错误 |
|Assemble Error| 汇编错误， 编译器生成的目标代码不能由gcc正确汇编|
| Execute Timeout  |执行超时， 可能是编译器生成了错误的目标代码|
|Execute Error|程序运行时崩溃， 可能原因同Execute Timeout|
|Wrong Answer|答案错误， 执行程序得到的输出与标准输出不同|

具体的错误信息可在对应的.log文件中查看。

* GCC Assembly Code
```
    make gccasm
```
使用gcc编译器生成汇编代码。

* 清理:
```
    make clean
```
清除所有可执行文件和测试输出。
```
    make clean-test
```
清除所有测试输出。
```
    make clean-app
```
清除编译器可执行文件。