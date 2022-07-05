#include "SceneComposite.h"
#include "UVector.h"
#include "sphere.h"
/*
Tratamento de colisao

Seja F o vetor representando a for�a exercida sobre o corpo.
A componente de F na normal a face onde houve a colisao ser� dada por,

F' = F.cos%

onde % � o angulo entre a normal e o vetor F.

O cosseno pode ser calculado pelo produto escalar de F pela normal (normalizada), dividido pelo modulo de F.

O movimento do corpo pode ser decomposto em dois tipos, transla��o e rota��o.

O movimento de transla��o ser� causado pela componente de F' no vetor que liga o ponto de intersec��o ao centro de massa do corpo.

O movimento de rota��o ocorrerr� devido ao torque provocado pela componente de F' perpendicular ao vetor que liga o ponto de intersec��o ao centro.

Se considerarmos o atrito, seu modulo ser� dado por,

|Fa| = |F'|.@

Onde @ � o coeficiente de atrito do objeto.

Sua dire��o e sentido ser�o dados pela componente de F perpendicular a normal da face e contida no plano definido por F e a normal.
*/


class physicut
{
    public:
    static vector3D gravity;
    static List<SceneElement> ph_objects;

    static int add(SceneElement *e);

    static vector3D angularVelocity( double inercia, vector3D raio, vector3D strg, double tempo, vector3D velI = vector3D(0,0,0) );
    static vector3D linearVelocity( double massa, vector3D strg, double tempo, vector3D velI = vector3D(0,0,0) );

    //Trata a a��o de uma for�a sobre um objeto ( aplicavel em objetos descritos matematicamente )
    static bool strengthOnObject( SceneElement *object, vector3D strg, vector3D reference, double tempo );

    //Movimento uniformemente acelerado. Recebe como paramentro os dados da equa��o S = S0 + V0 + (at^2)/2, e retorna o espa�o percorrido.
    static vector3D movement( double tInterval, vector3D iPosition, vector3D iVelocity, vector3D speed  );

    static vector3D velocity( double tInterval, vector3D iVelocity, vector3D speed );

    static bool collisionDetectAndTreat( int id1, int id2 );

    static void run(double t);
};

