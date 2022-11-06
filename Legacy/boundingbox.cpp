#include "Legacy/boundingbox.h"
#include "Legacy/collisionutil.h"
#include "Legacy/draw.h"
#include "Legacy/lista.h"
#include <GL/gl.h>
#include <stdio.h>

BoundingBox::BoundingBox()
{
}

BoundingBox::BoundingBox( vector3D center, vector3D max, vector3D min )
{
    this->center = center;

    vertices[0].x = max.x;
    vertices[0].y = max.y;
    vertices[0].z = max.z;

    vertices[1].x = max.x;
    vertices[1].y = max.y;
    vertices[1].z = min.z;

    vertices[2].x = min.x;
    vertices[2].y = max.y;
    vertices[2].z = min.z;

    vertices[3].x = min.x;
    vertices[3].y = max.y;
    vertices[3].z = max.z;

    vertices[4].x = max.x;
    vertices[4].y = min.y;
    vertices[4].z = max.z;

    vertices[5].x = min.x;
    vertices[5].y = min.y;
    vertices[5].z = max.z;

    vertices[6].x = min.x;
    vertices[6].y = min.y;
    vertices[6].z = min.z;

    vertices[7].x = max.x;
    vertices[7].y = min.y;
    vertices[7].z = min.z;

    faces[TOP][0] = 0;
    faces[TOP][1] = 1;
    faces[TOP][2] = 2;
    faces[TOP][3] = 3;

    faces[BOTTOM][0] = 4;
    faces[BOTTOM][1] = 5;
    faces[BOTTOM][2] = 6;
    faces[BOTTOM][3] = 7;

    faces[FRONT][0] = 0;
    faces[FRONT][1] = 3;
    faces[FRONT][2] = 5;
    faces[FRONT][3] = 4;

    faces[BACK][0] = 1;
    faces[BACK][1] = 7;
    faces[BACK][2] = 6;
    faces[BACK][3] = 2;

    faces[RIGHT][0] = 3;
    faces[RIGHT][1] = 2;
    faces[RIGHT][2] = 6;
    faces[RIGHT][3] = 5;

    faces[LEFT][0] = 0;
    faces[LEFT][1] = 4;
    faces[LEFT][2] = 7;
    faces[LEFT][3] = 1;
}

bool BoundingBox::isInside(vector3D point)
{
    vector3D v0 = vertices[faces[1][0]] - vertices[faces[0][0]];
    vector3D v1 = vertices[faces[0][1]] - vertices[faces[0][0]];
    vector3D v2 = vertices[faces[0][3]] - vertices[faces[0][0]];
    vector3D v3 = v1.vetorial(v2);
    float volume = v3.normalizado().escalar(v0)*v3.modulo();
    float soma = 0;

    for(int i=0; i<6; i++)
    {
        vector3D normal = (vertices[faces[i][1]] - vertices[faces[i][0]]).vetorial(vertices[faces[i][3]] - vertices[faces[i][0]]).normalizado();
        float altura = (point - vertices[faces[i][0]]).escalar(normal);
        float base_area = (vertices[faces[i][1]] - vertices[faces[i][0]]).vetorial(vertices[faces[i][3]] - vertices[faces[i][0]]).modulo();
        float v_piramide = altura*base_area/3;

        soma += modulo(v_piramide);
    }

    if( modulo( soma ) - modulo( volume ) > 0.0001 )
        return false;

    return true;
}

