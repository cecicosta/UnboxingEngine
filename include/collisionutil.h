#include "MeshBuffer.h"
#include "SceneComposite.h"
#include "UVector.h"
#include "lista.h"
#include "octree.h"

#define SIMPLE_NOSIMPLE   0
#define NOSIMPLE_SIMPLE   1
#define SIMPLE_SIMPLE     2
#define NOSIMPLE_NOSIMPLE 3

class collut
{
    public:
    static List<octree> objects;
    static List<meshBuffer> mesh;
    static List<SceneElement> SEObjects;


    static void registerObjectForCollision( SceneElement *e );
    static bool rayWithObject(vector3D point, vector3D ray, int ObjectID, vector3D &intersection, face &poly);
    static void rayWithAllObjects(vector3D point, vector3D ray, List<int> &ObjectsID, List<vector3D> &intersections, List<face> &faces);
    static bool betweenObjects( int id1, int id2, int parameter, List<vector3D> &intersections, List<vector3D> &triangles );
    static bool rayWithTriangle(vector3D point, vector3D ray, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection);

    static bool isInsideTriangle( vector3D v1, vector3D v2, vector3D v3, vector3D intersection );
    static bool sphereWithTriangle( sphere spr, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection );
};
