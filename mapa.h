#ifndef MAPA_H
#define MAPA_H

#include "mazeGenerator.h"
#include "matrix.h"
#include "SceneElement.h"

#define MAP_X 0
#define MAP_Y 0
#define TILE_DIM 30

class Mapa{
    Labirinto l;
    List<SceneElement*> elements;
    List<SceneElement*> **SEmap;
    int size_i;
    int size_j;
public:
    Mapa();
    bool checkCollision();
    void updateMapa();
    void drawAll();
};

#endif
