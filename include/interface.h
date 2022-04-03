#ifndef INTERFACE_H
#define INTERFACE_H

//#include "estado.h"

//TODO: Defines com os números correspondentes a cada tipo de peça

class interface
{
    public:

    AnimSist a;

    //Possui todas as pecas do jogo
    List<SElement*> elements;

    //Texture *tiles[2];
    //Texture *win;
    //Texture *lose;

    //Contrutor - Deverá carregar todos os recursos do jogo no momento da criação da interface
    interface();
    //Loop principal do jogo
    void gameLoop();
    //Desenha o tabuleiro com as peças
    void drawTab();
    //Permite selecionar uma peça e move-la através do mouse
    //ExPeca *mousePicking( int &jog_i, int &jog_j );

    void playerCpu(int player);
    //
    void playerJog(int player);

    bool victoryTest();

    void paintTiles(Lista_jogada *l, float color[]);

    void updateEstado();
};


#endif

