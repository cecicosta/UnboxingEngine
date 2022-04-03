#ifndef GRAFO_H
#define GRAFO_H

#define ORIGEM 0
#define DESTINO 1
#define BRANCO 0
#define CINZA 1
#define PRETO 2
#define INF 10000000

#include "mazeGenerator.h"
#include "mazeUtil.h"
#include "SceneElement.h"

typedef struct aresta
{
    int vizinho;
    int peso;
}aresta;

typedef struct vertice
{
    List<SceneElement*> elements;
    aresta adj[4];
    int cor;
    int pai;
    int dist;
    int grau;
    int id;
    int i,j;
}vertice;

typedef struct Grafo
{
    vertice *V;
    int tam;
}Grafo;

//Utilizado para criar um rastro o personagem.
void melhor_caminho(Grafo *G, int u, int k);

void conectar(Grafo *G, int vert1, int vert2, int peso);
//Utilizado para criar o grafo do labirinto no game
int **Criar_grafo(Labirinto L, Grafo *G);

void apagar_grafo(Grafo *G);

#endif
