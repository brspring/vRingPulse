all: vRingPulse

vRingPulse: vRingPulse.o smpl.o rand.o
	$(LINK.c) -o $@ -Bstatic vRingPulse.o smpl.o rand.o -lm

smpl.o: smpl.c smpl.h
	$(COMPILE.c)  -g smpl.c

vRingPulse.o: vRingPulse.c smpl.h
	$(COMPILE.c) -g  vRingPulse.c

rand.o: rand.c
	$(COMPILE.c) -g rand.c

clean:
	$(RM) *.o vRingPulse relat saida
