#include "region.h"

void region::setVelocity( vector3D v )
{
    this->v = v;
}

vector3D region::getVelocity()
{
    return this->v;
}

void region::setAngularVelocity( vector3D w )
{
    this->w = w;
}

vector3D region::getAngularVelocity()
{
    return this->w;
}

float region::getMass()
{
    return this->mass;
}
void region::setMass(float mass)
{
    this->mass = mass;
}

float region::getInertia()
{
    return this->inertia;
}

float region::getCOR()
{
    return COR;
}
