#include "mazeGenerator.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "MyList.h"

using namespace std;

void Labirinto::inicializar()
{
    matriz.criar_matriz(21,21);
    rows = 21;
    columns = 21;
}

void Labirinto::redimencionar(int i, int j)
{
    matriz.redimencionar(2*i+1,2*j+1);
    rows = 2*i+1;
    columns = 2*j+1;
}

void Labirinto::ronald_lab()
{
    int i,j;
    int floor=3; //Variavel que marca o caminho percorrido ja percorrido pelo auxiliar atraves do labirinto.
    int escolha[4];
    int cont=0;
    //Criação da pilhas usadas para guardar os caminhos percorridos pelo labirinto.
    List<Matriz::node*> *caminho_1 = new List<Matriz::node*>();
    List<Matriz::node*> *caminho_2 = new List<Matriz::node*>();

    Matriz::node *aux[6]; //Vetor de nós auxiliares.
    aux[5]=aux[3]=matriz.prim->baixo->direita;
    aux[6]=aux[4]=matriz.ult->cima->esquerda;
    aux[0]=matriz.prim;
    aux[1]=matriz.prim;

    //Cria a grade do labirinto
    for(i=0;i<matriz.x;i++)
    {
        for(j=0;j<matriz.y;j++)
        {
            if(i%2==0)
                aux[0]->valor=1;
            if(j%2==0)
                aux[0]->valor=1;
            aux[0]=aux[0]->direita;
        }
        aux[0]=aux[1]->baixo;
        aux[1]=aux[1]->baixo;
    }


    j=2;

    //Laço que define o percurso feito desde a entrada e desde a saida atravez do labirinto.
    while(aux[3]->cima->valor!=j&&aux[3]->cima->valor!=j&&aux[3]->cima->valor!=j&&aux[3]->cima->valor!=j)
    {

        //Laço para subir a pilha caso o caminho esteja trancado.
        do
        {

        aux[3]->valor=floor;

        //Condições de escolha da proxima celula a ser percorrida.
        if(aux[3]->cima->cima!=NULL&&aux[3]->cima->cima->valor!=floor)
            escolha[cont++]=1;
        if(aux[3]->direita->direita!=NULL&&aux[3]->direita->direita->valor!=floor)
            escolha[cont++]=2;
        if(aux[3]->esquerda->esquerda!=NULL&&aux[3]->esquerda->esquerda->valor!=floor)
            escolha[cont++]=3;
        if(aux[3]->baixo->baixo!=NULL&&aux[3]->baixo->baixo->valor!=floor)
            escolha[cont++]=4;

        aux[5]=aux[3];

        if(cont==0&&floor==3)
            aux[3]=caminho_1->remove(0);
        if(cont==0&&floor==2)
            aux[3]=caminho_2->remove(0);

        }while(cont==0); //se o numero de escolhas possiveis for zero percorre a pilha ate encontrar uma celula que possa ser percorrida.

        //printf("size = %d\n", caminho_2.size);

        if(floor==3)
            caminho_1->add(aux[3]); //Empilha o caminho para o percurso a partir da entrada.
        else
            caminho_2->add(aux[3]); //Empilha o percurso a partir da saida.

        cont=rand()%cont; //Joga aleatoriamente entre as escolhas possiveis para a proxima celula.
        switch(escolha[cont])
        {
            case 1:
            aux[3]->cima->valor=floor;
            aux[3]=aux[3]->cima->cima;
            break;
            case 2:
            aux[3]->direita->valor=floor;
            aux[3]=aux[3]->direita->direita;
            break;
            case 3:
            aux[3]->esquerda->valor=floor;
            aux[3]=aux[3]->esquerda->esquerda;
            break;
            case 4:
            aux[3]->baixo->valor=floor;
            aux[3]=aux[3]->baixo->baixo;
            break;
        }

        cont=0;
        //Alterna o floor entre o caminho desde a entrada e o caminho desde a saida.
        aux[0]=aux[3];
        aux[3]=aux[4];
        aux[4]=aux[0];
        aux[1]=aux[5];
        aux[5]=aux[6];
        aux[6]=aux[1];
        i=floor;
        floor=j;
        j=i;
    }

    //Percorre toda a matriz abrindo a celulas trancadas aplicando o percurso em cada uma até que todo o labirinto esteja conectado.
    floor=4;
    aux[1]=aux[0]=matriz.prim->baixo->direita;
    for(i=1;i<matriz.x;i=i+2)
    {
        for(j=1;j<matriz.y;j=j+2)
        {

            aux[5]=aux[3]=aux[0];
            caminho_1->free();

            while(aux[3]->valor==floor||aux[3]->valor==0)
            {

                do
                {
                aux[3]->valor=floor;
                if(aux[3]->cima->cima!=NULL&&aux[3]->cima->valor==1&&aux[3]->cima->cima->valor!=floor)
                    escolha[cont++]=1;
                if(aux[3]->direita->direita!=NULL&&aux[3]->direita->valor==1&&aux[3]->direita->direita->valor!=floor)
                    escolha[cont++]=2;
                if(aux[3]->esquerda->esquerda!=NULL&&aux[3]->esquerda->valor==1&&aux[3]->esquerda->esquerda->valor!=floor)
                    escolha[cont++]=3;
                if(aux[3]->baixo->baixo!=NULL&&aux[3]->baixo->valor==1&&aux[3]->baixo->baixo->valor!=floor)
                    escolha[cont++]=4;

                aux[5]=aux[3];
                if(cont==0)
                    aux[3]=caminho_1->remove(0);

                }while(cont==0);

                caminho_1->add(aux[3]);
                cont=rand()%cont;
                switch(escolha[cont])
                {
                    case 1:
                    aux[3]->cima->valor=floor;
                    aux[3]=aux[3]->cima->cima;
                    break;
                    case 2:
                    aux[3]->direita->valor=floor;
                    aux[3]=aux[3]->direita->direita;
                    break;
                    case 3:
                    aux[3]->esquerda->valor=floor;
                    aux[3]=aux[3]->esquerda->esquerda;
                    break;
                    case 4:
                    aux[3]->baixo->valor=floor;
                    aux[3]=aux[3]->baixo->baixo;
                    break;
                }
                cont=0;

        }

        aux[0]=aux[0]->direita->direita;
        floor++;

    }
        aux[1]=aux[1]->baixo->baixo;
        aux[0]=aux[1];
    }

    //Substitue os caminhos criados no percurso por 0(zero);
    aux[0]=matriz.prim;
    aux[1]=matriz.prim;
    for(i=0;i<matriz.x;i++)
    {
        for(j=0;j<matriz.y;j++)
        {
            if(aux[0]->valor>1)
                aux[0]->valor=0;
            aux[0]=aux[0]->direita;
        }
        aux[0]=aux[1]->baixo;
        aux[1]=aux[1]->baixo;
    }
    matriz.prim->baixo->valor=0;
    matriz.ult->cima->valor=0;
}

