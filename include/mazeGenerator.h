#ifndef MAZE_GEN_H
#define MAZE_GEN_H

#define vert_dir 0
#define pared_horizontal 1
#define vert_esq 2
#define pared_vertical 3
#define vert_cima_dir 4
#define cruzado 5
#define vert_cima_esq 6

#define caminho 7
#define T_dir 8
#define T_esq 9
#define T_baixo 10
#define T_cima 11
#define porta 12

#include "mazeUtil.h"

class Labirinto
{
    public:
    int rows;
    int columns;

    Matriz matriz;

    void inicializar();

    void redimencionar(int i, int j);
    //Metodo de geração do labirirnto.
    void ronald_lab();
    //Especifica o tipo de cada parede do labirirnto (horizontal, vertical, vertice, etc).
    void lab_mod();
    //Imprime o labirirnto no console.
    void ronald_print();
    //Zera todas as celulas do labirirnto.
    void zerar_lab();

};

#endif