bool BoundingBox::intersectionWithRay(vector3D point, vector3D ray, vector3D &intersection)
{
    List<vector3D> intersec;
    vector3D pontos[12];

    for(int i=0; i<6; i++)
    {
        vector3D v1 = vertices[faces[i][0]];
        vector3D v2 = vertices[faces[i][1]];
        vector3D v3 = vertices[faces[i][2]];
        vector3D v4 = vertices[faces[i][3]];

        vector3D normal = (v2 - v1).vetorial( v3 - v1 ).normalizado();

        vector3D ponto = v1;

        double a = normal.x;
        double b = normal.y;
        double c = normal.z;
        double d = -(a*ponto.x + b*ponto.y + c*ponto.z);

        double t = -(d + a*point.x + b*point.y + c*point.z )/(a*ray.x + b*ray.y + c*ray.z);

        vector3D inter = point + t*ray;


        if( isInsideTriangle(v1, v2, v3, inter) )
        {
            pontos[i] = inter;
            intersec.add(&pontos[i], (int) (point - inter).modulo());
        }
        normal = (v4 - v3).vetorial( v1 - v3 ).normalizado();

        ponto = v3;

        a = normal.x;
        b = normal.y;
        c = normal.z;
        d = -(a*ponto.x + b*ponto.y + c*ponto.z);

        t = -(d + a*point.x + b*point.y + c*point.z )/(a*ray.x + b*ray.y + c*ray.z);

        inter = point + t*ray;

        //printf("(%f,%f,%f)\n", inter.x, inter.y, inter.z);

        if( isInsideTriangle(v3, v4, v1, inter) )
        {
            pontos[2*i] = inter;
            intersec.add(&pontos[2*i], (int) (point - inter).modulo());
        }

    }

    if( !intersec.empty() )
    {
        intersection = *(intersec.get(0));
        return true;
    }
    intersection = vector3D(9999999,9999999,9999999);

    return false;
}

void BoundingBox::drawWireframe()
{
    glColor3f(0,0,1);

    for(int i=0; i<6; i++)
    {

        glBegin( GL_LINE_STRIP );
        glVertex3f(vertices[faces[i][3]].x, vertices[faces[i][3]].y, vertices[faces[i][3]].z);
        for(int j=0; j<4; j++)
            glVertex3f(vertices[faces[i][j]].x, vertices[faces[i][j]].y, vertices[faces[i][j]].z);

        glEnd();
    }
//
//    glLoadIdentity();
//    float size = max.x - min.x;
//    glTranslatef(center.x, center.y, center.z);
//    float scale[] = {size/100, size/100, size/100};
//    draw::esfera(scale);
//    glLoadIdentity();
}

bool BoundingBox::isInsideTriangle( vector3D v1, vector3D v2, vector3D v3, vector3D intersection )
{
    //Checa se a coordenada está limitada pelo triangulo
    double area = (v2-v1).vetorial(v3-v1).modulo()*0.5;
    vector3D vet1 = intersection - v1;
    vector3D vet2 = v2 - v1;
    double area1 = vet1.vetorial(vet2).modulo()*0.5/area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    double area2 = vet1.vetorial(vet2).modulo()*0.5/area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    double area3 = vet1.vetorial(vet2).modulo()*0.5/area;

    //Condição para que o ponto esteja contido no triangulo
    if( area1 + area2 + area3 <= 1.001 )
        return true;

    return false;
}

bool BoundingBox::intersectionWithSphere(sphere esf)
{

    for(int i=0; i<6; i++)
    {
        vector3D v1 = vertices[faces[i][0]];
        vector3D v2 = vertices[faces[i][1]];
        vector3D v3 = vertices[faces[i][2]];
        vector3D v4 = vertices[faces[i][3]];

        vector3D intersection;
        if( collut::sphereWithTriangle(esf, v1, v2, v3, intersection))
        {
            return true;
        }
        if( collut::sphereWithTriangle(esf, v3, v4, v1, intersection))
        {
            return true;
        }
        if( this->isInside(esf.position) )
        {
            return true;
        }
    }
    return false;
}

vector3D BoundingBox::getVertex(int side, int vert)
{
    return vertices[faces[side][vert]];
}
vector3D BoundingBox::getMax()
{
    return getVertex(TOP, 0);
}
vector3D BoundingBox::getMin()
{
    return getVertex(BOTTOM, 2);
}
vector3D BoundingBox::getCenter()
{
    return center;
}
