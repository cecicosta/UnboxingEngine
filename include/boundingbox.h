#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H
#include "Vector3d.h"
#include "sphere.h"
#define TOP 0
#define BOTTOM 1
#define FRONT 2
#define BACK 3
#define RIGHT 4
#define LEFT 5

class BoundingBox
{
    public:
    int faces[6][4];
    vector3D vertices[8];
    vector3D center;

    BoundingBox();
    BoundingBox( vector3D center, vector3D max, vector3D min );

    bool isInside(vector3D point);
    bool intersectionWithRay(vector3D point, vector3D ray, vector3D &intersection);
    bool intersectionWithSphere(sphere esf);
    void drawWireframe();
    vector3D getVertex(int side, int vert);
    vector3D getMax();
    vector3D getMin();
    vector3D getCenter();

    private:
    bool isInsideTriangle( vector3D v1, vector3D v2, vector3D v3, vector3D intersection );
    float modulo( float val )
    {
        if( val < 0 )
            return -val;
        return val;
    }
};

#endif //BOUNDING_BOX_H
