all: expr3

expr3.tab.c expr3.tab.h: expr3.y
	bison -d expr3.y
expr3: expr3.tab.c expr3.tab.h
	g++ expr3.tab.c -o expr3

expr2.tab.c expr2.tab.h: expr2.y
	bison -d expr2.y
expr2: expr2.tab.c expr2.tab.h
	gcc expr2.tab.c -o expr2
	
expr1.tab.c expr1.tab.h: expr1.y
	bison -d expr1.y
expr1: expr1.tab.c expr1.tab.h
	gcc expr1.tab.c -o expr1
	

calc.tab.c calc.tab.h:	calc.y
	bison -t -v -d calc.y
calc.lex.yy.c: calc.l calc.tab.h
	flex calc.l
calc: calc.lex.yy.c calc.tab.c calc.tab.h
	gcc -o calc calc.tab.c calc.lex.yy.c

clean:
	rm expr3 expr3.tab.c expr3.tab.h expr3.output
	rm expr2 expr2.tab.c expr2.tab.h expr2.output
	rm expr1 expr1.tab.c expr1.tab.h expr1.output
	rm calc calc.tab.c calc.lex.yy.c calc.tab.h calc.output
	