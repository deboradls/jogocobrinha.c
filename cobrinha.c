#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#define COLUNAS 60
#define LINHAS 30

enum TipoComida {
  VERDE,
  AMARELO,
  VERMELHO
};

void definirVelocidadeJogo(int dificuldade, int *atraso) {
    switch (dificuldade) {
        case 1:  // Fácil
            *atraso = 150000;
            break;
        case 2:  // Médio
            *atraso = 100000;
            break;
        case 3:  // Difícil
            *atraso = 50000;
            break;
        default:
            *atraso = 100000;  // Valor padrão para médio
            break;
        }
}

struct EntradaRanking {
    char nome[50];
    int pontuacao;
};

// Função para comparar duas entradas de pontuação (necessária para qsort)
int compararPontuacoes(const void *a, const void *b) {
    return ((struct EntradaRanking *)b)->pontuacao - ((struct EntradaRanking *)a)->pontuacao;
}

// Função para salvar no ranking e obter a posição
int salvarNoRanking(char *nome, int pontuacao) {
    FILE *arquivo = fopen("ranking.txt", "a+");

    if (arquivo != NULL) {
        struct EntradaRanking entradas[100];
        int contagem = 0;

        // Ler entradas existentes do arquivo
        while (fscanf(arquivo, "%s %d", entradas[contagem].nome, &entradas[contagem].pontuacao) == 2) {
            contagem++;
        }

        // Adicionar a nova entrada
        strcpy(entradas[contagem].nome, nome);
        entradas[contagem].pontuacao = pontuacao;
        contagem++;

        // Ordenar as entradas por pontuação (maior primeiro)
        qsort(entradas, contagem, sizeof(struct EntradaRanking), compararPontuacoes);

        // Truncar o arquivo para limpar o conteúdo existente
        if (ftruncate(fileno(arquivo), 0) == 0) {
            // Atualizar o arquivo com as entradas ordenadas
            rewind(arquivo);
            for (int i = 0; i < contagem; i++) {
                fprintf(arquivo, "%s %d\n", entradas[i].nome, entradas[i].pontuacao);
            }
        } 
        else {
            printf("Erro ao truncar o arquivo.\n");
        }

        fclose(arquivo);

        // Encontrar a posição do jogador no ranking
        for (int i = 0; i < contagem; i++) {
            if (strcmp(entradas[i].nome, nome) == 0 && entradas[i].pontuacao == pontuacao) {
                return i + 1;  // Posição no ranking (1-indexed)
            }
        }
    } 
    else {
    printf("Erro ao abrir o arquivo de ranking.\n");
    }

    return 1;  
}

