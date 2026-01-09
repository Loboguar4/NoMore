/*
 * ============================================================
 * ⏱️ NoMore — Gerenciador de Tempo em Linha de Comando
 *
 * Versão: 1.1
 * Autor: Bandeirinha
 *
 * 🔔 NOTA DE ATUALIZAÇÃO
 * ------------------------------------------------------------
 * • Atualização para acompanhar o calendário real.
 * • Exibição aprimorada de tempo (anos/meses quando aplicável).
 * • Melhorias na apresentação dos cronômetros.
 * • Manutenção da compatibilidade e preservação do funcionamento
 *   clássico da versão 1.0 — sem quebra de comportamento.
 * ------------------------------------------------------------
 *
 * Copyright (C) 2025-2026 Bandeirinha
 *
 * Este programa é software livre: você pode redistribuí-lo
 * e/ou modificá-lo sob os termos da Licença Pública Geral GNU
 * conforme publicada pela Free Software Foundation, na versão 3
 * da Licença, ou (a seu critério) qualquer versão posterior.
 *
 * Este programa é distribuído na esperança de que seja útil,
 * mas SEM QUALQUER GARANTIA; sem sequer a garantia implícita de
 * COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO PROPÓSITO.
 * Veja a Licença Pública Geral GNU para mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU
 * junto com este programa. Caso contrário, veja:
 * https://www.gnu.org/licenses/
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#define MAX_NAME 100
#define MAX_CRONOS 50
#define DATA_FILE "cronometros.dat"

/* ========= ESTRUTURA ========= */
typedef struct {
    char nome[MAX_NAME];
    time_t inicio;
    time_t alvo;
    time_t pausa;
    int reverso;
    int ativo;
    int pausado;
} Cronometro;

/* ========= ESTADO GLOBAL ========= */
Cronometro cronos[MAX_CRONOS];
int total = 0;
int executando = 1;
int modo_continuo = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* ========= INPUT ========= */
int ler_int(const char *msg) {
    char buf[32];
    int v;

    while (1) {
        printf("%s", msg);
        if (!fgets(buf, sizeof(buf), stdin))
            return -1;
        if (sscanf(buf, "%d", &v) == 1)
            return v;
        printf("Entrada inválida.\n");
    }
}

/* ========= PERSISTÊNCIA ========= */
void salvar_sem_lock() {
    FILE *f = fopen(DATA_FILE, "wb");
    if (!f) return;

    fwrite(&total, sizeof(int), 1, f);
    fwrite(cronos, sizeof(Cronometro), total, f);
    fclose(f);
}

void salvar() {
    pthread_mutex_lock(&mutex);
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);
}

void carregar() {
    pthread_mutex_lock(&mutex);

    FILE *f = fopen(DATA_FILE, "rb");
    if (f) {
        fread(&total, sizeof(int), 1, f);
        if (total > MAX_CRONOS) total = MAX_CRONOS;
        fread(cronos, sizeof(Cronometro), total, f);
        fclose(f);
    }

    pthread_mutex_unlock(&mutex);
}

/* ========= CONVERSÃO DE TEMPO ========= */
/* Conversão aproximada: 1 mês = 30 dias, 1 ano = 365 dias */
typedef struct {
    int anos;
    int meses;
    int dias;
    int horas;
    int minutos;
    int segundos;
} Duracao;

Duracao converter_tempo(time_t diff) {
    Duracao d;

    d.anos = diff / (365 * 86400);
    diff %= (365 * 86400);

    d.meses = diff / (30 * 86400);
    diff %= (30 * 86400);

    d.dias = diff / 86400;
    d.horas = (diff / 3600) % 24;
    d.minutos = (diff / 60) % 60;
    d.segundos = diff % 60;

    return d;
}

/* ========= EXIBIÇÃO ========= */
/* Agora exibe anos e meses e mostra dica de menu mesmo sem cronômetros */
void exibir_cronometros() {
    printf("=== CRONÔMETROS ===\n\n");

    pthread_mutex_lock(&mutex);

    if (total == 0) {
        printf("Nenhum cronômetro criado.\n");
        pthread_mutex_unlock(&mutex);

        printf("\nENTER = atualizar | m = menu | c = modo contínuo [%s]\n",
               modo_continuo ? "ON" : "OFF");
        return;
    }

    time_t agora = time(NULL);

    for (int i = 0; i < total; i++) {
        if (!cronos[i].ativo) continue;

        time_t diff;

        if (cronos[i].pausado)
            diff = cronos[i].pausa - cronos[i].inicio;
        else if (cronos[i].reverso)
            diff = cronos[i].alvo - agora;
        else
            diff = agora - cronos[i].inicio;

        if (diff <= 0 && cronos[i].reverso) {
            printf("%d) %s: FINALIZADO!\n", i + 1, cronos[i].nome);
            cronos[i].ativo = 0;
            salvar_sem_lock();
            continue;
        }

        Duracao d = converter_tempo(diff);

        printf(
            "%d) %s [%s]: %d anos, %d meses, %d dias e %02d:%02d:%02d hrs\n",
            i + 1,
            cronos[i].nome,
            cronos[i].pausado ? "Pausado" :
            (cronos[i].reverso ? "Regressivo" : "Crescente"),
               d.anos, d.meses, d.dias,
               d.horas, d.minutos, d.segundos
        );
    }

    pthread_mutex_unlock(&mutex);

    printf("\nENTER = atualizar | m = menu | c = modo contínuo [%s]\n",
           modo_continuo ? "ON" : "OFF");
}

