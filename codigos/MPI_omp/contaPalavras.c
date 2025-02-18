#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#define NUM_THREADS 4
#define MAX_WORDS 21

const char *palavras[] = {
    "amor", "amizade", "casa", "felicidade", "natureza", "sol", "chuva",
    "flor", "montanha", "rio", "vida", "saude", "alegria", "esperanca",
    "sorriso", "trabalho", "familia", "paz", "luz", "estrela", "pspd"
};

/* Função para encontrar o índice da palavra (comparação exata) */
int find_word_index(const char *word) {
    for (int i = 0; i < MAX_WORDS; i++) {
        if (strcmp(word, palavras[i]) == 0)
            return i;
    }
    return -1;
}

int main(int argc, char *argv[]){
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    /* Determina o comprimento máximo entre as palavras (igual ao usado na criação) */
    int max_len = 0;
    for (int i = 0; i < MAX_WORDS; i++){
        int len = strlen(palavras[i]);
        if (len > max_len)
            max_len = len;
    }
    int record_length = max_len + 1; // palavra preenchida + '\n'
    
    MPI_File mpi_file;
    if (MPI_File_open(MPI_COMM_WORLD, "../palavras.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, &mpi_file) != MPI_SUCCESS) {
        if (rank == 0)
            fprintf(stderr, "Erro ao abrir o arquivo.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Obtém o tamanho total do arquivo
    MPI_Offset filesize;
    MPI_File_get_size(mpi_file, &filesize);
    
    // Calcula o total de registros (linhas) no arquivo
    MPI_Offset total_records = filesize / record_length;
    
    // Divide os registros entre os processos
    MPI_Offset records_per_proc = total_records / size;
    MPI_Offset extra = total_records % size;
    MPI_Offset my_records = records_per_proc + (rank < extra ? 1 : 0);
    MPI_Offset start_record = rank * records_per_proc + (rank < extra ? rank : extra);
    MPI_Offset start_offset = start_record * record_length;
    MPI_Offset my_chunk_size = my_records * record_length;
    
    // Aloca buffer para o chunk do processo
    char *buffer = (char*) malloc(my_chunk_size + 1);
    if (!buffer) {
        perror("Erro ao alocar buffer");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Leitura assíncrona com MPI I/O
    MPI_Request request;
    MPI_Status status;
    MPI_File_iread_at(mpi_file, start_offset, buffer, my_chunk_size, MPI_CHAR, &request);
    MPI_Wait(&request, &status);
    buffer[my_chunk_size] = '\0';  // Garante terminação da string
    
    MPI_File_close(&mpi_file);
    
    /* Contagem local de cada palavra */
    int local_counts[MAX_WORDS] = {0};
    
    omp_set_num_threads(NUM_THREADS);
    // Usamos reduction com array para eliminar a seção crítica
    #pragma omp parallel for schedule(static) reduction(+: local_counts[:MAX_WORDS])
    for (long i = 0; i < my_records; i++){
        // Cada registro começa em: buffer + i * record_length
        char *record = buffer + i * record_length;
        
        // Copia os primeiros max_len caracteres para formar a palavra (sem o '\n')
        char temp[max_len + 1];
        memcpy(temp, record, max_len);
        temp[max_len] = '\0';
        
        // Remove os espaços à direita (os preenchimentos)
        for (int j = max_len - 1; j >= 0; j--){
            if (temp[j] == ' ')
                temp[j] = '\0';
            else
                break;
        }
        
        int index = find_word_index(temp);
        if (index != -1)
            local_counts[index]++;
    }
    
    free(buffer);
    
    // Reduz (soma) as contagens locais em um vetor global no processo 0
    int global_counts[MAX_WORDS] = {0};
    MPI_Reduce(local_counts, global_counts, MAX_WORDS, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Frequência de Palavras:\n");
        for (int i = 0; i < MAX_WORDS; i++){
            printf("%s: %d\n", palavras[i], global_counts[i]);
        }
    }
    
    MPI_Finalize();
    return 0;
}