int main() {
    int dificuldade;
    int atraso;
    int pontuacao = 0;  // Pontuação inicial
    char nomeJogador[50];
    system("clear");

    // Escolha do nível de dificuldade
    printf("Escolha o nível de dificuldade:\n");
    printf("1 - Fácil\n");
    printf("2 - Médio\n");
    printf("3 - Difícil\n");
    scanf("%d", &dificuldade);
    system("clear");
    // Esconder cursor
    printf("\e[?25l");

    // Mudar para o modo canônico, desativar eco
    struct termios antigo, novo;
    tcgetattr(STDIN_FILENO, &antigo);
    novo = antigo;
    novo.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &novo);

    int x[1000], y[1000];
    int sair = 0;
    int novo_comprimentoCobra = 0;  // Comprimento inicial da cobra

    definirVelocidadeJogo(dificuldade, &atraso);
    while (!sair) {

        // Renderizar tabela
        printf("┌");
        for (int i = 0; i < COLUNAS; i++)
            printf("─");
        printf("┐\n");

        for (int j = 0; j < LINHAS; j++) {
            printf("│");
            for (int i = 0; i < COLUNAS; i++)
                printf("·");
            printf("│\n");
        }

        printf("└");
        for (int i = 0; i < COLUNAS; i++)
            printf("─");
        printf("┘\n");

        // Mover cursor de volta para o topo
        printf("\e[%iA", LINHAS + 2);

        int cabeca = 0, cauda = 0;
        x[cabeca] = COLUNAS / 2;
        y[cabeca] = LINHAS / 2;
        int fimDoJogo = 0;
        int xdir = 1, ydir = 0;
        int macaX = -1, macaY;
        enum TipoComida tipoComida;

        while (!sair && !fimDoJogo) {
            if (macaX < 0) {
                // Criar nova maçã com probabilidades ajustadas
                macaX = rand() % COLUNAS;
                macaY = rand() % LINHAS;
                int probabilidade = rand() % 100;

                if (probabilidade < 50)
                    tipoComida = VERDE;
                else if (probabilidade < 70)
                    tipoComida = AMARELO;
                else
                    tipoComida = VERMELHO;

                for (int i = cauda; i != cabeca; i = (i + 1) % 1000)
                    if (x[i] == macaX && y[i] == macaY)
                        macaX = -1;

                if (macaX >= 0) {
                    // Desenhar maçã com base no tipo
                    if (tipoComida == VERDE)
                        printf("\e[%iB\e[%iC\x1b[32m❤\x1b[0m", macaY + 1, macaX + 1);  // Verde
                    else if (tipoComida == AMARELO)
                        printf("\e[%iB\e[%iC\x1b[33m❤\x1b[0m", macaY + 1, macaX + 1);  // Amarelo
                    else
                        printf("\e[%iB\e[%iC\x1b[31m❤\x1b[0m", macaY + 1, macaX + 1);  // Vermelho

                    printf("\e[%iF", macaY + 1);
                }
            }

            // Limpar cauda da cobra
            printf("\e[%iB\e[%iC·", y[cauda] + 1, x[cauda] + 1);
            printf("\e[%iF", y[cauda] + 1);

            if (x[cabeca] == macaX && y[cabeca] == macaY) {
                macaX = -1;
                // Emitir um bip
                Beep(500, 900);

                // Aumentar o comprimento da cobra com base no tipo de comida
                if (tipoComida == VERDE) {
                    novo_comprimentoCobra = 1;
                    pontuacao += 1;
                } 
                else if (tipoComida == AMARELO) {
                    novo_comprimentoCobra = 3;
                    pontuacao += 3;
                } 
                else {
                    novo_comprimentoCobra = 0;
                    pontuacao += 2;
                }
            } 
            else {
                novo_comprimentoCobra = 0;
            }

            cauda = (cauda - novo_comprimentoCobra + 1) % 1000;

            int novaCabeca = (cabeca + 1) % 1000;
            x[novaCabeca] = (x[cabeca] + xdir + COLUNAS) % COLUNAS;
            y[novaCabeca] = (y[cabeca] + ydir + LINHAS) % LINHAS;
            cabeca = novaCabeca;

            // Verificar colisão com as bordas
            if (x[cabeca] <= 0 || x[cabeca] >= COLUNAS || y[cabeca] <= 0 || y[cabeca] >= LINHAS) {
                fimDoJogo = 1;
            }

            for (int i = cauda; i != cabeca; i = (i + 1) % 1000)
                if (x[i] == x[cabeca] && y[i] == y[cabeca])
                    fimDoJogo = 1;

            // Desenhar cabeça
            printf("\e[%iB\e[%iC▓", y[cabeca] + 1, x[cabeca] + 1);
            printf("\e[%iF", y[cabeca] + 1);
            fflush(stdout);

            usleep(atraso);
            // Ler teclado
            struct timeval tv;
            fd_set fds;
            tv.tv_sec = 0;
            tv.tv_usec = 0;

            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);
            select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                int ch = getchar();
                if (ch == 27 || ch == 'q') {
                    sair = 1;
                } 
                else if (ch == 'a' && xdir != 1) {
                    xdir = -1;
                    ydir = 0;
                } 
                else if (ch == 'd' && xdir != -1) {
                    xdir = 1;
                    ydir = 0;
                } 
                else if (ch == 's' && ydir != -1) {
                    xdir = 0;
                    ydir = 1;
                } 
                else if (ch == 'w' && ydir != 1) {
                    xdir = 0;
                    ydir = -1;
                }
            }

            // Imprimir pontuação atual
            printf("\e[%iB\e[%iC Pontuação: %d", LINHAS + 2, COLUNAS / 2 - 5, pontuacao);
            printf("\e[%iF", LINHAS + 2);
        }

        if (!sair) {
            // Mostrar fim do jogo
            printf("\e[%iB\e[%iC Fim do Jogo! ", LINHAS / 2, COLUNAS / 2 - 5);
            printf("\e[%iF", LINHAS / 2);
            fflush(stdout);

            printf("\e[?25h");
            tcsetattr(STDIN_FILENO, TCSANOW, &antigo);

            // Perguntar o nome do jogador
            system("clear");
            printf("\nDigite seu nome para salvar a pontuação: ");
            scanf("%s", nomeJogador);

            // Informar a posição no ranking e salvar
            printf("Você ficou em %dº lugar no ranking!\n", salvarNoRanking(nomeJogador, pontuacao));  // Atualizar com a posição real
            return 0;
        }
    }

  // Mostrar cursor
  printf("\e[?25h");
  tcsetattr(STDIN_FILENO, TCSANOW, &antigo);
  return 0;
}
