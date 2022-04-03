#ifndef INTERFACE_H
#define INTERFACE_H

//#include "estado.h"

//TODO: Defines com os n�meros correspondentes a cada tipo de pe�a

class interface
{
    public:

    AnimSist a;

    //Possui todas as pecas do jogo
    List<SElement*> elements;

    //Texture *tiles[2];
    //Texture *win;
    //Texture *lose;

    //Contrutor - Dever� carregar todos os recursos do jogo no momento da cria��o da interface
    interface();
    //Loop principal do jogo
    void gameLoop();
    //Desenha o tabuleiro com as pe�as
    void drawTab();
    //Permite selecionar uma pe�a e move-la atrav�s do mouse
    //ExPeca *mousePicking( int &jog_i, int &jog_j );

    void playerCpu(int player);
    //
    void playerJog(int player);

    bool victoryTest();

    void paintTiles(Lista_jogada *l, float color[]);

    void updateEstado();
};


#endif

