#include "physicsutil.h"
#include "collisionutil.h"
#include <math.h>
vector3D physicut::gravity = vector3D(0,0,0);
List<SceneElement> physicut::ph_objects;

int physicut::add(SceneElement *e)
{
    ph_objects.add(e, e->getID());
    collut::registerObjectForCollision(e);
}

vector3D physicut::angularVelocity( double inercia, vector3D raio, vector3D strg, double tempo, vector3D velI)
{
    vector3D torque = raio.vetorial( strg );
    vector3D mmAngular = torque*tempo;
    double velAngMod = mmAngular.modulo()/inercia;
    //Vetor velocidade angular, obtido o através do produto vetorial do momento angular pelo raio, normalizado, multiplicado pelo modulo da velocidade angular.
    vector3D w = mmAngular.vetorial( raio ).normalizado()*velAngMod;

    return w + velI;
}
vector3D physicut::linearVelocity( double massa, vector3D strg, double tempo, vector3D velI )
{
    //Impulso representa a variação do momento linear
    vector3D I =  strg*tempo;

    // I = mv1 - mv0 -> I/m = v1 - v0 -> v1 = I/m + v0
    vector3D v = velI + I/massa;

    return v;
}

//Trata a ação de uma força sobre um objeto ( aplicavel em objetos descritos matematicamente )
bool physicut::strengthOnObject( SceneElement *object, vector3D strg, vector3D reference, double tempo )
{
    vector3D intersection;
    if( !object->reg.intersectionWithRay( reference, reference + strg, intersection ) )
        return false;

    vector3D normal = object->reg.getNormal(intersection);
    normal = normal.normalizado();
    double cosseno = strg.escalar( normal )/strg.modulo();
    vector3D resultante = strg*cosseno;



    //Vetor que liga o centro de massa do objeto e o ponto de intersecção
    vector3D raio = intersection - object->reg.position;
    //Componente da reaultante que afeta a translação
    vector3D tComponent = raio.normalizado()*resultante.escalar( raio.normalizado() );
    //Calcula velocidade de translação
    vector3D vComponent = linearVelocity( object->reg.mass, tComponent, tempo, object->reg.v );
    object->reg.v = vComponent;


    //Vetor perpendicular ao plano definido pelo raio e a resultante
    vector3D aux = resultante.vetorial( raio );
    //Vetor perpendicular ao raio contido no plano definido pelo raio e a resultante
    vector3D aComponent = raio.vetorial( aux ).normalizado();
    //Modulo da componente da resultante na direção do vetor perpendicular ao reio e contido no plano do raio com a resultante
    double mod = resultante.escalar( aComponent );
    //Vetor força que aplica torque no objeto
    aComponent = aComponent*mod;

    vector3D wComponent = angularVelocity(  object->reg.inertia,
                                            intersection + -1*object->reg.position,
                                            aComponent,
                                            tempo,
                                            object->reg.w );
    object->reg.w = wComponent;

    printf("ang: (%f,%f,%f)\n", aComponent.x, aComponent.y, aComponent.z);

    return true;
}

//Movimento uniformemente acelerado. Recebe como paramentro os dados da equação S = S0 + V0 + (at^2)/2, e retorna o espaço percorrido.
vector3D physicut::movement( double tInterval, vector3D iPosition, vector3D iVelocity, vector3D speed  )
{
    vector3D aux = iPosition + iVelocity*tInterval;
    vector3D s = aux + speed*pow( tInterval, 2 )/2;
    return s;
}

vector3D physicut::velocity( double tInterval, vector3D iVelocity, vector3D speed )
{
    return iVelocity + speed*tInterval;
}

