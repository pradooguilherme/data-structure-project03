#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct Register
{
    char id_aluno[4];
    char sigla_disc[4];
    char nome_aluno[50];
    char nome_disc[50];
    float media;
    float freq;

} Register;

Register *read_insert_file();
FILE *iniciaLogFile();
FILE *iniciaArquivo();
FILE *iniciaLogFile();
FILE *iniciaArquivo();
int calcula_tamanho(Register *registro);
void insertRegister(Register *registro);