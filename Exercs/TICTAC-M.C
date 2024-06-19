#include <system.h>
#include <stdio.h>

PTR_DESC d_tic, d_tac, d_main;
int count = 0;

void far tic(){
    while(count < 100){
        printf("TIC-");
        count++;
        transfer(d_tic, d_tac);
    }
}
void far tac(){
    while(1){
        printf("TAC. ");
        transfer(d_tac, d_tic);
    }
}

int main(){
    d_tic = cria_desc();
    d_tac = cria_desc();
    d_main = cria_desc();

    newprocess(tic, d_tic);
    newprocess(tac, d_tac);

    transfer(d_main, d_tic);
}