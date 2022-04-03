#include "interface.h"
#include "GLAux.h"
#include <math.h>
#include "mathUtil.h"
#include <time.h>
//#include "AnimSist.h"
#include "resources.h"

#define TEX_DIM_X 128.0f
#define TEX_DIM_Y 128.0f
#define TEX_SPRIT_DIM_X 128.0f
#define TEX_SPRIT_DIM_Y 128.0f
#define SPRITS_NUM 8


interface::interface()
{

    /// seed para a funcao random
    srand(time(NULL));

    int id;
    //Resources::loadResource("Textures", TEXTURE, id);
    //a.setAnim(0);


    /// carregar recursos do jogo
    //tiles[TILE#] = GLAux::LoadTexture("texture");

    /// definir o estado inicial
    //Iniciar level????

    pecas.add( new SElement() );
}

void interface::drawTab()
{
    //Desenhar o tabuleiro
    for(int i=0; i<8; i++)
    for(int j=0; j<8; j++)
    {
        //TODO - desenhar o tile com sua respectiva textura e frame caso haja animação.
        Texture *tex;
        if( (i+j)%2 == 0 )
            tex = tiles[BRANCO];
        else
            tex = tiles[PRETO];

        //Calculo da posição de cada tile
        float pos[] = {TAB_X + j*TILE_DIM - TAB_DIM/2 + TILE_DIM/2, TAB_Y + i*TILE_DIM - TAB_DIM/2 + TILE_DIM/2, 0};
        float scale[] = {TILE_DIM, TILE_DIM, 1};

        GLAux::drawPlane( pos, scale, tex );
    }
}

bool interface::victoryTest()
{
    int p = E.utility();

    //Desenhar mensagem de vitoria ou derrota
    if( p == -INFINITO )
    {
        float pos[] = {0,0,0};
        float scale[] = {TAB_DIM, TAB_DIM};
        float color[] = {1,1,1,0.5};
        GLAux::drawPlane( pos, scale, color, win );
        return true;
    }
    else if( p == INFINITO )
    {
        float pos[] = {0,0,0};
        float scale[] = {TAB_DIM, TAB_DIM};
        float color[] = {1,1,1,0.5};
        GLAux::drawPlane( pos, scale, color, lose );
        return true;
    }
    return false;
}

void interface::playerJog( int player )
{
    int i =0;
    int j = 0;


    if( player == MORTE && turno_morte )
    {
        ExPeca *p = mousePicking( i, j );
        if( p != NULL && p->player == MORTE && mov->pertence(p->pos_i, p->pos_j, i, j) )
        {
            int ant_i = p->pos_i;
            int ant_j = p->pos_j;
            p->move(&E, i, j);
            turno_morte = !turno_morte;
        }
    }
    if( player == PESSOAS && !turno_morte )
    {
        ExPeca *p = mousePicking( i, j );
        if( p != NULL && p->player == PESSOAS && mov->pertence(p->pos_i, p->pos_j, i, j))
        {
            int ant_i = p->pos_i;
            int ant_j = p->pos_j;
            p->move(&E, i, j);
            turno_morte = !turno_morte;
        }
    }
}

void interface::playerCpu( int player )
{
    retorno_minmax aux;

    if( player == PESSOAS && !turno_morte || player == MORTE && turno_morte )
    {
        if( player == PESSOAS )
            aux = E.MinMax_alfa_beta(opcoes[REC_PESSOAS], opcoes[REC_PESSOAS], -INFINITO_MAIOR, INFINITO_MAIOR);
        else
            aux = E.MinMax_alfa_beta(opcoes[REC_MORTE], opcoes[REC_MORTE], -INFINITO_MAIOR, INFINITO_MAIOR);

        ExPeca *p = ExPeca::getPecaByPosition( pecas, aux.jog.de.i, aux.jog.de.j );
        p->move( &E, aux.jog.para.i, aux.jog.para.j );

        turno_morte = !turno_morte;

        FILE* arq=fopen("minmax.txt", "a");
        fprintf(arq, "\nMINMAX:\t%d\n", num_minmax);
        fclose(arq);
        /// salvar log no arquivo
        num_minmax=0;
    }
    /// se for turno das Pessoas, computador joga sozinho
}


void interface::paintTiles(Lista_jogada *l, float color[])
{
    jogada *el = l->prim;
    while( el != NULL )
    {

        int i = (el->para.i - 4)*TILE_DIM;
        int j = (el->para.j - 4)*TILE_DIM;
        el = el->prox;

        float pos[] = {j + TILE_DIM/2, i + TILE_DIM/2, 0};
        float scale[] = {TILE_DIM, TILE_DIM, 1};

        GLAux::drawPlane( pos, scale, color );
    }

}

