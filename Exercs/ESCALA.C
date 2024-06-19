#include <system.h>
#include <stdio.h>

PTR_DESC dtic, dtac, desc, dmain;

void far tic(){
     while(1){
              printf("TIC-");
     }
}

void far tac(){
     while(1){
         printf("TAC. ");
     }
}

void far escalonador(){
     p_est -> p_origem = desc;
     p_est -> p_destino = dtic;
     p_est -> num_vetor = 8;

     while(1){
         iotransfer();
         disable();
         if(p_est->p_destino == dtic){
             p_est -> p_destino = dtac;
         }
         else{
             p_est -> p_destino = dtic;
         }
         enable();
     }
}

int main(){
     dtic = cria_desc();
     dtac = cria_desc();
     desc = cria_desc();
     dmain = cria_desc();

     newprocess(tic, dtic);
     newprocess(tac, dtac);
     newprocess(escalonador, desc);

     transfer(dmain, desc);
}