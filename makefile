all: main

#FINAL PHASE SUBMISSION WITH OPTIMIZATION
# Link to generate the main executable: tiger
main: main.o semant.o types.o table.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o tree.o temp.o frame.o translate.o printtree.o prabsyn.o
	cc -o tiger -g main.o semant.o types.o table.o symbol.o absyn.o y.tab.o lex.yy.o errormsg.o util.o tree.o temp.o frame.o translate.o printtree.o prabsyn.o

main.o: main.c
	cc -g -c main.c

prabsyn.o: prabsyn.c prabsyn.h
	cc -g -c prabsyn.c

# Added for Assignment 5 START

translate.o: translate.c translate.h frame.h translate.h mipsframe.h tree.h
	cc -g -c translate.c

frame.o: mipsframe.c mipsframe.h frame.h
	cc -o frame.o -g -c mipsframe.c

printtree.o: printtree.c printtree.h
	cc -g -c printtree.c

tree.o: tree.c tree.h
	cc -g -c tree.c

temp.o: temp.c temp.h
	cc -g -c temp.c
# Added for Assignment 5 END

table.o: table.c
	cc -g -c table.c

y.tab.o: y.tab.c
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

errormsg.o: errormsg.c errormsg.h util.h
	cc -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

absyn.o: absyn.c absyn.h
	cc -g -c absyn.c

types.o: types.c types.h
	cc -g -c types.c

semant.o: semant.c semant.h
	cc -g -c semant.c
prabsyn.o: prabsyn.c prabsyn.h
	cc -g -c prabsyn.c
lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	cc -g -c util.c

clean: 
	rm -f tiger  a.out *.o y.tab.c y.tab.h y.tab.o
