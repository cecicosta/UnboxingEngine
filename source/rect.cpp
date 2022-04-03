#include "rect.h"
#include "collisionutil.h"

bool rect::isInside(vector3D point){
    bool collision;
    collision = collut::isInsideTriangle( a, b, c, point);
    collision = collut::isInsideTriangle( a, c, d, point);

    return collision;
}

bool rect::intersectionWithRay( vector3D p1, vector3D p2, vector3D &intersection ){
    bool collision;
    collision = collut::rayWithTriangle(p1, p2-p1, a, b, c, intersection);
    collision = collut::rayWithTriangle(p1, p2-p1, a, c, d, intersection);

    return collision;
}
bool rect::intersectionWithLine( vector3D p1, vector3D p2, vector3D &intersection ){
}
