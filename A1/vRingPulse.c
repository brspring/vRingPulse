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

void print_states(int token, int N) {
   //printf("Processo %d checando timeouts no tempo %.1f\n", token, time());
   printf("\n======================= Vetor state de cada processo no tempo %.1f ================================\n", time());
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
   printf("====================================================================================================\n\n");
}

int main(int argc, char *argv[]) {
   static int N,           // número de processos
              token,       // indica o processo que está executando
              event, r, i;
   static int MaxTempoSimulac = 150;
   static char fa_name[5];
   static int T = 30;      // Intervalo de heartbeat

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
         processo[i].Timer[j] = 0;
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
      schedule(CHECK_TIMEOUT, T, i);
   }
   
   // Escalonar falhas para teste
   schedule(FAULT, 31.0, 0);
   schedule(FAULT, 31.0, 1);
   schedule(FAULT, 31.0, 3);

   puts("===============================================================");
   puts("           Sistemas Distribuídos - vRing Pulse");
   puts("          LOG da Simulação do Protocolo");
   printf("   Executando para N = %d processos.\n", N);
   printf("   Intervalo de heartbeat T = %d\n", T);
   printf("           Tempo Total de Simulação = %d\n", MaxTempoSimulac);
   puts("===============================================================");

   // Inicializa os tempos de checagem
   double last_check_time[N];
   for (int i = 0; i < N; i++) last_check_time[i] = time();

   // Loop de simulação
   while (time() <= MaxTempoSimulac) {
      cause(&event, &token);

      switch (event) {  
         case HEARTBEAT:
            if (status(processo[token].id) != 0) break; // Processo falho não envia heartbeats

            // Envia heartbeats para os vizinhos
            printf("\nProcesso %d enviando HEARTBEAT para %d (esq) e %d (dir) no tempo %.1f\n", 
                  token, processo[token].left, processo[token].right, time());

            // Reset timer nos vizinhos que receberam o heartbeat
            processo[processo[token].left].Timer[token] = 0;
            processo[processo[token].right].Timer[token] = 0;

            // Atualiza estado local dos vizinhos sobre este processo (token está correto)
            processo[processo[token].left].State[token] = 0;  // CORRETO
            processo[processo[token].right].State[token] = 0;

            for (int s = 0; s < N; s++) {
               int estado_local = processo[token].State[s];
               if (processo[processo[token].left].State[s] < estado_local)
                  processo[processo[token].left].State[s] = estado_local;

               if (processo[processo[token].right].State[s] < estado_local)
                  processo[processo[token].right].State[s] = estado_local;
            }

            // Reagenda o próximo heartbeat
            schedule(HEARTBEAT, T, token);
            break;
            
         case FAULT:
            r = request(processo[token].id, token, 0);

            printf("\n");
            printf("================================================================\n");
            printf("O processo %d falhou no tempo %.1f\n", token, time());
            printf("================================================================\n");

            break;
         case CHECK_TIMEOUT:
            if (status(processo[token].id) != 0) break; // Se estou falho, não checo

            double delta = time() - last_check_time[token];
            last_check_time[token] = time();
            int vizinhos[2] = {processo[token].left, processo[token].right};

            for (int i = 0; i < 2; i++) {
               int j = vizinhos[i];
               processo[token].Timer[j] += delta;

               //printf("Processo %d checando o timer de %d: %d\n", token, j, processo[token].Timer[j]);

               if (processo[token].Timer[j] >= 2*T) {
                     processo[token].State[j] = 1; // Marca como suspeito
                     printf("Processo %d detecta que %d está SUSPEITO no tempo %.1f (timeout)\n", token, j, time());

                     int salto = 1;
                     int k = j;

                     while (salto < N) {
                        if (j == processo[token].left)
                           k = (j - salto + N) % N;  // lado esquerdo
                        else
                           k = (j + salto) % N;      // lado direito

                        if (k == token) break; // não envia para si mesmo

                        if (processo[token].State[k] == 1) {
                           salto++;
                           continue;
                        }

                        // Simular envio de heartbeat para k e propagação do State[]
                        processo[k].Timer[token] = 0;
                        processo[k].State[token] = 0;

                        for (int s = 0; s < N; s++) {
                           int estado_local = processo[token].State[s];
                           if (processo[k].State[s] == -1 || processo[k].State[s] < estado_local) {
                                 processo[k].State[s] = estado_local;
                           }
                        }

                        break; // encontrou um processo correto para receber
                     }

                     if (processo[token].left == j) {
                        int old = processo[token].left;
                        processo[token].left = (processo[token].left - 1 + N) % N;
                        printf("Processo %d atualiza vizinho ESQUERDO: %d -> %d\n", token, old, processo[token].left);
                     }
                     if (processo[token].right == j) {
                        int old = processo[token].right;
                        processo[token].right = (processo[token].right + 1) % N;
                        printf("Processo %d atualiza vizinho DIREITO: %d -> %d\n", token, old, processo[token].right);
                     }
               }
            }

            print_states(token, N);   
            schedule(CHECK_TIMEOUT, T, token);
            break;

      }
   }

   // Libera a memória alocada
   for (i = 0; i < N; i++) {
      free(processo[i].Timer);
      free(processo[i].State);
   }
   free(processo);
   
   return 0;
}
