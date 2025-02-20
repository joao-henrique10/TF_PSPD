#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define TOTAL_PALAVRAS 250000000
#define NUM_THREADS 4

const char *palavras[] = {
    "amor", "amizade", "casa", "felicidade", "natureza", "sol", "chuva",
    "flor", "montanha", "rio", "vida", "saude", "alegria", "esperanca",
    "sorriso", "trabalho", "familia", "paz", "luz", "estrela", "pspd"
};

#define TOTAL_OPCOES (sizeof(palavras) / sizeof(palavras[0]))

int main() {
    FILE *arquivo = fopen("palavras.txt", "w");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }
    
    // Determina o comprimento máximo entre as palavras
    int max_len = 0;
    for (int i = 0; i < TOTAL_OPCOES; i++) {
        int len = strlen(palavras[i]);
        if (len > max_len)
            max_len = len;
    }
    // Cada registro terá: [max_len caracteres para a palavra (padded) + 1 para '\n']
    int record_length = max_len + 1;
    
    omp_set_num_threads(NUM_THREADS);
    
    /*  
       Para evitar conflitos de escrita no arquivo compartilhado, 
       cada thread gera seu próprio bloco em um buffer local e depois,
       em seção crítica, escreve-o no arquivo.
    */
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        unsigned int seed = time(NULL) + thread_id;
        
        // Divide TOTAL_PALAVRAS entre as threads (a última pega o resto)
        long palavras_por_thread = TOTAL_PALAVRAS / NUM_THREADS;
        if (thread_id == NUM_THREADS - 1) {
            palavras_por_thread += TOTAL_PALAVRAS % NUM_THREADS;
        }
        
        // Aloca buffer local: cada registro tem record_length bytes
        char *local_buffer = (char *) malloc(palavras_por_thread * record_length + 1);
        if (!local_buffer) {
            perror("Erro ao alocar buffer");
            exit(1);
        }
        
        char *p = local_buffer;
        for (long i = 0; i < palavras_por_thread; i++) {
            // Seleciona uma palavra aleatoriamente
            const char *palavra = palavras[rand_r(&seed) % TOTAL_OPCOES];
            int len = strlen(palavra);
            // Copia a palavra para o buffer
            memcpy(p, palavra, len);
            // Preenche com espaços até atingir max_len (caso a palavra seja menor)
            if (len < max_len) {
                memset(p + len, ' ', max_len - len);
            }
            // Coloca o caractere de separação (newline)
            p[max_len] = '\n';
            p += record_length;
        }
        *p = '\0';
        
        // Escrita no arquivo de forma segura (seção crítica)
        #pragma omp critical
        {
            fputs(local_buffer, arquivo);
        }
        free(local_buffer);
    }
    
    fclose(arquivo);
    printf("Arquivo 'palavras.txt' gerado com sucesso!\n");
    printf("Cada registro possui %d bytes (incluindo o '\\n').\n", record_length);
    return 0;
}
