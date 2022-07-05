#ifndef OCTREE_H
#define OCTREE_H
#include "MeshBuffer.h"
#define MAX_FACES 10000
#include "SceneComposite.h"
#include "lista.h"

class branch
{
    public:
    branch *child[8];
    uint nfaces;
    face *faces[MAX_FACES];
    BoundingBox box;
    branch(vector3D center, vector3D min, vector3D max);
    void subdivideOct();
};

class octree
{
    public:
    meshBuffer *mesh;
    branch *root;
    Matrix *transformation;

    public:
    octree( SceneElement *e );
    octree( meshBuffer *mesh );
    void draw(meshBuffer* mesh);
    private:
    static void createOctree( vertexcoord vertices[], List<face> *faces, branch *root);
    static bool isInsideOct( vector3D vertice, branch *oct );

};

#endif //OCTREE_H
