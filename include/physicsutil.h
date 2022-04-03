#include "vector3D.h"
#include "sphere.h"
#include "SceneElement.h"
/*
Tratamento de colisao

Seja F o vetor representando a força exercida sobre o corpo.
A componente de F na normal a face onde houve a colisao será dada por,

F' = F.cos%

onde % é o angulo entre a normal e o vetor F.

O cosseno pode ser calculado pelo produto escalar de F pela normal (normalizada), dividido pelo modulo de F.

O movimento do corpo pode ser decomposto em dois tipos, translação e rotação.

O movimento de translação será causado pela componente de F' no vetor que liga o ponto de intersecção ao centro de massa do corpo.

O movimento de rotação ocorrerrá devido ao torque provocado pela componente de F' perpendicular ao vetor que liga o ponto de intersecção ao centro.

Se considerarmos o atrito, seu modulo será dado por,

|Fa| = |F'|.@

Onde @ é o coeficiente de atrito do objeto.

Sua direção e sentido serão dados pela componente de F perpendicular a normal da face e contida no plano definido por F e a normal.
*/


class physicut
{
    public:
    static vector3D gravity;
    static List<SceneElement> ph_objects;

    static int add(SceneElement *e);

    static vector3D angularVelocity( double inercia, vector3D raio, vector3D strg, double tempo, vector3D velI = vector3D(0,0,0) );
    static vector3D linearVelocity( double massa, vector3D strg, double tempo, vector3D velI = vector3D(0,0,0) );

    //Trata a ação de uma força sobre um objeto ( aplicavel em objetos descritos matematicamente )
    static bool strengthOnObject( SceneElement *object, vector3D strg, vector3D reference, double tempo );

    //Movimento uniformemente acelerado. Recebe como paramentro os dados da equação S = S0 + V0 + (at^2)/2, e retorna o espaço percorrido.
    static vector3D movement( double tInterval, vector3D iPosition, vector3D iVelocity, vector3D speed  );

    static vector3D velocity( double tInterval, vector3D iVelocity, vector3D speed );

    static bool collisionDetectAndTreat( int id1, int id2 );

    static void run(double t);
};

