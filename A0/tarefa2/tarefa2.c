#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smpl.h"

// Definição dos eventos
#define test 1
#define fault 2
#define recovery 3

// Estrutura de processo
typedef struct {
   int id; // identificador de facility do SMPL
} TipoProcesso;

TipoProcesso *processo;

int main(int argc, char *argv[]) {
   static int N,       // número de processos
            token,   // indica o processo que está executando
            event, r, i;
   static int MaxTempoSimulac = 150;
   static char fa_name[5];

   if (argc != 2) {
      puts("Uso correto: ./tarefa1 <número de processos>");
      exit(1);
   }

   N = atoi(argv[1]);

   smpl(0, "Simulação de Teste em Anel");
   reset();
   stream(1);

   // Alocação e inicialização dos processos
   processo = (TipoProcesso *) malloc(sizeof(TipoProcesso) * N);

   for (i = 0; i < N; i++) {
      memset(fa_name, '\0', 5);
      sprintf(fa_name, "%d", i);
      processo[i].id = facility(fa_name, 1);
   }

   // Escalonar o primeiro teste de cada processo
   for (i = 0; i < N; i++) {
      schedule(test, 30.0, i);
   }
   for (i = 1; i < N; i++) {
      schedule(fault, 31.0, i);
   }

   schedule(recovery, 61.0, 1);


   puts("===============================================================");
   puts("           Sistemas Distribuídos - Prof. Elias");
   puts("          LOG do Trabalho Prático 0, Tarefa 1");
   printf("   Executando para N = %d processos.\n", N);
   printf("           Tempo Total de Simulação = %d\n", MaxTempoSimulac);
   puts("===============================================================");

   // Loop de simulação
   while (time() <= MaxTempoSimulac) {
      cause(&event, &token);

      int next = (token + 1) % N;
      switch (event) {  
         case test:
            if (status(processo[token].id) != 0) break;
            while (status(processo[next].id) != 0) {
               printf("O processo %d testou o processo %d FALHO no tempo %.1f\n", token, next, time());
               next = (next + 1) % N;
               if (next == token) break;
            }
            if(next != token){
               printf("O processo %d testou o processo %d CORRETO no tempo %.1f\n", token, next, time());
            }
            // Reagenda o próximo teste para 30 unidades de tempo depois
            schedule(test, 30.0, token);
            break;

         case fault:
            r = request(processo[token].id, token, 0);
            printf("O processo %d falhou no tempo %.1f\n", token, time());
            break;
         case recovery:
            release(processo[token].id, token);
            printf("O processo %d recuperou no tempo %4.1f\n", token, time());
            schedule(test, 1.0, token);
            break;
      }
   }

   free(processo);
   return 0;
}
