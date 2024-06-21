/* SISTEMAS OPERACIONAIS II - NUCLEO MULTIPROGRAMADO =============================================================
    Autores:
        - Manuele S. Christofalo | 221026291
        - Dante Ramacciotti | 221023259
        - Paulo Henrique Dionysio | 221026169
=================================================================================================================*/


// I - VARIAVEIS GLOBAIS E DEFINICOES ============================================================================
#include <system.h>

// Variaveis globais ---------------------------------------------------------------
PTR_DESC_PROC prim;         //Cabeca de fila para processos prontos
PTR_DESC d_esc;             //Ponteiro para o descritor do escalonador


// Registradores da regiao critica -------------------------------------------------
typedef struct registros{
    unsigned bx1, es1;      //bx = registrador base; es = registrador de segmento extra
}regis;

// Union dos registradores da regiao critica
typedef union k{
    regis x;                //Registradores bx + es
    char far *y;            //Valor da flag de servicos
}APONTA_REG_CRIT;
APONTA_REG_CRIT a;


// Descritor de processos (BCP) ----------------------------------------------------
typedef struct desc_p{
    char nome[35];                                  //Nome do processo
    enum{ativo, bloqueado, terminado} estado;       //Estado do processo
    PTR_DESC contexto;                              //Ponteiro para descritor de contexto
    struct desc_p *fila_sem;                        //Fila de processos bloqueados por um semaforo
    struct desc_p *prox_desc;                       //Ponteiro para o proximo descritor
} DESCRITOR_PROC;
typedef DESCRITOR_PROC *PTR_DESC_PROC;              //Ponteiro para o descritor


// Tipo semaforo -------------------------------------------------------------------
typedef struct{
    int s;                  //Parte inteira 
    PTR_DESC_PROC Q;        //Fila bloqueados
} semaforo;



// II - FUNCOES BASICAS DO NUCLEO ================================================================================

// 1. Cricao do processo -----------------------------------------------------------
void far cria_processo(void far (*p_address)(), char nome_p[16]){
    // Definicoes base
    PTR_DESC_PROC descritor = (PTR_DESC_PROC)malloc(sizeof(struct desc_p));     //Criacao dinamica do descritor
    strcpy(descritor->nome, nome_p);                                            //Copia nome
    descritor->estado = ativo;                                                  //Marca o estado como "ativo"
    descritor->contexto = cria_desc();                                          //Cria descritor de contexto 
    newprocess(p_address, descritor->contexto);                                 //Inicia descritor de contexto
    descritor->fila_sem = NULL;                                                 //Nenhum processo bloqueado

    // Insercao na fila de processos prontos
    descritor->prox_desc = NULL;
    
    if(prim == NULL){ // -> Fila vazia
        descritor->prox_desc = descritor;           //Como a fila eh circular, aponta para ele mesmo
        prim = descritor;                           //Ele eh o cabeca de fila
    }
    
    else{ // -> Fila povoada
        PTR_PCB aux = prim;                                     //Criacao de um auxiliar para percorrer a fila
        while(aux->prox_desc != prim) aux = aux->prox_desc;     //Enquanto a fila nao acabar, percorre a fila
    
        aux->prox_desc = descritor;                             //Auxiliar aponta para o processo
        descritor->prox_desc = prim;                            //Fecha a LCSE apontando para o cabeca de fila
    }
}


// 2. Procurar o proximo processo ativo --------------------------------------------
PTR_DESC_PROC Procura_proc_ativo(){
    PTR_DESC_PROC aux = prim->prox_desc;               //Criacao de um auxiliar para percorrer a fila de processos

    while(aux != prim){ // -> ate a lista acabar (como eh uma LCSE, isso ocorre quando aux volta a ser prim)
        if(aux->estado == ativo) return aux;           //Achou um processoa ativo -> retorna-o
        aux = aux->prox_desc;                          //Nao achou -> analisa o proximo processo
    }

    return NULL;                                      //Nenhum processo ativo encontrado
}


// 3. Termina processo -------------------------------------------------------------
void far termina_processo(){
    disable();                                      //Disabilita as interrupcoes
    prim->estado = terminado;                       //Marca processo corrente como terminado
    enable();                                       //Habilita as interrupcoes
    while(1);                                       //Laco eterno
}


