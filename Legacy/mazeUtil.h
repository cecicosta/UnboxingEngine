#ifndef MAZE_UTIL_H
#define MAZE_UTIL_H

#include <stdio.h>
#include <stdlib.h>

class Matriz
{
    public:

    class node
    {
        public:

        int i;
        int j;

        int valor; //campo que guarda o valor do elemento.

        //Campos que apontam para os vizinhos do nó na matriz.
        node *cima;
        node *baixo;
        node *direita;
        node *esquerda;

    };

    node *prim;
    node *ult;
    int x;
    int y;

    Matriz(): x(0),y(0),prim(NULL), ult(NULL) {};

    //Cria e retorna um novo objeto node.
    node *novo_no(int i, int j);
    //Cria a matriz com tamanho inicial.
    void criar_matriz(int tam_x, int tam_y);
    //Redimenciona a matriz.
    void redimencionar(int tam_x, int tam_y);
    //Libera a matriz.
    void free_matriz();
    //Armazena valor dada uma posição;
    void armazenar(int x, int y, int valor);
    //Obtem o valor de uma dada posição da matriz.
    node *obter_valor (int x, int y);


};
class Pilha
{
    public:

    //Classe de objetos a serem empilhados e encadeados;
    class elemento
    {
        public:
        Matriz::node *celula;
        elemento *prox;
        elemento *ant;

    };


    //Primeiro e ultimo elementos da pilha.
    elemento *prim;
    elemento *ult;
    //Tamanho da pilha.
    int tam;
    //Inicializa a pilha.
    void init();

    elemento *novo_elemento();

    void empilhar(Matriz::node *celula);
    void empilhar(int v);

    //Metodo para desempilhar elementos.
    Matriz::node *desempilhar();
};
class Fila
{
    public:

    //Classe de objetos a serem empilhados e encadeados;
    class elemento
    {
        public:
        Matriz::node *celula;
        elemento *prox;
        elemento *ant;

    };

    //Primeiro e ultimo elementos da pilha.
    elemento *prim;
    elemento *ult;
    //Tamanho da pilha.
    int tam;
    //Inicializa a pilha.
    void init();

    elemento *novo_elemento();

    void incluir(Matriz::node *celula);
    //Metodo para desempilhar elementos.
    Matriz::node *remove();

};
typedef struct arvore
{
        int val;
        int balanco;
        struct arvore *dir;
        struct arvore *esq;
}arvore;
class Arvore
{
    public:

    arvore *raiz;
    static int incluir(arvore **raiz,int val);
    static int menor_valor(arvore **raiz);
    static void printa_arv(arvore *arv );
    static int buscar(arvore *arc, int val);
    static void free_arv(arvore *arv);
};

#endif
