#include <pthread.h> // necessário para o uso de threads
#include <semaphore.h> // necessário para o uso de semáforos
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>

/**
    @author Éllen Neves
    GitHub: https://github.com/Eosn
*/


/*
bibliografia: 
    - https://blog.pantuza.com/artigos/o-jantar-dos-filosofos-problema-de-sincronizacao-em-sistemas-operacionais
    - https://www.delftstack.com/howto/c/mutex-in-c/
    - https://www.revista-programar.info/artigos/threads-semaforos-e-deadlocks-o-jantar-dos-filosofos/
    - 
*/

/*
não pode ter (regras da disciplina):
    - SLEEP, WARNING, ERROR, VARIÁVEL GLOBAL, BUSY-WAIT

 não pode ter (regras do trabalho):
    - STARVATION, DEADLOCK
*/

/* número de filósofos é o mesmo do número de garfos,
   são necessários dois garfos para cada filósofo comer.

   o programa parará quando um filosófo comer
   max_porcoes vezes.
*/

typedef struct tmesa Mesa;
typedef struct tfilosofo Filosofo;
typedef struct tgarfo Garfo;
void exista(Filosofo *f);
void come(Filosofo *f);
void pense(Filosofo *f);
void tentaComer(Filosofo *f);
Filosofo *criaFilosofo(Mesa *mesa, int id);
Mesa *criaMesa (int qtdParada, int qtdFilosofos);

typedef struct tmesa {
    int id, num_filosofos, num_garfosdisponiveis, max_porcoes;
    Garfo **garfos;
    Filosofo **filosofos;
    sem_t sem_filosofo, sem_garfo, sem_finalizador, sem_comidas;
} Mesa;

typedef struct tfilosofo {
    int id, qtd_comeu;
    char esta_comendo;
    Mesa *mesa;
} Filosofo;

typedef struct tgarfo {
    Filosofo *quem_segura;
    Mesa *mesa;
    sem_t sem_lock;
} Garfo;

// numero de caracteres para printar um int
int ceillog10(int i){
    int j = 0;
    while(i%10 || i/10){
        i/=10;
        j++;
    }
    return j == 0 ? 1 : j; // zero ocupa 1 caractere
}

void *thread_exista(void *f) {
    exista(f);
    return f;
}

void exista(Filosofo *f) {
    while (1) {
        pense(f);
        tentaComer(f);
    }
}

void pense(Filosofo *f) {
    // na morte da bezerra, quietinho
}

void come(Filosofo *f) { 
    sem_wait(&(f->mesa->sem_comidas));
    f->esta_comendo = 1;
    f->qtd_comeu++;
    sem_post(&(f->mesa->sem_comidas));

    Filosofo **companheiros = malloc(sizeof(Filosofo*)*f->mesa->num_filosofos);
    for (int i = 0; i < f->mesa->num_filosofos; i++)
        companheiros[i] = NULL;
    int tamanho = 0;
    for (int i = 0; i < f->mesa->num_filosofos; i++)
        if (f->mesa->filosofos[i]->esta_comendo && f->mesa->filosofos[i]->id != f->id)
            companheiros[tamanho++] = f->mesa->filosofos[i];
    if (tamanho > 0) {
        char *status = calloc(60+(ceillog10(f->mesa->num_filosofos)+1)*f->mesa->num_filosofos, sizeof(char));
        char *string_builder = status;
        string_builder += sprintf(string_builder, "Filosofo %d esta comendo ao mesmo tempo que os filosofos ", f->id);
        for (int i = 0; i < tamanho; i++)
            string_builder += sprintf(
                string_builder,
                "%d%c", 
                companheiros[i]->id, 
                (i+1 == tamanho) // if
                    ? '.' // true
                    : ',' // false
            );
        string_builder += sprintf(string_builder, "\n");
        printf("%s", status);
        free(status);
        fflush(stdout);
    }
    
    sem_wait(&(f->mesa->sem_comidas));
    f->esta_comendo = 0;
    if (f->qtd_comeu >= f->mesa->max_porcoes) {  // se foi o primeiro a chegar no objetivo
        sem_post(&(f->mesa->sem_finalizador)); // libera a thread principal
        return; // e também trava todo mundo no sem_wait
    }
    sem_post(&(f->mesa->sem_comidas));
    free(companheiros);
}

void tentaComer(Filosofo *f) {
    int esquerda = (f->id) % f->mesa->num_filosofos;
    int direita = (f->id + 1) % f->mesa->num_filosofos;

    if (sem_trywait(&(f->mesa->garfos[esquerda]->sem_lock)) == 0) {
        f->mesa->garfos[esquerda]->quem_segura = f;
        if (sem_trywait(&(f->mesa->garfos[direita]->sem_lock)) == 0) {
            f->mesa->garfos[direita]->quem_segura = f;
            come(f);
            f->mesa->garfos[direita]->quem_segura = NULL;
            sem_post(&(f->mesa->garfos[direita]->sem_lock));
        }
        f->mesa->garfos[esquerda]->quem_segura = NULL;
        sem_post(&(f->mesa->garfos[esquerda]->sem_lock));
    }

    return; // volta a pensar
}

Filosofo *criaFilosofo(Mesa *mesa, int id) {
    Filosofo *f = malloc(sizeof(Filosofo));
    Garfo *g = malloc(sizeof(Garfo));

    f->id = id;
    f->qtd_comeu = 0;
    f->mesa = mesa;

    g->quem_segura = NULL; // o garfo começa na mesa
    g->mesa = mesa;

    mesa->filosofos[id] = f;
    mesa->garfos[id] = g;

    sem_init(&(g->sem_lock), 0, 1);

    return f;
}

Mesa *criaMesa (int qtdParada, int qtdFilosofos) {
    Mesa *mesa = malloc(sizeof(Mesa));
    Filosofo **filosofos = malloc(sizeof(Filosofo*)*qtdFilosofos);
    Garfo **garfos = malloc(sizeof(Garfo*)*qtdFilosofos);

    mesa->num_filosofos = qtdFilosofos;
    mesa->num_garfosdisponiveis = qtdFilosofos;
    mesa->max_porcoes = qtdParada;
    mesa->filosofos = filosofos;
    mesa->garfos = garfos;

    sem_init(&(mesa->sem_comidas), 0, 1);
    sem_init(&(mesa->sem_finalizador), 0, 0);

    return mesa;
}

int main (int argc, char **argv) {
    int i;
    pthread_t *thread_filosofos;
    Mesa *mesa;
    int num_filosofos, num_porcoes;

    if (argc != 3) {
        printf("Use ./exe <número de filósofos> <número de porções de comida>\n");
        return 0;
    }

    num_filosofos = atoi(argv[1]); // atoi = array to int
    num_porcoes = atoi(argv[2]);

    mesa = criaMesa(num_porcoes, num_filosofos);

    thread_filosofos = malloc(sizeof(pthread_t) * num_filosofos);
    for (i = 0; i < num_filosofos; i++)
        criaFilosofo(mesa, i);

    for (i = 0; i < num_filosofos; i++)
        pthread_create(thread_filosofos+i, NULL, thread_exista, mesa->filosofos[i]);

    sem_wait(&(mesa->sem_finalizador));

    for (i = 0; i < num_filosofos; i++)
        pthread_cancel(thread_filosofos[i]);
    // for (i = 0; i < num_filosofos; i++)
    //     pthread_join(thread_filosofos[i], NULL);

    for (i = 0; i < num_filosofos; i++)
        printf("Filosofo %d comeu %d vezes.\n", mesa->filosofos[i]->id, mesa->filosofos[i]->qtd_comeu);

    return 0;
}
