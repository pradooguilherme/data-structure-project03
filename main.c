#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAXKEYS 3
#define MINKEYS 1
#define NIL (-1)
#define NOKEY "@@@@@@"
#define NO 0
#define YES 1
#define DUPLICADA -1
#define DELIMITADOR '#'

typedef struct
{
    short keycount;           // number of keys in page
    char key[MAXKEYS][7];     // the actual keys
    short child[MAXKEYS + 1]; // pointers to rrns of descendants
    short byteofset[MAXKEYS];

} BTPAGE;

#define PAGESIZE sizeof(BTPAGE)

short root; // rrn of root page
FILE *btfd; // file descriptor of btree file

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

void printBTPAGE(BTPAGE *page_ptr)
{
    printf("------------------------------------ PAGINA -------------------------------\n");

    // Imprime o número de chaves
    printf("Key Count: %d\n", page_ptr->keycount);

    // Imprime as chaves
    printf("Keys:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("Key %d: %s\n", i, page_ptr->key[i]);
    }

    // Imprime os ponteiros para descendentes
    printf("Child Pointers:\n");
    for (int i = 0; i <= 3; i++)
    {
        printf("Child %d: %d\n", i, page_ptr->child[i]);
    }

    // Imprime os deslocamentos em bytes
    printf("Byte Offsets:\n");
    for (int i = 0; i < 3; i++)
    {
        printf("Byte Offset %d: %d\n", i, page_ptr->byteofset[i]);
    }

    printf("------------------------------------ PAGINA -------------------------------\n");
}

void pageinit(BTPAGE *p_page)
{
    for (int j = 0; j < MAXKEYS; j++)
    {
        strcpy(p_page->key[j], NOKEY);
        p_page->child[j] = NIL;
        p_page->byteofset[j] = NIL;
    }
    p_page->child[MAXKEYS] = NIL;
}

short getpage()
{
    long addr;
    fseek(btfd, 0, SEEK_END);
    addr = ftell(btfd);

    return ((short)(addr / PAGESIZE));
}

void btwrite(short rrn, BTPAGE *page_ptr)
{
    long addr;
    addr = (long)rrn * (long)PAGESIZE + sizeof(short);

    fseek(btfd, addr, SEEK_SET);

    fwrite(&page_ptr->keycount, sizeof(short), 1, btfd);
    fwrite(page_ptr->key[0], sizeof(char), 7, btfd);
    fwrite(page_ptr->key[1], sizeof(char), 7, btfd);
    fwrite(page_ptr->key[2], sizeof(char), 7, btfd);
    fwrite(page_ptr->child, sizeof(short), MAXKEYS + 1, btfd);
    fwrite(page_ptr->byteofset, sizeof(short), MAXKEYS, btfd);
}

bool btopen()
{
    btfd = fopen("btree.bin", "r+");
    return (btfd != NULL);
}

void btclose()
{
    fclose(btfd);
}

short getroot()
{
    short root;
    fseek(btfd, 0, SEEK_SET);

    if (fread(&root, sizeof(root), 1, btfd) == 0)
    {
        printf("Error: Unable to get root. \007\n");
        exit(1);
    }

    return root;
}

void putroot(short root)
{
    fseek(btfd, 0, SEEK_SET);
    fwrite(&root, sizeof(root), 1, btfd);
}

short create_root(char key[7], short left, short right, short byteofSet)
{
    BTPAGE page;
    short rrn = getpage();

    pageinit(&page);

    strncpy(page.key[0], key, 7);

    page.child[0] = left;
    page.child[1] = right;
    page.keycount = 1;
    page.byteofset[0] = byteofSet;

    btwrite(rrn, &page);

    putroot(rrn);
    return rrn;
}

short create_tree(char key[7], short byteofSet)
{
    short position = NIL;
    btfd = fopen("btree.bin", "wb+");
    fwrite(&position, sizeof(short), 1, btfd);
    fclose(btfd);
    btfd = fopen("btree.bin", "r+");

    return create_root(key, NIL, NIL, byteofSet);
}

