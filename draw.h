#ifndef DRAW_H
#define DRAW_H
#include <math.h>
/*Desenha objetos definidos diretamente em opengl*/

#define  PLANO 0
#define  BLOCO 1
#define  ESFERA 2
#define  CASA  3
#define  CARROCA 4
#define  ESTANDARTE 5
#define  TORRE 6
#define  ARVORE 7
namespace draw
{
    void normal(float p1[], float p2[], float p3[]);
    //Desenha um plano 30x30
    void plano(float scale[]);    //Desenha um cubo 30x30x30. Formato da textura utilizada 3x2 ( faces )
    void bloco(float scale[]);

    void esfera(float scale[]);
    void casa(float scale[]);

    void carroca(float scale[]);

    void estandarte(float scale[]);

    void torre(float scale[]);
    void arvore(float scale[]);

    void desenhar( int name, float scale[3] );

}

#endif // DRAW_H
