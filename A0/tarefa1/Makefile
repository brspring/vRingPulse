all: tarefa1

tarefa1: tarefa1.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa1.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

tarefa1.o: tarefa1.c smpl.h
	$(COMPILE.c) -g  tarefa1.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o tarefa1 relat saida
