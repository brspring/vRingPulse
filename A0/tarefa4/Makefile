all: tarefa4

tarefa4: tarefa4.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic tarefa4.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

tarefa4.o: tarefa4.c smpl.h
	$(COMPILE.c) -g  tarefa4.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o tarefa4 relat saida
