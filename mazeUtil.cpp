#include "mazeUtil.h"
#include <iostream>

using namespace std;

Matriz::node *Matriz::novo_no(int i, int j)
{
    node *novo = (node*)malloc(sizeof(node));
    novo->cima = NULL;
    novo->baixo = NULL;
    novo->esquerda = NULL;
    novo->direita = NULL;
    novo->valor=0;
    novo->i=i;
    novo->j=j;
    return novo;
}

void Matriz::criar_matriz(int tam_x, int tam_y)
{

    int i,j;
    node *aux;
    if((tam_x==0)||(tam_y==0))
	{
		std::cout << "Tamanho invalido!\n";
	}
	else
	{
	    //Percorre as linhas da matriz.
		for(i=0;i<tam_x;i++)
		{

			//Percorre as colunas da matriz.
			for(j=0;j<tam_y;j++)
			{
			    //Cria o novo no da matriz.
				node *novo = novo_no(i,j);

				//Se for o primeiro elemento da matriz, coloca na primeira posição.
				if((i==0)&&(j==0))
				{
					prim=novo;
					ult=novo;
					aux=prim;
				}
				//Adiciona os demais elementos da matriz.
				else
				{
					if(i==0)
					{
					novo->esquerda=ult;
					ult->direita=novo;
					ult=ult->direita;
					}
					else
					{
					prim->baixo=novo;
					novo->cima=prim;
					novo->esquerda=ult;
					ult->direita=novo;
					ult=ult->direita;
					prim=prim->direita;
					}
				}
			}
		}
	prim=aux;
	}
	//Percorre as laterais fechando as linhas.
	while(aux)
	{
	    aux->esquerda=NULL;
	    aux=aux->baixo;
	}
	aux=ult;
	while(aux)
	{
	    aux->direita=NULL;
	    aux=aux->cima;
	}

    //Atualiza o tamanho da matriz.
    x=tam_x;
    y=tam_y;

}

void Matriz::redimencionar(int tam_x, int tam_y)
{
    node *novo;
    node *aux;
    node *aux2;
    node *aux3;
    int i,j;

    aux=prim;

    while(aux->baixo)
    {
        aux=aux->baixo;
    }
    if((tam_x-x)>0)
    {
        for(j=0;j<(tam_x-x);j++)
        {
            for(i=0;i<y;i++)
            {
                if(i==0)
                {
                    novo=novo_no(i,j);
                    novo->cima=aux;
                    aux->baixo=novo;
                    aux2=aux->baixo;
                    aux3=aux2;
                    aux=aux->direita;
                    ult=novo;
                }
                else
                {

                    novo=novo_no(i,j);
                    novo->cima=aux;
                    aux->baixo=novo;
                    novo->esquerda=aux2;
                    aux2->direita=novo;
                    aux=aux->direita;
                    aux2=aux2->direita;
                    ult=novo;
                }
            }
        aux=aux3;
        }
        x=tam_x;
    }


    aux=prim;
    while(aux->direita)
    {
        aux=aux->direita;
    }
    if(tam_y-y>0)
    {
        for(j=0;j<(tam_y-y);j++)
        {
            for(i=0;i<x;i++)
            {
                if(i==0)
                {

                    novo=novo_no(i,j);
                    novo->esquerda=aux;
                    aux->direita=novo;
                    aux2=aux->direita;
                    aux3=aux2;
                    aux=aux->baixo;
                    ult=novo;
                }
                else
                {

                    novo=novo_no(i,j);
                    novo->esquerda=aux;
                    aux->direita=novo;
                    novo->cima=aux2;
                    aux2->baixo=novo;
                    aux=aux->baixo;
                    aux2=aux2->baixo;
                    ult=novo;
                }
            }
             aux=aux3;
        }
            y=tam_y;
    }
}

void Matriz::free_matriz()
{
    int i,j;
    node *aux,*aux2,*aux3;
    aux2=aux=prim;
    for(i=0;i<x;i++)
    {
        aux=aux->baixo;
        for(j=0;j<x;j++)
        {
            aux3=aux2;
            aux2=aux2->direita;
            free(aux3);
        }
        aux2=aux;
    }
    prim=NULL;
    ult=NULL;
    x=0;
    y=0;
}

void Matriz::armazenar(int x, int y, int valor)
{
        int i;
        node *aux;
        aux=prim;
        for(i=0;i<x;i++)
        {
            aux=aux->direita;
        }
        for(i=0;i<y;i++)
        {
            aux=aux->baixo;
        }
        aux->valor=valor;
}

Matriz::node *Matriz::obter_valor (int x, int y)
{
    int i;
        node *aux;
        aux=prim;
        for(i=0;i<x;i++)
        {
            aux=aux->direita;
        }
        for(i=0;i<y;i++)
        {
            aux=aux->baixo;
        }
    return (aux);
}


void Pilha::init()
{
    prim=NULL;
    ult=NULL;
    tam=0;
}

Pilha::elemento *Pilha::novo_elemento()
{
    elemento *novo = (elemento*)malloc(sizeof(elemento));
    novo->celula = NULL;
    novo->prox = NULL;
    novo->ant = NULL;
    return novo;
}

