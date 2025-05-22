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
   int* State;
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
   if (processo == NULL) {
      puts("Erro ao alocar memória para os processos.");
      exit(1);
   }
   for (i = 0; i < N; i++) {
      processo[i].State = (int *) malloc(sizeof(int) * N);
      if (processo[i].State == NULL) {
         puts("Erro ao alocar memória para os estados dos processos.");
         exit(1);
      }
      memset(processo[i].State, 0, sizeof(int) * N);
   }

   // Inicializa o estado de cada processo
   for (i = 0; i < N; i++) {
      processo[i].State[i] = 0; // Cada processo é correto inicialmente
      for (int j = 0; j < N; j++) {
         if (j != i) {
            processo[i].State[j] = -1; // Inicializa os demais como unknown
         }
      }
   }
   
   for (i = 0; i < N; i++) {
      memset(fa_name, '\0', 5);
      sprintf(fa_name, "%d", i);
      processo[i].id = facility(fa_name, 1);
   }

   // Escalonar o primeiro teste de cada processo
   for (i = 0; i < N; i++) {
      schedule(test, 30.0, i);
   }
   for (i = 1; i < 3; i++) {
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

      int testados[N];
      int next = (token + 1) % N;
      switch (event) {  
         case test:
            if (status(processo[token].id) != 0) break; 

            for (int j = 0; j < N; j++) testados[j] = 0;
            
            while (status(processo[next].id) != 0) {
               printf("O processo %d testou o processo %d FALHO no tempo %.1f\n", token, next, time());
               processo[token].State[next] = 1; // Marca o próximo como falho
               testados[next] = 1; // Marca o próximo como testado
               next = (next + 1) % N;
               if (next == token) break;
            }
            if(next != token){
               printf("O processo %d testou o processo %d CORRETO no tempo %.1f\n", token, next, time());
               processo[token].State[next] = 0;
               testados[next] = 1; // Também marca como testado

               // Copia o estado do processo correto (next)
               for (int j = 0; j < N; j++) {
                     if (j != token && testados[j] == 0) {
                        processo[token].State[j] = processo[next].State[j];
                     }
               }
            }

            // Último processo testador
            if (token == N - 1) {
               printf("\n=========== Estado dos Processos no tempo %.1f ===========\n", time());
               for (int j = 0; j < N; j++) {
                     printf("P%d: ", j);
                     for (int k = 0; k < N; k++) {
                        if (processo[j].State[k] == 0)
                           printf(" S%d:CORRETO /", k);
                        else if (processo[j].State[k] == 1)
                           printf(" S%d:FALHO /", k);
                        else
                           printf(" S%d:UNKNOWN /", k);
                     }
                     printf("\n");
               }
               printf("===========================================================\n\n");
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

   // Libera a memória alocada
   for (i = 0; i < N; i++) {
      free(processo[i].State);
   }

   free(processo);
   return 0;
}