void btread(short rrn, BTPAGE *page_ptr)
{
    long addr;
    addr = (long)rrn * (long)PAGESIZE + sizeof(short);

    fseek(btfd, addr, SEEK_SET);

    if (fread(&page_ptr->keycount, sizeof(short), 1, btfd) != 1)
    {
        perror("Erro ao ler keycount");
        return;
    }

    // Lendo as chaves (key) - assume-se que MAXKEYS <= 3
    for (int i = 0; i < 3; i++)
    {
        if (fread(page_ptr->key[i], sizeof(char), 7, btfd) != 7)
        {
            perror("Erro ao ler key[i]");
            return;
        }
    }

    // Lendo os ponteiros para os filhos
    if (fread(page_ptr->child, sizeof(short), MAXKEYS + 1, btfd) != (MAXKEYS + 1))
    {
        perror("Erro ao ler child");
        return;
    }

    // Lendo os offsets dos bytes
    if (fread(page_ptr->byteofset, sizeof(short), MAXKEYS, btfd) != MAXKEYS)
    {
        perror("Erro ao ler byteofset");
        return;
    }

    // printf("Essa foi a página lida antes da inserção\n");

    // printBTPAGE(page_ptr);
}

int search_node(char key[7], BTPAGE *p_page, short *pos)
{
    int i;
    for (i = 0; i < p_page->keycount && strcmp(key, p_page->key[i]) > 0; i++)
        ;
    *pos = i;
    if (*pos < p_page->keycount && strcmp(key, p_page->key[*pos]) == 0)
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

void ins_in_page(char key[7], short r_child, short byteofSet, BTPAGE *p_page)
{
    int j;
    for (j = p_page->keycount; j > 0 && strcmp(key, p_page->key[j - 1]) < 0; j--)
    {
        strcpy(p_page->key[j], p_page->key[j - 1]);
        p_page->child[j + 1] = p_page->child[j];
        p_page->byteofset[j] = p_page->byteofset[j - 1];
    }
    strcpy(p_page->key[j], key);
    p_page->child[j + 1] = r_child;
    p_page->byteofset[j] = byteofSet;
    p_page->keycount++;
}

void split(char key[7], short r_child, short byteofSet, BTPAGE *p_oldpage, char promo_key[7], short *promo_r_child, short *promo_byteofSet, BTPAGE *p_newpage)
{
    int j;
    char workkeys[MAXKEYS + 1][7];
    short workchil[MAXKEYS + 2];
    short workoffsets[MAXKEYS + 1];
    int mid = 2;

    for (j = 0; j < MAXKEYS; j++)
    {
        strcpy(workkeys[j], p_oldpage->key[j]);
        workchil[j] = p_oldpage->child[j];
        workoffsets[j] = p_oldpage->byteofset[j];
    }
    workchil[j] = p_oldpage->child[j];

    for (j = MAXKEYS; j > 0 && strcmp(key, workkeys[j - 1]) < 0; j--)
    {
        strcpy(workkeys[j], workkeys[j - 1]);
        workchil[j + 1] = workchil[j];
        workoffsets[j] = workoffsets[j - 1];
    }
    strcpy(workkeys[j], key);
    workchil[j + 1] = r_child;
    workoffsets[j] = byteofSet;

    strcpy(promo_key, workkeys[mid]);
    *promo_byteofSet = workoffsets[mid];
    *promo_r_child = getpage();

    printf("Aviso: Chave promovida %s\n", promo_key);

    strcpy(p_oldpage->key[mid], "@@@@@@");

    pageinit(p_newpage);

    for (j = 0; j < mid; j++)
    {
        strcpy(p_oldpage->key[j], workkeys[j]);
        p_oldpage->child[j] = workchil[j];
        p_oldpage->byteofset[j] = workoffsets[j];
    }
    p_oldpage->child[j] = workchil[j];
    p_oldpage->keycount = mid;

    for (j = mid + 1; j <= MAXKEYS; j++)
    {
        strcpy(p_newpage->key[j - mid - 1], workkeys[j]);
        p_newpage->child[j - mid - 1] = workchil[j];
        p_newpage->byteofset[j - mid - 1] = workoffsets[j];
    }
    p_newpage->child[j - mid - 1] = workchil[j];
    p_newpage->keycount = MAXKEYS - mid;
}

int insert(short rrn, char key[7], short *promo_r_child, char promo_key[7], short byteofSet, short *promo_byteofSet)
{
    BTPAGE page, newpage;
    int found, promoted;
    short pos, p_b_rrn;
    char p_b_key[7];
    short p_b_offset;

    if (rrn == NIL)
    {
        strcpy(promo_key, key);
        *promo_r_child = NIL;
        *promo_byteofSet = byteofSet;
        return YES;
    }

    btread(rrn, &page);
    found = search_node(key, &page, &pos);

    if (found)
    {
        printf("Erro: chave duplicada %s\n", key);
        return DUPLICADA;
    }

    promoted = insert(page.child[pos], key, &p_b_rrn, p_b_key, byteofSet, &p_b_offset);

    if (promoted == NO)
        return NO;

    if (page.keycount < MAXKEYS)
    {
        ins_in_page(p_b_key, p_b_rrn, p_b_offset, &page);
        btwrite(rrn, &page);

        return NO;
    }
    else if (promoted != DUPLICADA)
    {
        printf("Aviso: Houve uma divisão de nó\n");
        split(p_b_key, p_b_rrn, p_b_offset, &page, promo_key, promo_r_child, promo_byteofSet, &newpage);

        btwrite(rrn, &page);
        btwrite(*promo_r_child, &newpage);

        return YES;
    }

    return promoted;
}

typedef struct Register
{
    char id_aluno[4];
    char sigla_disc[4];
    char nome_aluno[50];
    char nome_disc[50];
    float media;
    float freq;

} Register;

typedef struct busca
{
    char id_aluno[4];
    char sigla_disc[4];
} Busca;

int search_tree(short rrn, char key[7], short *pos, short *found_rrn)
{
    BTPAGE page;
    int found;

    if (rrn == NIL)
    {
        return NO;
    }

    btread(rrn, &page);

    found = search_node(key, &page, pos);

    if (found)
    {
        *found_rrn = rrn;
        return YES;
    }
    else
    {
        return search_tree(page.child[*pos], key, pos, found_rrn);
    }
}

void realizaBusca(Busca *busca, short root)
{

    FILE *log_file = iniciaLogFile();

    int ultima_chave_buscada;

    fseek(log_file, 4, SEEK_SET);
    fread(&ultima_chave_buscada, sizeof(int), 1, log_file);

    ultima_chave_buscada++;

    char key[7];

    snprintf(key, sizeof(key), "%s%s", busca[ultima_chave_buscada].id_aluno, busca[ultima_chave_buscada].sigla_disc);

    short pos = 0, found_rrn = 0;
    int res = search_tree(root, key, &pos, &found_rrn);

    if (res == YES)
    {
        printf("Chave %s encontrada, página %hd, posição %hd\n", key, found_rrn, pos);
    }
    else
    {
        printf("Chave %s não encontrada\n", key);
    }

    fseek(log_file, 4, SEEK_SET);
    fwrite(&ultima_chave_buscada, sizeof(int), 1, log_file);

    fclose(log_file);
}

void print_rrn_data(short byteofset)
{
    FILE *data_file = iniciaArquivo();

    fseek(data_file, byteofset, SEEK_SET);

    int tamanho_registro = 0;
    char delimitador = '#';

    fread(&tamanho_registro, sizeof(int), 1, data_file);

    char buffer[tamanho_registro + 1];
    fread(buffer, sizeof(char), tamanho_registro - 9, data_file);

    Register registro;

    char *token = strtok(buffer, &delimitador);
    if (token != NULL)
        strncpy(registro.id_aluno, token, sizeof(registro.id_aluno) - 1);
    registro.id_aluno[3] = '\0';

    token = strtok(NULL, &delimitador);
    if (token != NULL)
        strncpy(registro.sigla_disc, token, sizeof(registro.sigla_disc) - 1);
    registro.sigla_disc[3] = '\0';

    token = strtok(NULL, &delimitador);
    if (token != NULL)
        strncpy(registro.nome_aluno, token, sizeof(registro.nome_aluno) - 1);

    token = strtok(NULL, &delimitador);
    if (token != NULL)
        strncpy(registro.nome_disc, token, sizeof(registro.nome_disc) - 1);

    fread(&registro.media, sizeof(float), 1, data_file);
    fseek(data_file, 1, SEEK_CUR);
    fread(&registro.freq, sizeof(float), 1, data_file);

    printf("\nID Aluno: %s\n", registro.id_aluno);
    printf("Sigla Disciplina: %s\n", registro.sigla_disc);
    printf("Nome do Aluno: %s\n", registro.nome_aluno);
    printf("Nome da Disciplina: %s\n", registro.nome_disc);
    printf("Média: %.2f\n", registro.media);
    printf("Frequência: %.2f\n", registro.freq);

    fclose(data_file);
}

void inorder(short rrn)
{
    if (rrn == -1)
    {
        return;
    }

    BTPAGE page;
    btread(rrn, &page);

    for (short i = 0; i < page.keycount; i++)
    {
        inorder(page.child[i]);

        if (page.byteofset[i] != -1)
        {
            print_rrn_data(page.byteofset[i]);
        }

        if (i == page.keycount - 1)
        {
            if (page.child[i + 1] != -1)
            {
                inorder(page.child[i + 1]);
            }
        }
    }
}

Busca *read_busca_file()
{
    FILE *arquivo = fopen("busca.bin", "r+b");

    Busca *busca = (Busca *)malloc(100 * sizeof(Busca));

    if (busca != NULL)
    {
        fread(busca, sizeof(struct busca), 100, arquivo);

        fclose(arquivo);
        return busca;
    }

    printf("Falha na alocação de memória para o registro de inserção\n");

    fclose(arquivo);
    return NULL;
}

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
    int i = 0, tam_reg = 0;

    bool operation_flag, flag = true;
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

int main()
{
    int promoted;
    short root;
    short promo_rrn;
    char promo_key[7];
    char key[7];
    short byteofSet;
    short promobyteofSet;

    Register *registros = read_insert_file();
    Busca *busca = read_busca_file();

    int answr;
    bool flag = true;

    while (flag)
    {
        printf("--------------MENU--------------------\n");
        printf("O que deseja fazer: \n(1)Inserção\n(2)Remoção dos Arquivos\n(3)Busca em Ordem\n(4)Busca Especifíca\nR: ");
        scanf("%d", &answr);

        if (answr == 1)
        {
            FILE *log_file = iniciaLogFile();
            FILE *data_file = iniciaArquivo();

            if (btopen())
            {
                int i;

                fread(&i, sizeof(int), 1, log_file);
                i++;

                fseek(data_file, 0, SEEK_END);
                byteofSet = ftell(data_file);

                snprintf(key, sizeof(key), "%s%s", registros[i].id_aluno, registros[i].sigla_disc);

                fclose(data_file);
                fclose(log_file);

                root = getroot();

                promoted = insert(root, key, &promo_rrn, promo_key, byteofSet, &promobyteofSet);

                if (promoted == YES)
                {
                    root = create_root(promo_key, root, promo_rrn, promobyteofSet);
                }

                if (promoted != DUPLICADA)
                {
                    insertRegister(registros);
                    printf("Aviso: Chave %s inserida com sucesso\n", key);
                }

                memset(key, '\0', sizeof(key));
                btclose();
            }
            else
            {
                fseek(data_file, 0, SEEK_END);
                byteofSet = ftell(data_file);

                snprintf(key, sizeof(key), "%s%s", registros[0].id_aluno, registros[0].sigla_disc);

                root = create_tree(key, byteofSet);

                fclose(data_file);
                fclose(log_file);

                insertRegister(registros);

                printf("Aviso: Chave %s inserida com sucesso\n", key);
                memset(key, '\0', sizeof(key));

                root = getroot();
            }
        }
        if (answr == 2)
        {
            remove("dados.bin");
            remove("log_file.bin");
            remove("btree.bin");

            printf("Aviso: Os arquivos foram removidos\n");
        }

        if (answr == 3)
        {
            btopen();
            short root = getroot();
            inorder(root);
            btclose();

            printf("Aviso: Busca em Ordem realizada com sucesso\n");
        }

        if (answr == 4)
        {
            btopen();
            short root = getroot();
            realizaBusca(busca, root);
            btclose();

            printf("Aviso: Busca Especifíca realizada com sucesso\n");
        }
    }

    free(registros);
    free(busca);
    return 0;
}