// 4. Volta ao DOS -----------------------------------------------------------------
void far volta_DOS(){
    disable();                                      //Desabilita as interrupcoes
    setvect(8, p_est->int_anterior);                //Retorna o estado do bit de interrupcao para o padrao
    enable();                                       //Habilita as interrupcoes
    exit(0);                                        //Retorna ao DOS
}


// 5. Escalador de Processos -----------------------------------------------------
void far escalador(){
    p_est->p_origem  = d_esc;                       //Origem = escalonador 
    p_est->p_destino = prim->contexto;              //Destino = prim->contexto
    p_est->num_vetor = 8;                           //Bit de interrupcao do timer

    // Inicia ponteiro para regiao critica do DOS
    _AH=0x34;
    _AL=0x00; 
    geninterrupt(0x21);                             //Gera uma interrupcao
    a.x.bx1 = _BX;                                  //Bits menos significativos da RC
    a.x.es1 = _ES;                                  //Bits mais significativos

    // Troca de processos
    while(1){
        iotransfer();                               //Interrupcao de controle
        disable();                                  //Desabilita as interrupcoes

        // Se nao esta na regiao critica, troca os processos
        if(!(*a.y)){
            if((prim = Procura_proc_ativo()) == NULL){ // -> Proximo processo nao existe
                volta_DOS(); //Volta ao DOS
            }

            // -> Proximo processo existe  
            p_est->p_destino = prim->contexto;      //Troca os processos
        }

        enable();                                   //Habilita as interrupcoes     
    }
}


// 6. Tranferencia do Controle ao Escalador ----------------------------------------
void far dispara_sistema(){
    PTR_DESC desc_dispara;
    d_esc = cria_desc();
    desc_dispara = cria_desc()
    newprocess(scheduler, d_esc);
    transfer(desc_dispara, d_esc);
}



// III - SEMAFORO ================================================================================================

// 1. Definicao semaforo -----------------------------------------------------------
void far initiateSemaphore(semaforo *sem, int size_semaphore){
  sem->s = size_semaphore; /* Iniciando o valor com o tamanho dado pelo usuario */
  sem->Q = NULL;   /* Iniciando a fila de bloqueados com nulo */
}


// 2. Primitiva P (down) -----------------------------------------------------------
//      -> Decrementa o semaforo. Se o valor chegar em 0, o processo eh bloqueado
void far P(semaforo *sem){
    disable();                              //Desabilita as interrupcoes

    if(sem->s > 0){ // -> regiao critica em uso
        sem->s--;                           //Decrementa S
        enable();
    }

    else{
        PTR_DESC_PROC p_aux;                //Descritor auxiliar
        prim->estado = bloqueado;           //Bloqueia o processo na fila sem->Q
        
        if(sem->Q == NULL){ // -> Fila de bloqueados vazia
            sem->Q = prim;                  //Insere o processo como cabeca de fila
        }

        else{ // -> Fila povoada
            PTR_DESC_PROC aux;              //Auxiliar para percorrer a fila de bloqueados
            aux = sem->Q;

            while(aux->fila_sem != NULL) aux = aux->fila_sem;   //Percorre a fila ate seu fim

            aux->fila_sem = prim;           //Salva o processo na fila
        }

        prim->fila_sem = NULL;              //Semaforo do processo atual torna-se nulo
        

        // Procura o proximo processo pronto
        p_aux = prim;
        if((prim = Procura_proc_ativo()) == NULL){  // -> Nao encontrou mais processos ativos
            volta_DOS();                    //DEADLOCK -> processo atual se bloqueou e nao ha nenhum outro ativo
        }

        // Encontrou -> contexto passa para o novo processo ativo
        transfer(p_aux->contexto, prim->contexto);
    }

    enable(); //Habilita as interrupcoes
}


// 2. Primitiva V (up) -------------------------------------------------------------
//      -> Incrementa o semaforo. O coloca como ativo se nulo e tiver algum processo na fFila.
void far V(semaforo *sem){
        disable();  //Desabilita as interrupcoes

        if(sem->Q == NULL){ // -> Fila de bloquados nula
            sem->s++;  //Incrementa o semaforo
        }

        else{
            PTR_DESC_PROC p_prox;   //Processo uxiliar

            // Avanca para o proximo da fila de bloqueados
            p_prox = sem->Q;        
            sem->Q = p_prox->fila_sem;

            // Remove o processo da fila de bloqueados e o ativa
            p_prox->fila_sem = NULL;
            p_prox->estado = ativo;
        }

        enable();   //Habilita as interrupcoes
}