#include <system.h>

/* Descritor de processos (BCP) ------------------------- */
typedef struct desc_p{
        char name[35];                               /* Nome do processo */
        enum{ativo, bloqueado, terminado} estado;    /* Estado do processo */
        PTR_DESC contexto;                           /* Ponteiro para descritor de contexto */
        struct desc_p *fila_sem;                     /* Fila de processos bloqueados por um semaforo */
        struct desc_p *prox_desc;                    /* Ponteiro para o proximo descritor */
} DESCRITOR_PROC;

/* Ponteiro para o Bloco de Controle de Processo */
typedef DESCRITOR_PROC *PTR_DESC_PROC;               /* Ponteiro para o descritor */


/*Tipo Semaforo -------------------------------------------- */
typedef struct{
        int s;                                       /* Parte inteira */
        PTR_DESC_PROC Q;                             /* Fila bloqueados */
}

/* FUNCOES BASICAS DO NUCLEO VISIVEIS AO USUARIO */

/* Fucao auxiliar para iniciar a Fila dos Processos PRONTOS para execucao como vazia */
void far iniciaFilaProcesso();

/* Criacao de um processo */
void far criaProcesso(void far (*p_address)(), char nome_p[16]);

/* Encerrar um processo */
void far terminaProcesso();

/* ativar o escalador */
void far ativaEscalador();

/* FUNCAO PARA UTILIZACAO DOS SEMAFOROS */

/* Incializar o Semaforo */
void far initiateSemaphore(semaforo *sem, int size_semaphore);

/* Primitiva P (down): Decrementar o valor do semaforo, se for zero, o processo eh colocado na Fila de Bloqueados */
void far P(semaforo *sem);

/* Primitiva V (up): Colocar o processo como ativo se o semaforo estuver nulo e tiver processo na fila, senao, decrementa o valor do semaforo */
void far V(semaforo *sem);