/* ========= OPERAÇÕES ========= */
void criar_cronometro() {
    pthread_mutex_lock(&mutex);
    if (total >= MAX_CRONOS) {
        pthread_mutex_unlock(&mutex);
        printf("Limite atingido.\n");
        sleep(1);
        return;
    }
    pthread_mutex_unlock(&mutex);

    Cronometro c = {0};

    printf("Nome: ");
    fgets(c.nome, MAX_NAME, stdin);
    c.nome[strcspn(c.nome, "\n")] = 0;

    c.reverso = ler_int("Tipo (0 crescente | 1 regressivo): ");

    if (c.reverso) {
        struct tm tm = {0};
        printf("Data alvo (AAAA MM DD HH MM SS): ");
        scanf("%d %d %d %d %d %d",
              &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
              &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
        getchar();
        tm.tm_year -= 1900;
        tm.tm_mon  -= 1;
        c.alvo = mktime(&tm);
    }

    c.inicio = time(NULL);
    c.ativo = 1;

    pthread_mutex_lock(&mutex);
    cronos[total++] = c;
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);

    printf("Criado com sucesso!\n");
    sleep(1);
}

void pausar_cronometro() {
    int idx = ler_int("Número: ") - 1;

    pthread_mutex_lock(&mutex);
    if (idx < 0 || idx >= total || cronos[idx].pausado) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    cronos[idx].pausa = time(NULL);
    cronos[idx].pausado = 1;
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);
}

void retomar_cronometro() {
    int idx = ler_int("Número: ") - 1;

    pthread_mutex_lock(&mutex);
    if (idx < 0 || idx >= total || !cronos[idx].pausado) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    cronos[idx].inicio += time(NULL) - cronos[idx].pausa;
    cronos[idx].pausado = 0;
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);
}

void remover_cronometro() {
    int idx = ler_int("Número: ") - 1;

    pthread_mutex_lock(&mutex);
    if (idx < 0 || idx >= total) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    for (int i = idx; i < total - 1; i++)
        cronos[i] = cronos[i + 1];

    total--;
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);
}

void limpar_todos_cronometros() {
    int conf = ler_int(
        "Tem certeza que deseja remover TODOS os cronômetros? (1 = sim, 0 = não): "
    );

    if (conf != 1) {
        printf("Operação cancelada.\n");
        sleep(1);
        return;
    }

    pthread_mutex_lock(&mutex);
    memset(cronos, 0, sizeof(cronos));
    total = 0;
    salvar_sem_lock();
    pthread_mutex_unlock(&mutex);

    printf("Todos os cronômetros foram removidos.\n");
    sleep(1);
}

/* ========= MENU ========= */
void menu() {
    while (executando) {
        system("clear");
        printf("=== MENU ===\n");
        printf("1 Novo\n2 Pausar\n3 Retomar\n4 Remover\n5 Limpar TODOS\n6 Voltar\n7 Sair\n");

        int op = ler_int("> ");

        switch (op) {
            case 1: criar_cronometro(); break;
            case 2: pausar_cronometro(); break;
            case 3: retomar_cronometro(); break;
            case 4: remover_cronometro(); break;
            case 5: limpar_todos_cronometros(); break;
            case 6: return;
            case 7: executando = 0; salvar(); return;
        }
    }
}

/* ========= MAIN ========= */
int main() {
    carregar();

    while (executando) {
        system("clear");
        exibir_cronometros();

        if (modo_continuo) {
            fd_set fds;
            struct timeval tv = {1, 0};

            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
                char tmp[8];
                fgets(tmp, sizeof(tmp), stdin);
                modo_continuo = 0;
            }
            continue;
        }

        char cmd[8];
        fgets(cmd, sizeof(cmd), stdin);

        if (cmd[0] == 'm') menu();
        else if (cmd[0] == 'c') modo_continuo = !modo_continuo;
    }

    return 0;
}