bool physicut::collisionDetectAndTreat( int id1, int id2 )
{
    //Vetores que guardam o ponto de intersecção dos objetos e a face do objeto identificado pelo id, com a qual houve colisão
    List<vector3D> intersections;
    List<vector3D> faces;

    bool coll = false;


    if( collut::betweenObjects(id1, id2, SIMPLE_SIMPLE, intersections, faces) )
    {
        vector3D intersection = *(intersections.get(0));
        SceneElement *e1 = ph_objects.getByID(id1);
        SceneElement *e2 = ph_objects.getByID(id2);

        vector3D normal1 = intersection - e1->getPosition();
        vector3D normal2 = intersection - e2->getPosition();

        //Calcula-se a componente da velocidade do objeto na direção da normal
        vector3D vel1 = e1->reg.v;
        double velMod1 = vel1.escalar( normal2 );
        vel1 =  normal2*velMod1;

        vector3D vel2 = e2->reg.v;
        double velMod2 = vel2.escalar( normal1 );
        vel2 =  normal1*velMod2;

        //Fixa-se um determinado tempo para o qual os objetos permaneceram em contato apó a colisão ( para calculo do impulso )
        double tempo = 1.0/200.0;

        vector3D vel_relativa = e2->reg.v - e1->reg.v;
        double velMod = vel_relativa.escalar( normal1 );
        vel2 =  normal1*velMod;

        vector3D force = - (e1->reg.COR*e2->reg.COR)*vel_relativa - vel_relativa;
        force = force *e1->reg.mass/tempo;

        physicut::strengthOnObject( e1, force, intersection, tempo );
        physicut::strengthOnObject( e2, force*-1, intersection, tempo );

//
//
//        //Fixado o tempo, calcula-se a força media atuante durante a colisão com base no coeficiente de restituição  do objeto.
//        //Segundo a formula, e = - Vf/Vi e a formula para o impulso dada por, I = F.t = mVf - mVi, variação do momento angular.
//        //A partir dai, obtemen-se, F = m/t( -eVi - Vi )
//        vector3D force1 =  -e1->reg.COR*vel1 - vel1;
//        force1 = force1*e1->reg.mass/tempo;
//
//        //Aplicando a força sobre o objeto e atribuindo sua velocidade após a colisão.
//        physicut::strengthOnObject( e1, force1, intersection, tempo );
//
        //Trata a colisão entre a superficie dos objetos
        vector3D distancia1 = intersection - e1->reg.position;
        vector3D raio1 = distancia1.normalizado()*e1->reg.radius;
        vector3D incremento1 = raio1 - distancia1;
        vector3D pos1 = e1->reg.position - incremento1;
        e1->setPosition(pos1);
//
//
//        vector3D force2 =  -e2->reg.COR*vel2 - vel2;
//        force2 = force2*e2->reg.mass/tempo;
//
//        //Aplicando a força sobre o objeto e atribuindo sua velocidade após a colisão.
//        physicut::strengthOnObject( e2, force2, intersection, tempo );
//
//        printf("force1: (%f,%f,%f)\n", force1.x, force1.y, force1.z);
//        printf("force2: (%f,%f,%f)\n", force2.x, force2.y, force2.z);
//
        //Trata a colisão entre a superficie dos objetos
        vector3D distancia2 = intersection - e2->reg.position;
        vector3D raio2 = distancia2.normalizado()*e2->reg.radius;
        vector3D incremento2 = raio2 - distancia2;
        vector3D pos2 = e2->reg.position - incremento2;
        e2->setPosition(pos2);

        return true;
    }

    if( collut::betweenObjects(id1, id2, NOSIMPLE_SIMPLE, intersections, faces) )
    {
        for(int i=0; i<intersections.size; i++)
        {

            vector3D triang[3];
            triang[0] = *(faces.get(3*i+0));
            triang[1] = *(faces.get(3*i+1));
            triang[2] = *(faces.get(3*i+2));
            vector3D intersection = *(intersections.get(i));

            SceneElement *e2 = ph_objects.getByID(id2);
            //printf( "Colisão, intante %f\n", SDL_GetTicks()/1000.0 - tInicial );

            //Vetor normal a face do objeto identificado pelo id, com o qual ouve colisão.
            vector3D normal = (triang[1] - triang[0]).vetorial( triang[2] - triang[0] );
            normal = normal.normalizado();

            //Calcula-se a componente da velocidade do objeto na direção da normal
            vector3D vel = e2->reg.v;
            double velMod = vel.escalar( normal );
            vel =  normal*velMod;

            vector3D v = e2->reg.v;

            //Fixa-se um determinado tempo para o qual os objetos permaneceram em contato apó a colisão ( para calculo do impulso )
            double tempo = 1.0/200.0;

            //Calcula-se a componente da velocidade do objeto no plano
            vector3D componente = v.normalizado().vetorial(normal);
            vector3D vel_x = componente.vetorial(normal);
            float u = 0.8;
            vector3D peso = gravity*e2->reg.mass;
            vector3D atrito = peso.modulo()*u*vel_x;
            //printf("(%f,%f,%f)\n", atrito.x, atrito.y, atrito.z);
            vector3D a = atrito/e2->reg.mass;

            vector3D vf = e2->reg.v + a*tempo;
            e2->reg.v = vf;
            //physicut::strengthOnObject( e2, atrito, intersection, tempo );


            vel = e2->reg.v;
            velMod = vel.escalar( normal );
            vel =  normal*velMod;

            //Fixado o tempo, calcula-se a força media atuante durante a colisão com base no coeficiente de restituição  do objeto.
            //Segundo a formula, e = - Vf/Vi e a formula para o impulso dada por, I = F.t = mVf - mVi, variação do momento angular.
            //A partir dai, obtemen-se, F = m/t( -eVi - Vi )
            vector3D force =  -e2->reg.COR*vel - vel;
            force = force*e2->reg.mass/tempo;

            //Aplicando a força sobre o objeto e atribuindo sua velocidade após a colisão.
            physicut::strengthOnObject( e2, force, intersection, tempo );

            //Trata a colisão entre a superficie dos objetos
            vector3D distancia = intersection - e2->reg.position;
            vector3D raio = distancia.normalizado()*e2->reg.radius;
            vector3D incremento = raio - distancia;
            vector3D pos = e2->reg.position - incremento;
            e2->setPosition(pos);

        }

        return true;
    }

    return false;

}


void physicut::run(double t)
{
    for(int i=0; i<ph_objects.size; i++)
    {
        SceneElement *e = ph_objects.getByID(i);
        if( e->getType() == SIMPLE )
        {
            vector3D pos = physicut::movement(t, e->reg.position, e->reg.v, gravity);
            vector3D vel = physicut::velocity(t, e->reg.v, gravity);
            e->setPosition(pos);
            e->reg.v = vel;
        }
    }
}

