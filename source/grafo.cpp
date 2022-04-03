#include "grafo.h"

//Utilizado para criar um rastro o personagem.
void melhor_caminho(Grafo *G, int u, int k)
{
    Arvore arv;
    arv.raiz=NULL;

    int i;
    for(i=0;i<G->tam;i++)
    {
        G->V[i].cor=BRANCO;
        G->V[i].dist=INF;
        G->V[i].pai=-1;
    }
    G->V[u].cor=CINZA;
    G->V[u].dist=0;
    G->V[u].pai=u;

    int z,w;
    Arvore::incluir(&(arv.raiz),u);
    while(arv.raiz!=NULL)
    {
        z=Arvore::menor_valor(&arv.raiz);
        G->V[z].cor=PRETO;

        for(i=0;i<G->V[z].grau;i++)
        {
            w=G->V[z].adj[i].vizinho;
            if( (G->V[w].cor != PRETO) && (G->V[w].dist > G->V[z].dist + G->V[z].adj[i].peso) && (G->V[z].dist + G->V[z].adj[i].peso <= k) )
            {
                G->V[w].dist=G->V[z].dist + G->V[z].adj[i].peso;
                G->V[w].pai=z;
                if(G->V[w].cor==BRANCO)
                {
                    G->V[w].cor=CINZA;
                    Arvore::incluir(&(arv.raiz),w);
                }

            }

        }

    }

}

void conectar(Grafo *G, int vert1, int vert2, int peso)
{
    vertice *pai, *filho;
    pai=&G->V[vert1];
    pai->id=vert1;
    filho=&G->V[vert2];
    filho->id = vert2;
    pai->adj[pai->grau].vizinho=vert2;
    filho->adj[filho->grau].vizinho=vert1;
    pai->adj[pai->grau++].peso=peso;
    filho->adj[filho->grau++].peso=peso;
}

//Utilizado para criar o grafo do labirinto no game
int **Criar_grafo(Labirinto L, Grafo *G)
{
    Fila fila;
    fila.prim=NULL;
    fila.ult=NULL;
    fila.tam=0;

    int i,j;
    int peso;
    int vert=0;


    bool teste=true;
    Matriz::node *w, *z;
    vertice *pai, *filho;

    //Fechar a saida do labirinto
    L.matriz.ult->cima->valor=1;

    G->V=(vertice*)malloc(sizeof(vertice)*L.matriz.x*L.matriz.y);
    for(i=0;i<L.matriz.x*L.matriz.y;i++)
    {
            G->V[i].grau=0;
            G->V[i].id=-1;
    }

    L.matriz.prim->baixo->valor=-1;
    L.matriz.prim->baixo->direita->valor=-1;

    int **mtz;

    mtz = (int**)malloc(L.matriz.x*sizeof(int));
    for(i=0;i<L.matriz.y;i++)
        mtz[i] = (int*)malloc(L.matriz.y*sizeof(int));

    for(i=0;i<L.matriz.x;i++)
        for(j=0;j<L.matriz.y;j++)
            mtz[i][j]=-1;


    fila.incluir(L.matriz.prim->baixo->direita);

    while(fila.prim!=NULL)
    {
        z=fila.remove();
        z->valor=-1;

        if(z!=L.matriz.ult->cima)
        {

            if(z->cima->valor==caminho)
            {
                w=z->cima;
                teste=true;
                peso=1;
                /*while((w->cima->valor>0 && w->direita->valor<=0 && w->baixo->valor>0 && w->esquerda->valor<=0) || (w->cima->valor<=0 && w->direita->valor>0 && w->baixo->valor<=0 && w->esquerda->valor>0))
                {
                    peso++;
                    w=w->cima;
                    if(w->valor==-1)
                        teste=false;
                }*/
                //if(teste)
             	{
                    conectar(G,vert,vert+fila.tam+1,peso);
                    fila.incluir(w);
                }
            }
            if(z->direita->valor==caminho)
            {
                peso=1;
                w=z->direita;
                teste=true;
               /* while((w->cima->valor>0 && w->direita->valor<=0 && w->baixo->valor>0 && w->esquerda->valor<=0) || (w->cima->valor<=0 && w->direita->valor>0 && w->baixo->valor<=0 && w->esquerda->valor>0))
                {
                    peso++;
                    w=w->direita;
                    if(w->valor==-1)
                        teste=false;
                }*/
               // if(teste)
                {
                    conectar(G,vert,vert+fila.tam+1,peso);
                    fila.incluir(w);
                }
            }
            if(z->baixo->valor==caminho)
            {
                peso=1;
                w=z->baixo;
                teste=true;
                /*while((w->cima->valor>0 && w->direita->valor<=0 && w->baixo->valor>0 && w->esquerda->valor<=0) || (w->cima->valor<=0 && w->direita->valor>0 && w->baixo->valor<=0 && w->esquerda->valor>0))
                {
                    peso++;
                    w=w->baixo;
                    if(w->valor==-1)
                        teste=false;
                }*/
               // if(teste)
                {
                    conectar(G,vert,vert+fila.tam+1,peso);
                    fila.incluir(w);
                }
            }
            if(z->esquerda->valor==caminho)
            {
                peso=1;
                w=z->esquerda;
                teste=true;
                /*while((w->cima->valor>0 && w->direita->valor<=0 && w->baixo->valor>0 && w->esquerda->valor<=0) || (w->cima->valor<=0 && w->direita->valor>0 && w->baixo->valor<=0 && w->esquerda->valor>0))
                {
                    peso++;
                    w=w->esquerda;
                    if(w->valor==-1)
                        teste=false;
                }*/
               // if(teste)
                {
                    conectar(G,vert,vert+fila.tam+1,peso);
                    fila.incluir(w);
                }
            }

            //////////////////////////////////////////////////////
            //Se a celula do labirinto não tem essa configuração//
            //       __                                         //
            //       __   ou essa configuração | | , então      //
            //   pode ser colocado um vertice do grafo nela     //
            //////////////////////////////////////////////////////

            G->V[vert].i = z->i;
            G->V[vert].j = z->j;
            mtz[z->i][z->j]=vert++;


        }
    }
    G->V[vert].i = L.matriz.x-2;
    G->V[vert].j = L.matriz.y-1;
    mtz[L.matriz.x-2][L.matriz.y-1]=vert;

    G->tam=vert;
    L.ronald_print();
    return mtz;

}

void apagar_grafo(Grafo *G)
{
    free(G->V);
    G->V=NULL;
}

