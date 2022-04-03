#include "mapa.h"
#include "SELoader.h"

Mapa::Mapa(){
    l.inicializar();
    l.ronald_lab();


    SEmap = new List<SceneElement*>[l.rows][l.columns];

    for(int i=0; i<l.rows; i++){
        for(int j=0; j<l.columns; j++){
            SceneElement* se = SELoader::getByID(l.matriz.obter_valor(j, i));
            se.setPosition(vector3D(MAP_X + j*TILE_DIM + TILE_DIM/2, MAP_Y + i*TILE_DIM + TILE_DIM/2, 0));
            SEmap[i][j].add(se);
        }
    }
    size_i = l.rows;
    size_j = l.columns;
}

void Mapa::drawAll(){

    for(int i=0; i<size_i; i++){
        for(int j=0; j<size_j; j++){
            for(int k=0; k<SEmap[i][j].size; k++){
                SceneElement* se = SEmap[i][j].get(k);

                if( se.getType() == SIMPLE )
                {
                    //TODO: usar sistema de animação para renderizar
                    float pos[] = {se.getPosition().x, se.getPosition().y, se.getPosition().z};
                    float scale[] = {se.getScale().x, se.getScale().y, se.getScale().z};
                    //se.reg.draw(pos, scale, Resources::getResource( TEXTURE, texID);
                }
            }
        }
    }
}
