all: tarefa3

tarefa3: tarefa3.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa3.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

tarefa3.o: tarefa3.c smpl.h
	$(COMPILE.c) -g  tarefa3.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o tarefa3 relat saida