void Pilha::empilhar(Matriz::node *celula)
{

    elemento *novo;
    novo = novo_elemento();
    novo->ant=ult;
    novo->celula=celula;//Guarda a celula percorrida do labirinto.
    tam++; //Atualiza o tamanho da pilha.

    //Se lista esta vazia então coloque o primeiro elemento.
    if(prim==NULL)
    {
        prim=novo;
        ult=novo;
    }
    //Senão adicione no ultimo elemento.
    else
    {
        ult->prox=novo;
        ult=ult->prox;
    }
}

Matriz::node *Pilha::desempilhar()
{
    Matriz::node *aux;
    elemento *FREE;
    if(ult==NULL)
        return NULL;
    //Remove, libera e rerorna os elementos.
    if(ult->ant==NULL)
    {
        aux=ult->celula;
        FREE=ult;
        ult=NULL;
        free(FREE); //Libera um elemento da pilha.
        return aux; //Retorna uma celula do labirinto.
    }
    else
    {
        aux=ult->celula;
        FREE=ult;
        ult=ult->ant;
        ult->prox=NULL;
        free(FREE);
        return aux;
    }

}


void Fila::init()
{
    prim=NULL;
    ult=NULL;
    tam=0;
}

Fila::elemento *Fila::novo_elemento()
{
    elemento *novo = (elemento*)malloc(sizeof(elemento));
    novo->celula = NULL;
    novo->prox = NULL;
    novo->ant = NULL;
    return novo;
}

void Fila::incluir(Matriz::node *celula)
{

    elemento *novo;
    novo = novo_elemento();
    novo->ant=ult;
    novo->celula=celula;//Guarda a celula percorrida do labirinto.
    tam++; //Atualiza o tamanho da pilha.

    //Se lista esta vazia então coloque o primeiro elemento.
    if(prim==NULL)
    {
        prim=novo;
        ult=novo;
    }
    //Senão adicione no ultimo elemento.
    else
    {
        ult->prox=novo;
        ult=ult->prox;
    }
}

Matriz::node *Fila::remove()
{
    if(prim==NULL)
        return NULL;

    Matriz::node *elmt;
    elemento *aux;
    aux=prim;
    elmt=aux->celula;
    prim=prim->prox;
    free(aux);
    tam--;
    return elmt;

}


void rot_simples_dir(arvore** u) //alterar referencia
{
    arvore *z,*w; //auxiliares

    z=*u;
    *u=(*u)->esq;
    w=(*u)->dir;
    (*u)->dir=z;;
    z->esq=w;
}

int Arvore::incluir(arvore **raiz, int val)
{

    int ret;
    if(*raiz==NULL)
    {
         arvore *novo;
         novo=(arvore*)malloc(sizeof(arvore));
         novo->val=val;
         novo->dir=NULL;
         novo->esq=NULL;
         *raiz=novo;
         return 1;
    }
    else
    {
         if((*raiz)->val==val)
                           return 0;
         if((*raiz)->val<val)
             ret=incluir(&((*raiz)->dir),val);
         else
             ret=incluir(&((*raiz)->esq),val);
    }
    return ret;
}

int Arvore::menor_valor(arvore **raiz)
{
    int menor=0;
    arvore *aux;
    if(*raiz==NULL)
        return -1;
    if((*raiz)->esq!=NULL)
    {
        menor = menor_valor(&(*raiz)->esq);
    }
    else
    {
        menor = (*raiz)->val;
        aux=*raiz;
        (*raiz)=aux->dir;
        free(aux);
        return menor;
    }

    return menor;
}

void Arvore::printa_arv(arvore *arv)
{
     if(arv!=NULL)
     {
        printa_arv(arv->esq);
        printf("%d\n",arv->val);
        printa_arv(arv->dir);
     }

 }

void  perc_pre_ord(arvore *arv)
{
	if(arv!=NULL)
	{
		printf("%d\n",arv->val);
		perc_pre_ord(arv->esq);
		perc_pre_ord(arv->dir);

	}
}


void perc_pos_ord(arvore *arv)
{
	if(arv!=NULL)
	{
		perc_pre_ord(arv->esq);
		perc_pre_ord(arv->dir);
		printf("%d\n",arv->val);
	}
}


int profundidade(arvore *arv, int nivel, int max)
{
    if(arv==NULL)
    {
        if(nivel>max)
            return nivel;
        else
            return max;
    }
    else
    {
        max=profundidade(arv->esq, nivel +1, max);
        max=profundidade(arv->dir, nivel +1, max);
    }
    return max;
}

void Arvore::free_arv(arvore *arv)
{
    arvore *aux1,*aux2;
	if(arv!=NULL)
	{
	    aux1=arv->esq;
	    aux2=arv->dir;
	    free(arv);
		free_arv(aux1);
		free_arv(aux2);
	}
}

int Arvore::buscar(arvore *arv, int val)
{
    int resp;
    if(arv==NULL)
        return 0;
    if(arv->val==val)
        return 1;
    else
    {
        if(arv->val>val)
            resp=buscar(arv->esq,val);
        else
            resp=buscar(arv->dir,val);
    }
    return resp;
}
