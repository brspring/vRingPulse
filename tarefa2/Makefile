all: tarefa2

tarefa2: tarefa2.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa2.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

tarefa2.o: tarefa2.c smpl.h
	$(COMPILE.c) -g  tarefa2.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o tarefa2 relat saida
