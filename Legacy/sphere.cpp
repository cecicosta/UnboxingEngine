#include "sphere.h"
#include <math.h>

sphere::sphere()
{
    this->radius = 1;
}
sphere::sphere(float radius)
{
    this->radius = radius;
}

sphere::sphere(vector3D position, float radius)
{
    this->position = position;
    this->radius = radius;
    this->v = vector3D(0,0,0);
    this->w = vector3D(0,0,0);
    this->mass = 0.045;
    this->inertia = (2.0/5.0)*(mass*pow(radius, 2));
    this->COR =0.60;
}

void sphere::setRadius( float radius )
{
    this->radius = radius;
}

float sphere::getRadius()
{
    return radius;
}

vector3D sphere::getNormal(vector3D point)
{
    return position - point;
}

bool sphere::isInside( vector3D point )
{
    vector3D dif;
    dif = position - point;
    float mod = dif.modulo();
    if( mod < 0 )
        mod = - mod;

    if( mod <= radius )
        return true;

    return false;
}

//Metodo que checa se um raio dado pelos dois pontos p1 e p2 intercepta a esfera.
bool sphere::intersectionWithRay( vector3D p1, vector3D p2, vector3D &intersection )
{
    if(p1.x == p2.x && p1.y == p2.y && p1.z == p2.z )
        return false;
    //Primeiramente o raio é representado como um vetor de p1 para p2
    vector3D ray;
    ray = p2 - p1;

    //P representa a distancia de p1 para o centro da esfera
    vector3D p;
    p = position - p1;

    // m, n e k são os coeficientes obtidos a partir da equação da esfera
    float m = pow(ray.x,2) + pow(ray.y,2) + pow(ray.z,2);
    float n = -2*(ray.x*p.x + ray.y*p.y + ray.z*p.z);
    float k = pow(p.x,2) + pow(p.y,2) + pow(p.z,2) - pow(radius, 2);

    //Calcula-se o delta da formula de bhaskara, se este for negativo a equação não tem solução, logo o raio não intercepta a esfera
    float delta = pow( n, 2 ) - 4*m*k;

    if( delta < 0 )
        return false;

    // t, paremetro que fornece o ponto que o raio cruza a esfera
    float t1 = (- n + sqrt( delta ))/( 2*m );
    float t2 = (- n - sqrt( delta ))/( 2*m );


    //Checa-se  primeiro ponto que onde a esfera foi interceptada
    if( (ray*t1).modulo() < (ray*t2).modulo() )
        intersection = ray*t1;
    else
        intersection = ray*t2;

    //Calcula o ponto referente ao sistema de coordenadas
    intersection = intersection + p1;

    return true;
}

bool sphere::intersectionWithLine( vector3D p1, vector3D p2, vector3D &intersection )
{
    if(p1.x == p2.x && p1.y == p2.y && p1.z == p2.z )
        return false;
    //Primeiramente o raio é representado como um vetor de p1 para p2
    vector3D ray;
    ray = p2 - p1;

    //P representa a distancia de p1 para o centro da esfera
    vector3D p;
    p = position - p1;

    // m, n e k são os coeficientes obtidos a partir da equação da esfera
    float m = pow(ray.x,2) + pow(ray.y,2) + pow(ray.z,2);
    float n = -2*(ray.x*p.x + ray.y*p.y + ray.z*p.z);
    float k = pow(p.x,2) + pow(p.y,2) + pow(p.z,2) - pow(radius, 2);

    //Calcula-se o delta da formula de bhaskara, se este for negativo a equação não tem solução, logo o raio não intercepta a esfera
    float delta = pow( n, 2 ) - 4*m*k;

    if( delta < 0 )
        return false;

    // t, paremetro que fornece o ponto que o raio cruza a esfera
    float t1 = (- n + sqrt( delta ))/( 2*m );
    float t2 = (- n - sqrt( delta ))/( 2*m );

    if( ((ray*t1).modulo() > (p2 - p1).modulo() && (ray*t2).modulo() > (p2 - p1).modulo()) )
        return false;

    //Checa-se  primeiro ponto que onde a esfera foi interceptada
    if( (ray*t1).modulo() < (ray*t2).modulo() )
        intersection = ray*t1;
    else
        intersection = ray*t2;

    //Calcula o ponto referente ao sistema de coordenadas
    intersection = intersection + p1;

    return true;
}


