#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main() {
    FILE *file = fopen("../palavras.txt", "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir o arquivo.\n");
        return 1;
    }
    
    int word_counts[MAX_WORDS] = {0};
    
    // Determina o comprimento máximo das palavras (deve ser igual ao usado na geração)
    int max_len = 0;
    for (int i = 0; i < MAX_WORDS; i++) {
        int len = strlen(palavras[i]);
        if (len > max_len)
            max_len = len;
    }
    int record_length = max_len + 1; // incluindo o '\n'
    
    char buffer[record_length + 1];
    
    while (fgets(buffer, sizeof(buffer), file)) {
        // Remove espaços extras e o '\n' no final
        for (int i = max_len - 1; i >= 0; i--) {
            if (buffer[i] == ' ' || buffer[i] == '\n') {
                buffer[i] = '\0';
            } else {
                break;
            }
        }
        
        int index = find_word_index(buffer);
        if (index != -1) {
            word_counts[index]++;
        }
    }
    
    fclose(file);
    
    printf("Frequência de Palavras:\n");
    for (int i = 0; i < MAX_WORDS; i++) {
        printf("%s: %d\n", palavras[i], word_counts[i]);
    }
    
    return 0;
}