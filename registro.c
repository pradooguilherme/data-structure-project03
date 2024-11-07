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

Register *read_insert_file()
{
    FILE *arquivo = fopen("insere.bin", "r+b");

    Register *registros = (Register *)malloc(100 * sizeof(Register));

    if (registros != NULL)
    {
        fread(registros, sizeof(struct Register), 100, arquivo);

        fclose(arquivo);
        return registros;
    }

    printf("Falha na alocação de memória para o registro de inserção\n");

    fclose(arquivo);
    return NULL;
}

FILE *iniciaLogFile()
{
    FILE *log_file = fopen("log_file.bin", "r+b");

    if (log_file == NULL)
    {

        log_file = fopen("log_file.bin", "w+b");

        if (log_file != NULL)
        {
            int index = -1;

            fwrite(&index, sizeof(int), 1, log_file);
            fseek(log_file, 0, SEEK_SET);

            return log_file;
        }
        else
        {
            printf("Falha na abertura do arquivo log.\n");
            return NULL;
        }
    }

    return log_file;
}

FILE *iniciaArquivo()
{

    FILE *seuArquivo = fopen("dados.bin", "r+b");

    if (seuArquivo == NULL)
    {

        seuArquivo = fopen("dados.bin", "w+b");

        if (seuArquivo != NULL)
        {

            int byte_offset = -1;
            bool operacao_flag = false;

            fwrite(&operacao_flag, sizeof(bool), 1, seuArquivo);
            fseek(seuArquivo, 0, SEEK_SET);

            return seuArquivo;
        }
        else
        {
            printf("Falha na abertura do arquivo dados.\n");
            return NULL;
        }
    }
    return seuArquivo;
}

int calcula_tamanho(Register *registro)
{

    int tam = 0;

    tam += strlen(registro->id_aluno);
    tam += strlen(registro->sigla_disc);
    tam += strlen(registro->nome_aluno);
    tam += strlen(registro->nome_disc);
    tam += sizeof(registro->media);
    tam += sizeof(registro->freq);
    tam = tam + 5;

    return tam;
}

void insertRegister(Register *registro)
{
    FILE *log_file = iniciaLogFile();
    FILE *data_file = iniciaArquivo();

    char delimitador = '#';
    int i = 0, tam_reg = 0, tam_data = 0;
    int offset_last, next_byte_offset, actual_byte_offset;

    bool operation_flag, is_inserted, flag = true;
    fread(&operation_flag, sizeof(bool), 1, data_file);

    if (operation_flag)
    {
        fread(&i, sizeof(int), 1, log_file);
        i++;
    }

    fseek(data_file, 0, SEEK_SET);
    fwrite(&flag, sizeof(bool), 1, data_file);

    tam_reg = calcula_tamanho(&registro[i]);

    fseek(data_file, 0, SEEK_END);

    fwrite(&tam_reg, sizeof(int), 1, data_file);
    fwrite(registro[i].id_aluno, sizeof(char), 3, data_file);
    fwrite(&delimitador, sizeof(char), 1, data_file);
    fwrite(registro[i].sigla_disc, sizeof(char), 3, data_file);
    fwrite(&delimitador, sizeof(char), 1, data_file);
    fwrite(registro[i].nome_aluno, strlen(registro[i].nome_aluno), 1, data_file);
    fwrite(&delimitador, sizeof(char), 1, data_file);
    fwrite(registro[i].nome_disc, strlen(registro[i].nome_disc), 1, data_file);
    fwrite(&delimitador, sizeof(char), 1, data_file);
    fwrite(&registro[i].media, sizeof(float), 1, data_file);
    fwrite(&delimitador, sizeof(char), 1, data_file);
    fwrite(&registro[i].freq, sizeof(float), 1, data_file);

    fseek(log_file, 0, SEEK_SET);
    fwrite(&i, sizeof(int), 1, log_file);

    fclose(log_file);
    fclose(data_file);

    return;
}