void Labirinto::lab_mod()
{
   int i;
    int j;
    Matriz::node *aux, *aux2;
    aux2=aux=matriz.prim;

    for(i=0;i<matriz.x;i++)
    {
        aux2=aux;
        for(j=0;j<matriz.y;j++)
        {
            if(aux2->valor==1)
            {
                if(j==0)
                {
                    if(i==0)
                        aux2->valor=vert_dir;
                    else if(i==matriz.x-1)
                        aux2->valor=vert_cima_dir;
                    else if(aux2->direita->valor==1)
                        aux2->valor=T_dir;
                    else
                        aux2->valor=pared_vertical;
                }
                else if(j==matriz.y-1)
                {
                    if(i==0)
                        aux2->valor=vert_esq;
                    else if(i==matriz.x-1)
                        aux2->valor=vert_cima_esq;
                    else if(aux2->esquerda->valor!=caminho)
                        aux2->valor=T_esq;
                    else
                        aux2->valor=pared_vertical;
                }
                else if(i==0)
                {
                    if(aux2->baixo->valor==1)
                        aux2->valor=T_baixo;
                    else
                        aux2->valor=pared_horizontal;
                }
                else if(i==matriz.x-1)
                {
                    if(aux2->cima->valor!=caminho)
                        aux2->valor=T_cima;
                    else
                        aux2->valor=pared_horizontal;
                }
                else if(aux2->direita->valor==1)
                {
                    if(aux2->esquerda->valor!=caminho&&aux2->baixo->valor==1&&aux2->cima->valor!=caminho)
                        aux2->valor=cruzado;
                    else if(aux2->esquerda->valor!=caminho&&aux2->cima->valor!=caminho)
                        aux2->valor=T_cima;
                    else if(aux2->esquerda->valor!=caminho&&aux2->baixo->valor==1)
                        aux2->valor=T_baixo;
                    else if(aux2->cima->valor!=caminho&&aux2->baixo->valor==1)
                        aux2->valor=T_dir;
                    else if(aux2->esquerda->valor!=caminho)
                        aux2->valor=pared_horizontal;
                    else if(aux2->cima->valor!=caminho)
                        aux2->valor=vert_cima_dir;
                    else if(aux2->baixo->valor==1)
                        aux2->valor=vert_dir;
                }
                else if(aux2->esquerda->valor!=caminho)
                {
                    if(aux2->baixo->valor==1&&aux2->cima->valor!=caminho)
                        aux2->valor=T_esq;
                    else if(aux2->cima->valor!=caminho)
                        aux2->valor=vert_cima_esq;
                    else if(aux2->baixo->valor==1)
                        aux2->valor=vert_esq;
                }
                else
                    aux2->valor=pared_vertical;
            }
            else
            {
                    aux2->valor=caminho;
            }


            aux2=aux2->direita;
        }
        aux=aux->baixo;
    }

    matriz.prim->baixo->valor=porta;

}

void Labirinto::ronald_print ()
{
    int i,j;
    Matriz::node *aux, *aux2;
    aux=matriz.prim;
    system("clear");
    for(i=0;i<matriz.x;i++)
    {
        aux2=aux;
        for(j=0;j<matriz.y;j++)
        {
            if(aux2->valor==0)
                std::cout << " ";
            else if(i%2==0&&j%2==0)
                std::cout << "+";
            else if(i==0||i==matriz.x-1)
                std::cout << "-";
            else if(j==0||j==matriz.y-1)
                std::cout << "|";
            else if(aux2->valor==1&&(aux2->esquerda->valor==1||aux2->direita->valor==1))
                std::cout << "-";
            else if(aux2->valor==1&&(aux2->cima->valor==1||aux2->baixo->valor==1))
                std::cout << "|";
            else
                std::cout << ".";
            aux2=aux2->direita;
        }
        std::cout << "\n";
        aux=aux->baixo;
    }
}

void Labirinto::zerar_lab()
{
    int i,j;
    Matriz::node *aux,*aux2;
    aux2=aux=matriz.prim;
    for(i=0;i<matriz.x;i++)
    {
        aux=aux->baixo;
        for(j=0;j<matriz.y;j++)
        {
            aux2->valor=0;
            aux2=aux2->direita;

        }
        aux2=aux;
    }
}
