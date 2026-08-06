#pragma once

#include "UVector.h"
#include "LegacyCollisionAlgorithms.h"
#include <math.h>

namespace Mechanics {

const Vector3f kG = Vector3f(0, -9.8, 0);

/** From the definition L = I x w (Angular momentum equals, the cross product of the
 * Momentum of Inertia ({\sum_{i=1}^{n} = m_i r_{i}^{2}}) and Angular Velocity (w)
 */
inline Vector3f AngularVelocity(const float momentumOfInertia, const Vector3f& radius, const Vector3f& strength, float t, const Vector3f& initialVelocity) {
    Vector3f T = radius.CrossProduct(strength); // Torque
    Vector3f L = T * t; // Angular momentum
    float wMod = L.Length() / momentumOfInertia; // angular velocity intensity
    // Vetor velocidade angular, obtido o atrav�s do produto vetorial do momento angular pelo raio, normalizado, multiplicado pelo modulo da velocidade angular.
    // Angular velocity V_w = L \times r
    Vector3f w = L.CrossProduct(radius).Normalized() * wMod;

    return w + initialVelocity;
}
Vector3f linearVelocity(float massa, Vector3f strg, float tempo, Vector3f velI) {
    //Impulso representa a varia��o do momento linear
    Vector3f I = strg * tempo;

    // I = mv1 - mv0 -> I/m = v1 - v0 -> v1 = I/m + v0
    Vector3f v = velI + I / massa;

    return v;
}

//Trata a a��o de uma for�a sobre um objeto ( aplicavel em objetos descritos matematicamente )
bool strengthOnObject(SceneElement *object, Vector3f strg, Vector3f reference, float tempo) {
    Vector3f intersection;
    if (!object->reg.intersectionWithRay(reference, reference + strg, intersection))
        return false;

    Vector3f normal = object->reg.getNormal(intersection);
    normal = normal.Normalized();
    float cosseno = strg.escalar(normal) / strg.Length();
    Vector3f resultante = strg * cosseno;


    //Vetor que liga o centro de massa do objeto e o ponto de intersec��o
    Vector3f raio = intersection - object->reg.position;
    //Componente da reaultante que afeta a transla��o
    Vector3f tComponent = raio.Normalized() * resultante.escalar(raio.Normalized());
    //Calcula velocidade de transla��o
    Vector3f vComponent = linearVelocity(object->reg.mass, tComponent, tempo, object->reg.v);
    object->reg.v = vComponent;


    //Vetor perpendicular ao plano definido pelo raio e a resultante
    Vector3f aux = resultante.CrossProduct(raio);
    //Vetor perpendicular ao raio contido no plano definido pelo raio e a resultante
    Vector3f aComponent = raio.CrossProduct(aux).Normalized();
    //Modulo da componente da resultante na dire��o do vetor perpendicular ao reio e contido no plano do raio com a resultante
    float mod = resultante.escalar(aComponent);
    //Vetor for�a que aplica torque no objeto
    aComponent = aComponent * mod;

    Vector3f wComponent = AngularVelocity(object->reg.inertia,
                                          intersection + -1 * object->reg.position,
                                          aComponent,
                                          tempo,
                                          object->reg.w);
    object->reg.w = wComponent;

    printf("ang: (%f,%f,%f)\n", aComponent.x, aComponent.y, aComponent.z);

    return true;
}

//Movimento uniformemente acelerado. Recebe como paramentro os dados da equa��o S = S0 + V0 + (at^2)/2, e retorna o espa�o percorrido.
Vector3f movement(float tInterval, Vector3f iPosition, Vector3f iVelocity, Vector3f speed) {
    Vector3f aux = iPosition + iVelocity * tInterval;
    Vector3f s = aux + speed * pow(tInterval, 2) / 2;
    return s;
}

Vector3f velocity(float tInterval, Vector3f iVelocity, Vector3f speed) {
    return iVelocity + speed * tInterval;
}

bool collisionDetectAndTreat(int id1, int id2) {
    std::vector<Vector3f> intersections;
    std::vector<Vector3f> faces;

    if () {
        Vector3f intersection = *(intersections.get(0));
        SceneElement *e1 = ph_objects.getByID(id1);
        SceneElement *e2 = ph_objects.getByID(id2);

        Vector3f normal1 = intersection - e1->getPosition();
        Vector3f normal2 = intersection - e2->getPosition();

        //Calcula-se a componente da velocidade do objeto na dire��o da normal
        Vector3f vel1 = e1->reg.v;
        float velMod1 = vel1.escalar(normal2);
        vel1 = normal2 * velMod1;

        Vector3f vel2 = e2->reg.v;
        float velMod2 = vel2.escalar(normal1);
        vel2 = normal1 * velMod2;

        //Fixa-se um determinado tempo para o qual os objetos permaneceram em contato ap� a colis�o ( para calculo do impulso )
        float tempo = 1.0 / 200.0;

        Vector3f vel_relativa = e2->reg.v - e1->reg.v;
        float velMod = vel_relativa.escalar(normal1);
        vel2 = normal1 * velMod;

        Vector3f force = -(e1->reg.COR * e2->reg.COR) * vel_relativa - vel_relativa;
        force = force * e1->reg.mass / tempo;

        strengthOnObject(e1, force, intersection, tempo);
        strengthOnObject(e2, force * -1, intersection, tempo);

        //
        //
        //        //Fixado o tempo, calcula-se a for�a media atuante durante a colis�o com base no coeficiente de restitui��o  do objeto.
        //        //Segundo a formula, e = - Vf/Vi e a formula para o impulso dada por, I = F.t = mVf - mVi, varia��o do momento angular.
        //        //A partir dai, obtemen-se, F = m/t( -eVi - Vi )
        //        Vector3f force1 =  -e1->reg.COR*vel1 - vel1;
        //        force1 = force1*e1->reg.mass/tempo;
        //
        //        //Aplicando a for�a sobre o objeto e atribuindo sua velocidade ap�s a colis�o.
        //        strengthOnObject( e1, force1, intersection, tempo );
        //
        //Trata a colis�o entre a superficie dos objetos
        Vector3f distancia1 = intersection - e1->reg.position;
        Vector3f raio1 = distancia1.Normalized() * e1->reg.radius;
        Vector3f incremento1 = raio1 - distancia1;
        Vector3f pos1 = e1->reg.position - incremento1;
        e1->setPosition(pos1);
        //
        //
        //        Vector3f force2 =  -e2->reg.COR*vel2 - vel2;
        //        force2 = force2*e2->reg.mass/tempo;
        //
        //        //Aplicando a for�a sobre o objeto e atribuindo sua velocidade ap�s a colis�o.
        //        strengthOnObject( e2, force2, intersection, tempo );
        //
        //        printf("force1: (%f,%f,%f)\n", force1.x, force1.y, force1.z);
        //        printf("force2: (%f,%f,%f)\n", force2.x, force2.y, force2.z);
        //
        //Trata a colis�o entre a superficie dos objetos
        Vector3f distancia2 = intersection - e2->reg.position;
        Vector3f raio2 = distancia2.Normalized() * e2->reg.radius;
        Vector3f incremento2 = raio2 - distancia2;
        Vector3f pos2 = e2->reg.position - incremento2;
        e2->setPosition(pos2);

        return true;
    }

    if (collut::betweenObjects(id1, id2, NOSIMPLE_SIMPLE, intersections, faces)) {
        for (int i = 0; i < intersections.size; i++) {

            Vector3f triang[3];
            triang[0] = *(faces.get(3 * i + 0));
            triang[1] = *(faces.get(3 * i + 1));
            triang[2] = *(faces.get(3 * i + 2));
            Vector3f intersection = *(intersections.get(i));

            SceneElement *e2 = ph_objects.getByID(id2);
            //printf( "Colis�o, intante %f\n", SDL_GetTicks()/1000.0 - tInicial );

            //Vetor normal a face do objeto identificado pelo id, com o qual ouve colis�o.
            Vector3f normal = (triang[1] - triang[0]).CrossProduct(triang[2] - triang[0]);
            normal = normal.Normalized();

            //Calcula-se a componente da velocidade do objeto na dire��o da normal
            Vector3f vel = e2->reg.v;
            float velMod = vel.escalar(normal);
            vel = normal * velMod;

            Vector3f v = e2->reg.v;

            //Fixa-se um determinado tempo para o qual os objetos permaneceram em contato ap� a colis�o ( para calculo do impulso )
            float tempo = 1.0 / 200.0;

            //Calcula-se a componente da velocidade do objeto no plano
            Vector3f componente = v.Normalized().CrossProduct(normal);
            Vector3f vel_x = componente.CrossProduct(normal);
            float u = 0.8;
            Vector3f peso = kG * e2->reg.mass;
            Vector3f atrito = peso.Length() * u * vel_x;
            //printf("(%f,%f,%f)\n", atrito.x, atrito.y, atrito.z);
            Vector3f a = atrito / e2->reg.mass;

            Vector3f vf = e2->reg.v + a * tempo;
            e2->reg.v = vf;
            //strengthOnObject( e2, atrito, intersection, tempo );


            vel = e2->reg.v;
            velMod = vel.escalar(normal);
            vel = normal * velMod;

            //Fixado o tempo, calcula-se a for�a media atuante durante a colis�o com base no coeficiente de restitui��o  do objeto.
            //Segundo a formula, e = - Vf/Vi e a formula para o impulso dada por, I = F.t = mVf - mVi, varia��o do momento angular.
            //A partir dai, obtemen-se, F = m/t( -eVi - Vi )
            Vector3f force = -e2->reg.COR * vel - vel;
            force = force * e2->reg.mass / tempo;

            //Aplicando a for�a sobre o objeto e atribuindo sua velocidade ap�s a colis�o.
            strengthOnObject(e2, force, intersection, tempo);

            //Trata a colis�o entre a superficie dos objetos
            Vector3f distancia = intersection - e2->reg.position;
            Vector3f raio = distancia.Normalized() * e2->reg.radius;
            Vector3f incremento = raio - distancia;
            Vector3f pos = e2->reg.position - incremento;
            e2->setPosition(pos);
        }

        return true;
    }

    return false;
}


void run(float t) {
    for (int i = 0; i < ph_objects.size; i++) {
        SceneElement *e = ph_objects.getByID(i);
        if (e->getType() == SIMPLE) {
            Vector3f pos = movement(t, e->reg.position, e->reg.v, kG);
            Vector3f vel = velocity(t, e->reg.v, kG);
            e->setPosition(pos);
            e->reg.v = vel;
        }
    }
}
}// namespace Mechanics