ExPeca *interface::mousePicking( int &jog_i, int &jog_j )
{
    float mouse_x = GLAux::mouse_x;
    float mouse_y = GLAux::mouse_y;

    //Calcula o tile sobre o mouse
    int x = (floor(mouse_x/TILE_DIM))*TILE_DIM;
    int y = (floor(mouse_y/TILE_DIM))*TILE_DIM;

    //Checa se o mouse está nos limites d tabuleiro
    if( x >= (- TAB_DIM/2 + TAB_X ) && x < (TAB_DIM/2 + TAB_X ) && y >= ( -TAB_DIM/2 + TAB_Y ) && y < ( TAB_DIM/2 + TAB_Y ) )
    {
        //Pinta o tile sobre o mouse
        float pos[] = {x + TILE_DIM/2, y + TILE_DIM/2, 0};
        float scale[] = {TILE_DIM, TILE_DIM, 1};
        float color[] = {1,1,1,0.3};

        GLAux::drawPlane( pos, scale, color );
    }

    //Posição do mouse onde houve um clique
    mouse_x = GLAux::mouse_click_x;
    mouse_y = GLAux::mouse_click_y;

    int i = (floor(mouse_y/TILE_DIM)) + 4;
    int j = (floor(mouse_x/TILE_DIM)) + 4;

    ExPeca *peca = ExPeca::getPecaByPosition(pecas,i,j);

    //TODO: Menu de habilidades será exibido

    //Exibe os possiveis movimentos para uma determinada peça
    if( GLAux::mousestate[L_BUTTON] == 1 )
    {

        if( peca != NULL )
        {
            grab = true;
            //TODO:Escolher a habilidade da peça a ser utilizada e esta retorna as casas que ela afeta
            Lista_jogada *l = peca->p->movimentos(E.tab, peca->pos_i, peca->pos_j);

            if( peca->player == MORTE )
            {
                float cor[] = {1,0,0,0.5};
                paintTiles(l,cor);
            }
            else if( peca->player == PESSOAS )
            {
                float cor[] = {0,0,1,0.5};
                paintTiles(l, cor);
            }
        }
    }

    //Quando o botão do mouse é solto, se a jogada for valida o movimento é executado
    if( GLAux::mousestate[L_BUTTON] == 0 )
    {

        if( peca != NULL && grab == true)
        {
            grab = false;
            float mouse_x = GLAux::mouse_x;
            float mouse_y = GLAux::mouse_y;

            //Calcula o tile sobre o mouse
            int j = (floor(mouse_x/TILE_DIM)) + 4;
            int i = (floor(mouse_y/TILE_DIM)) + 4;
            jog_i = i;
            jog_j = j;
            return peca;
        }
        grab = false;

    }
    jog_i = -1;
    jog_j = -1;
    return NULL;

}

void interface::gameLoop()
{
    drawTab();

    if( enable_for_players )
    {
        if(!turno_morte)
        {
            if(opcoes[0] == 0)
                playerCpu(PESSOAS);
            else
                playerJog(PESSOAS);
        }
        else
        {
            if(opcoes[1] == 0)
                playerCpu(MORTE);
            else
            {
                playerJog(MORTE);
            }
        }
    }
    enable_for_players = true;
    //TODO:O turno não deve terminar até que todas as peças terminem suas ações iniciadas nesse turno. ( até que o estado de todas seja DEAD ou STAND )
    for( int i=0; i<pecas.size; i++ )
    {
        //Se ainda houver alguma ação em execução não permitir jogadas
        if( pecas.get(i)->stateManager(&E, &pecas) == false )
            enable_for_players = false;

    }

    List<ExPeca> render_ordem;
    for( int i=0; i<pecas.size; i++ )
    {
        ExPeca * p = pecas.get(i);
        //p->animManager();
        render_ordem.add(p, p->y*1000 );
    }


    for( int i=render_ordem.size -1; i>=0; i-- )
    {
        ExPeca * p = render_ordem.get(i);
        p->animManager();
        if( p->type == ELETRICA )
        {
            float posy = (p->y - 4.0f)*TILE_DIM + TILE_DIM/2;
            float posx = (p->x - 4.0f)*TILE_DIM + TILE_DIM/2;
            a.setReference(posx, posy);
            a.runAnimation();
        }
    }

    render_ordem.free();

//    for(int i=0; i<8; i++)
//    for(int j=0; j<8; j++)
//    {
//        if( E.tab[i][j] == ELETRICIDADE )
//        {
//            int y = (i - 4)*TILE_DIM;
//            int x = (j - 4)*TILE_DIM;
//            float pos[] = {x + TILE_DIM/2, y + TILE_DIM/2, 0};
//            float scale[] = {TILE_DIM, TILE_DIM, 1};
//            float color[] = {0.5,0.5,0,0.5};
//
//            Texture *tex = (Texture*)Resources::getResource(TEXTURE,6);
//            GLAux::drawPlane( pos, scale, tex  );
//        }
//    }

    GLAux::Timer(60);



    //TODO: Não pode haver mais de uma jogada no mesmo turno ( obs: turno só termina quando todas as ações desencadeadas pela jogada terminarem )
    //Utiliza a lista de peças para atualizar o estado do tabuleiro.
    updateEstado();

    if( victoryTest() == true )
        enable_for_players = false;
}

void interface::updateEstado()
{
    for(int i=0; i<8; i++)
    for(int j=0; j<8; j++)
        E.tab[i][j] = VAZIO;
    for( int i=0; i<pecas.size; i++ )
    {
        if( pecas.get(i)->est_pilha.topo() != DEAD )
            pecas.get(i)->posicionar(&E);
    }


    /// definir estado inicial
    E.turno_morte = turno_morte;
    E.atualizar_posicoes();
    /// atualizar estado E

    //mov->clear();

    mov = new Lista_jogada;
    /// inicializar as listas de jogadas e de estados

    E.sucessor(mov);
    /// gerar todas as possiveis jogadas, e salvar em 'mov'
    E.origem = new jogada(0,0,0,0,1);

}




