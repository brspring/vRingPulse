#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smpl.h"

// Definição dos eventos
#define HEARTBEAT 1
#define FAULT 2
#define RECOVERY 3
#define CHECK_TIMEOUT 4

// Estrutura de processo
typedef struct {
   int id;                // identificador de facility do SMPL
   int *Timer;            // Timers para cada processo
   int *State;            // Vetor de State (-1 = unknown, 0 = correto, 1 = suspeito)
   int left;              // Vizinho esquerdo
   int right;             // Vizinho direito
} TipoProcesso;

TipoProcesso *processo;

int main(int argc, char *argv[]) {
   static int N,           // número de processos
              token,       // indica o processo que está executando
              event, r, i;
   static int MaxTempoSimulac = 10;
   static char fa_name[5];
   static int T = 2;      // Intervalo de heartbeat

   if (argc != 2) {
      puts("Uso correto: ./vRingPulse <número de processos>");
      exit(1);
   }

   N = atoi(argv[1]);

   smpl(0, "Simulação do Protocolo vRing Pulse");
   reset();
   stream(1);

   // Alocação e inicialização dos processos
   processo = (TipoProcesso *) malloc(sizeof(TipoProcesso) * N);
   if (processo == NULL) {
      puts("Erro ao alocar memória para os processos.");
      exit(1);
   }

   for (i = 0; i < N; i++) {
      processo[i].Timer = (int *) malloc(sizeof(int) * N);
      processo[i].State = (int *) malloc(sizeof(int) * N);
      if (processo[i].Timer == NULL || processo[i].State == NULL) {
         puts("Erro ao alocar memória para timers/State.");
         exit(1);
      }
      
      // Inicializa os estados e timer
      for (int j = 0; j < N; j++) {
         processo[i].State[j] = (i == j) ? 0 : -1;
         processo[i].Timer[j] = 2 * T;
      }

      // Define vizinhos
      processo[i].left = (i - 1 + N) % N;
      processo[i].right = (i + 1) % N;
   }

   // Registrar os processos no SMPL
   for (i = 0; i < N; i++) {
      memset(fa_name, '\0', 5);
      sprintf(fa_name, "%d", i);
      processo[i].id = facility(fa_name, 1);
   }

   // Escalonar os primeiros heartbeats e checagens
   for (i = 0; i < N; i++) {
      schedule(HEARTBEAT, T, i);
      schedule(CHECK_TIMEOUT, 1.0, i);
   }
   
   // Escalonar falhas para teste
   schedule(FAULT, 3.0, 1);
   schedule(FAULT, 3.0, 2);
   schedule(FAULT, 3.0, 3);


   puts("===============================================================");
   puts("           Sistemas Distribuídos - vRing Pulse");
   puts("          LOG da Simulação do Protocolo");
   printf("   Executando para N = %d processos.\n", N);
   printf("   Intervalo de heartbeat T = %d\n", T);
   printf("           Tempo Total de Simulação = %d\n", MaxTempoSimulac);
   puts("===============================================================");

   // Loop de simulação
   while (time() <= MaxTempoSimulac) {
      cause(&event, &token);

      switch (event) {  
         case HEARTBEAT:
            if (status(processo[token].id) != 0) break; // Processo falho não envia heartbeats
            
            // Envia heartbeats para os vizinhos
            printf("Processo %d enviando HEARTBEAT para %d (esq) e %d (dir) no tempo %.1f\n", 
                  token, processo[token].left, processo[token].right, time());
            
            // Reset timer nos vizinhos que receberam o heartbeat
            processo[processo[token].left].Timer[token] = 0;
            processo[processo[token].right].Timer[token] = 0;
      
            // Atualiza estado nos vizinhos (visto como correto)
            processo[processo[token].left].State[token] = 0;
            processo[processo[token].right].State[token] = 0;
            
            // Reagenda o próximo heartbeat
            schedule(HEARTBEAT, T, token);
            break;
            
         case FAULT:
            r = request(processo[token].id, token, 0);
            printf("O processo %d falhou no tempo %.1f\n", token, time());
            break;

         case CHECK_TIMEOUT:
            if (status(processo[token].id) != 0) break; // Se estou falho, não checo
            
            // Incrementa timers para todos os outros
            for (int j = 0; j < N; j++) {
               if (j == token) continue;
               processo[token].Timer[j]++;

               // Verifica timeout
               if (processo[token].Timer[j] >= 2 * T && processo[token].State[j] == 0) {
                  processo[token].State[j] = 1; // Marca como suspeito
                  printf("Processo %d detecta que %d está SUSPEITO no tempo %.1f (timeout)\n",
                        token, j, time());
               }
            }

            // Reagenda checagem
            schedule(CHECK_TIMEOUT, 1.0, token);
            break;
      }

      // Imprime estado dos processos
      printf("\n=========== Estado dos Processos no tempo %.1f ===========\n", time());
      for (int j = 0; j < N; j++) {
         printf("P%d: ", j);
         for (int k = 0; k < N; k++) {
            if (processo[j].State[k] == 0)
               printf(" S%d:CORRETO /", k);
            else if (processo[j].State[k] == 1)
               printf(" S%d:SUSPEITO /", k);
            else
               printf(" S%d:UNKNOWN /", k);
         }
         printf("\n");
      }
      printf("===========================================================\n\n");
   }

   // Libera a memória alocada
   for (i = 0; i < N; i++) {
      free(processo[i].Timer);
      free(processo[i].State);
   }
   free(processo);
   
   return 0;
}
