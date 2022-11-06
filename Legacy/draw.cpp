#include "draw.h"
void draw::normal(float p1[], float p2[], float p3[]){
    float veta[3], vetb[3], x,y,z;
    float n;
    veta[0] = p2[0] - p1[0];
    veta[1] = p2[1] - p1[1];
    veta[2] = p2[2] - p1[2];

    vetb[0] = p3[0] - p1[0];
    vetb[1] = p3[1] - p1[1];
    vetb[2] = p3[2] - p1[2];

    x = veta[1]*vetb[2] - veta[2]*vetb[1];
    y = veta[2]*vetb[0] - veta[0]*vetb[2];
    z = veta[0]*vetb[1] - veta[1]*vetb[0];

    n = sqrt( x*x + y*y + z*z );

    x/=n;
    y/=n;
    z/=n;
    glNormal3f(x,y,z);
}
//Desenha um plano 30x30
void draw::plano(float scale[])
{
    glBegin( GL_QUADS );
    glNormal3f(0,0,1);// Start Drawing A Quad
    glTexCoord2f(0,0); glVertex3f( -16*scale[0], -16*scale[1], 0);	// Bottom Left
    glTexCoord2f(1,0); glVertex3f(  16*scale[0], -16*scale[1], 0);	// Bottom Right
    glTexCoord2f(1,1); glVertex3f(  16*scale[0],  16*scale[1], 0);	// Top Right
    glTexCoord2f(0,1); glVertex3f( -16*scale[0],  16*scale[1], 0);	// Top Left
    glEnd();
}

//Desenha um cubo 30x30x30. Formato da textura utilizada 3x2 ( faces )
void draw::bloco(float scale[])
{
    meshBuffer mesh;
    // Front Face (1,1)
    glNormal3f(0,0,1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(0.333f, 0.0f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(0.333f, 0.5f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Top Left Of The Texture and Quad
    // Back Face (1,2)
    glNormal3f(0,0,-1);
    glTexCoord2f(0.666f, 0.0f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(0.666f, 0.5f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.333f, 0.5f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.333f, 0.0f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    // Top Face (1,3)
    glNormal3f(0,1,0);
    glTexCoord2f(0.666f, 0.5f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.666f, 0.0f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 0.5f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Right Of The Texture and Quad
    // Bottom Face (2,1)
    glNormal3f(0,-1,0);
    glTexCoord2f(0.333f, 1.0f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.5f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(0.333f, 0.5f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    // Right face (2,2)
    glNormal3f(1,0,0);
    glTexCoord2f(0.666f, 0.5f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(0.666f, 1.0f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.333f, 1.0f); glVertex3f( 0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Top Left Of The Texture and Quad
    glTexCoord2f(0.333f, 0.5f); glVertex3f( 0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    // Left Face (2,3)
    glNormal3f(-1,0,0);
    glTexCoord2f(0.666f, 0.5f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1], -0.5f*scale[2]);	// Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.5f); glVertex3f(-0.5f*scale[0], -0.5f*scale[1],  0.5f*scale[2]);	// Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1],  0.5f*scale[2]);	// Top Right Of The Texture and Quad
    glTexCoord2f(0.666f, 1.0f); glVertex3f(-0.5f*scale[0],  0.5f*scale[1], -0.5f*scale[2]);	// Top Left Of The Texture and Quad
glEnd();

}

void draw::esfera(float scale[])
{
//glDisable( GL_TEXTURE_2D );
    double radius = 1;
    double RAD_90 = 1.570796;
    double RAD_15 = 0.261799;
    double RAD_7_5 = 0.130899;
    double RAD_3_75 = 0.0654498;
    double RAD_12_85 = 0.2242748;

    for(int j=0; j<24; j++)
    {
        glBegin( GL_TRIANGLE_STRIP );
        for(int i=0; i<=12; i++)
        {
            if( i <= 6 )
            {
                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j)), sin(RAD_90 - RAD_15*i));
            }
            else
                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j)), 0);
            glNormal3f( sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1)));
            glVertex3f( radius*sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i)*scale[0], radius*sin(RAD_90 + RAD_15*i)*scale[1], radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1))*scale[2] );

            if( i <= 6 )
            {
                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j+1)), sin(RAD_90 - RAD_15*i));
            }
            else
                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j+1)), 0);
            glNormal3f(     sin(RAD_15*j)*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*j));
            glVertex3f(     radius*sin(RAD_15*j)*cos(RAD_90 + RAD_15*i)*scale[0], radius*sin(RAD_90 + RAD_15*i)*scale[1], radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*j)*scale[2] );
        }
        glEnd();
    }
//glEnable(GL_TEXTURE_2D);

}
    void draw::casa(const float scale[]){
        glEnable(GL_CULL_FACE);
        //pontos casa
        float p1[3] = {1*scale[0], 0, 0};
        float p2[3] = {0, 0, 0};
        float p3[3] = {0, 0, -2*scale[2]};
        float p4[3] = {1*scale[0], 0, -2*scale[2]};
        float p5[3] = {1*scale[0], 1*scale[1], 0};
        float p6[3] = {0, 1*scale[1], 0};
        float p7[3] = {0, 1*scale[1], -2*scale[2]};
        float p8[3] = {1*scale[0], 1*scale[1], -2*scale[2]};

        //pontos telhado
        float t1[3] = {1.2f*scale[0], 1*scale[1], 0};
        float t2[3] = {-0.2f*scale[0], 1*scale[1], 0};
        float t3[3] = {-0.2f*scale[0], 1*scale[1], -2*scale[2]};
        float t4[3] = {1.2f*scale[0], 1*scale[1], -2*scale[2]};
        float t5[3] = {0.5f*scale[0], 1.6f*scale[1], 0};
        float t6[3] = {0.5f*scale[0], 1.6f*scale[1], -2*scale[2]};

        //glEnable(GL_CULL_FACE);
        glPushMatrix();

        glBegin(GL_QUADS);
        //casa
        //frente
            normal(p1, p5, p6);
            glTexCoord2d(0,0);
            glVertex3fv(p1);
            glTexCoord2d(0,0.5);
            glVertex3fv(p5);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p6);
            glTexCoord2d(0.5,0);
            glVertex3fv(p2);

            normal(p1, p2, p6);
            glTexCoord2d(0,0);
            glVertex3fv(p1);
            glTexCoord2d(0.5,0);
            glVertex3fv(p2);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p6);
            glTexCoord2d(0,0.5);
            glVertex3fv(p5);
        //lado direito
            normal(p1, p4, p8);
            glTexCoord2d(0.5,0);
            glVertex3fv(p1);
            glTexCoord2d(1,0);
            glVertex3fv(p4);
            glTexCoord2d(1,0.5);
            glVertex3fv(p8);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p5);

            normal(p1, p5, p8);
            glTexCoord2d(0.5,0);
            glVertex3fv(p1);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p5);
            glTexCoord2d(1,0.5);
            glVertex3fv(p8);
            glTexCoord2d(1,0);
            glVertex3fv(p4);
        //tras
            normal(p4, p3, p7);
            glTexCoord2d(0,0.5);
            glVertex3fv(p4);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p3);
            glTexCoord2d(0.5,1);
            glVertex3fv(p7);
            glTexCoord2d(0,1);
            glVertex3fv(p8);

            normal(p4, p8, p7);
            glTexCoord2d(0,0.5);
            glVertex3fv(p4);
            glTexCoord2d(0,1);
            glVertex3fv(p8);
            glTexCoord2d(0.5,1);
            glVertex3fv(p7);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p3);
        //lado esquerdo
            normal(p3, p2, p6);
            glTexCoord2d(0.5,0);
            glVertex3fv(p3);
            glTexCoord2d(1,0);
            glVertex3fv(p2);
            glTexCoord2d(1,0.5);
            glVertex3fv(p6);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p7);

            normal(p3, p7, p6);
            glTexCoord2d(0.5,0);
            glVertex3fv(p3);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(p7);
            glTexCoord2d(1,0.5);
            glVertex3fv(p6);
            glTexCoord2d(1,0);
            glVertex3fv(p2);
        //baixo
            normal(p1, p2, p3);
            glVertex3fv(p1);
            glVertex3fv(p2);
            glVertex3fv(p3);
            glVertex3fv(p4);

            normal(p1, p4, p3);
            glVertex3fv(p1);
            glVertex3fv(p4);
            glVertex3fv(p3);
            glVertex3fv(p2);
        //teto
            //lado direito
            normal(t1, t4, t6);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(t1);
            glTexCoord2d(1,0.5);
            glVertex3fv(t4);
            glTexCoord2d(1,1);
            glVertex3fv(t6);
            glTexCoord2d(0.5,1);
            glVertex3fv(t5);

            //lado esquerdo
            normal(t3, t2, t5);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(t3);
            glTexCoord2d(1,0.5);
            glVertex3fv(t2);
            glTexCoord2d(1,1);
            glVertex3fv(t5);
            glTexCoord2d(0.5,1);
            glVertex3fv(t6);

            //baixo
            normal(t1, t2, t3);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(t1);
            glTexCoord2d(1,0.5);
            glVertex3fv(t2);
            glTexCoord2d(1,1);
            glVertex3fv(t3);
            glTexCoord2d(0.5,1);
            glVertex3fv(t4);

        glEnd();

        glBegin(GL_TRIANGLES);
            //frente
            normal(t2, t1, t5);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(t2);
            glTexCoord2d(1,0.5);
            glVertex3fv(t1);
            glTexCoord2d(0.75,1);
            glVertex3fv(t5);

            //tras
            normal(t4, t3, t6);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(t4);
            glTexCoord2d(1,0.5);
            glVertex3fv(t3);
            glTexCoord2d(0.75,1);
            glVertex3fv(t6);

        glEnd();
        glPopMatrix();
        glDisable(GL_CULL_FACE);
    }

    void draw::carroca(float scale[]){
        //Pontos
        //pontos carro
        float c1[3] = {0, 0, 0};
        float c2[3] = {1*scale[0], 0, 0};
        float c3[3] = {1*scale[0],0, -2*scale[2]};
        float c4[3] = {0, 0, -2*scale[2]};
        float c5[3] = {0, 1*scale[1], 0};
        float c6[3] = {1*scale[0], 1*scale[1], 0};
        float c7[3] = {1*scale[0], 1*scale[1], -2*scale[2]};
        float c8[3] = {0, 1*scale[1], -2*scale[2]};
        //pontos alsas
        float a1[3] = {0, 1*scale[1], -3.5f*scale[2]};
        float a2[3] = {0, 0.9f*scale[1], -3.5f*scale[2]};
        float a3[3] = {0, 0.9f*scale[1], -2*scale[2]};
        float a4[3] = {0, 1*scale[1], -2*scale[2]};
        float a5[3] = {1*scale[0], 1*scale[1], -3.5f*scale[2]};
        float a6[3] = {1*scale[0], 0.9f*scale[1], -3.5f*scale[2]};
        float a7[3] = {1*scale[0], 0.9f*scale[1], -2*scale[2]};
        float a8[3] = {1*scale[0], 1*scale[1], -2*scale[2]};

        float r1[3] = {-0.05f*scale[0], 0, -1*scale[2]};
        float r2[3] = {1.05f*scale[0], 0, -1*scale[2]};
        float rr[3] = {0,0.8f*scale[1],0.8f*scale[2]};
        glPushMatrix();
        //glEnable(GL_CULL_FACE);
        glBegin(GL_QUADS);
            //frente
            normal(c1, c2, c6);
            glTexCoord2d(0,1);
            glVertex3fv(c1);
            glTexCoord2d(0,0.5);
            glVertex3fv(c2);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c6);
            glTexCoord2d(0.5,1);
            glVertex3fv(c5);

            normal(c1, c5, c6);
            glTexCoord2d(0,1);
            glVertex3fv(c1);
            glTexCoord2d(0,0.5);
            glVertex3fv(c5);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c6);
            glTexCoord2d(0.5,1);
            glVertex3fv(c2);

            //direita
            normal(c2, c3, c7);
            glTexCoord2d(0,1);
            glVertex3fv(c2);
            glTexCoord2d(0,0.5);
            glVertex3fv(c3);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c7);
            glTexCoord2d(0.5,1);
            glVertex3fv(c6);

            normal(c2, c6, c7);
            glTexCoord2d(0,1);
            glVertex3fv(c2);
            glTexCoord2d(0,0.5);
            glVertex3fv(c6);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c7);
            glTexCoord2d(0.5,1);
            glVertex3fv(c3);

            //tras
            normal(c3, c4, c8);
            glTexCoord2d(0,1);
            glVertex3fv(c3);
            glTexCoord2d(0,0.5);
            glVertex3fv(c4);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c8);
            glTexCoord2d(0.5,1);
            glVertex3fv(c7);

            normal(c3, c7, c8);
            glTexCoord2d(0,1);
            glVertex3fv(c3);
            glTexCoord2d(0,0.5);
            glVertex3fv(c7);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c8);
            glTexCoord2d(0.5,1);
            glVertex3fv(c4);

            //esquerda
            normal(c4, c1, c5);
            glTexCoord2d(0,1);
            glVertex3fv(c4);
            glTexCoord2d(0,0.5);
            glVertex3fv(c1);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c5);
            glTexCoord2d(0.5,1);
            glVertex3fv(c8);

            normal(c4, c8, c5);
            glTexCoord2d(0,1);
            glVertex3fv(c4);
            glTexCoord2d(0,0.5);
            glVertex3fv(c8);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c5);
            glTexCoord2d(0.5,1);
            glVertex3fv(c1);

            //fundo
            normal(c1, c2, c3);
            glTexCoord2d(0,1);
            glVertex3fv(c1);
            glTexCoord2d(0,0.5);
            glVertex3fv(c2);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c3);
            glTexCoord2d(0.5,1);
            glVertex3fv(c4);

            normal(c1, c4, c3);
            glTexCoord2d(0,1);
            glVertex3fv(c1);
            glTexCoord2d(0,0.5);
            glVertex3fv(c4);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(c3);
            glTexCoord2d(0.5,1);
            glVertex3fv(c2);

            //alsa direita
            normal(a5, a8, a7);
            glTexCoord2d(0,1);
            glVertex3fv(a5);
            glTexCoord2d(0,0.5);
            glVertex3fv(a8);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(a7);
            glTexCoord2d(0.5,1);
            glVertex3fv(a6);

            normal(a5, a6, a7);
            glTexCoord2d(0,1);
            glVertex3fv(a5);
            glTexCoord2d(0,0.5);
            glVertex3fv(a6);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(a7);
            glTexCoord2d(0.5,1);
            glVertex3fv(a8);

            //alsa esquerda
            normal(a4, a1, a2);
            glTexCoord2d(0,1);
            glVertex3fv(a4);
            glTexCoord2d(0,0.5);
            glVertex3fv(a1);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(a2);
            glTexCoord2d(0.5,1);
            glVertex3fv(a3);

            normal(a4, a3, a2);
            glTexCoord2d(0,1);
            glVertex3fv(a4);
            glTexCoord2d(0,0.5);
            glVertex3fv(a3);
            glTexCoord2d(0.5,0.5);
            glVertex3fv(a2);
            glTexCoord2d(0.5,1);
            glVertex3fv(a1);

            //Roda 1
            glNormal3f(1, 0, 0);
            glTexCoord2d(0.5,0.5);
            glVertex3f(r1[0],r1[1]+rr[1],r1[2]+rr[2]);
            glTexCoord2d(0.5,1);
            glVertex3f(r1[0],r1[1]+rr[1],r1[2]-rr[2]);
            glTexCoord2d(1,1);
            glVertex3f(r1[0],r1[1]-rr[1],r1[2]-rr[2]);
            glTexCoord2d(1,0.5);
            glVertex3f(r1[0],r1[1]-rr[1],r1[2]+rr[2]);

            glNormal3f(-1, 0, 0);
            glTexCoord2d(0.5,0.5);
            glVertex3f(r1[0],r1[1]+rr[1],r1[2]+rr[2]);
            glTexCoord2d(1,0.5);
            glVertex3f(r1[0],r1[1]-rr[1],r1[2]+rr[2]);
            glTexCoord2d(1,1);
            glVertex3f(r1[0],r1[1]-rr[1],r1[2]-rr[2]);
            glTexCoord2d(0.5,1);
            glVertex3f(r1[0],r1[1]+rr[1],r1[2]-rr[2]);

            //Roda 2
            glNormal3f(-1, 0, 0);
            glTexCoord2d(0.5,0.5);
            glVertex3f(r2[0],r2[1]+rr[1],r2[2]+rr[2]);
            glTexCoord2d(0.5,1);
            glVertex3f(r2[0],r2[1]+rr[1],r2[2]-rr[2]);
            glTexCoord2d(1,1);
            glVertex3f(r2[0],r2[1]-rr[1],r2[2]-rr[2]);
            glTexCoord2d(1,0.5);
            glVertex3f(r2[0],r2[1]-rr[1],r2[2]+rr[2]);

            glNormal3f(1, 0, 0);
            glTexCoord2d(0.5,0.5);
            glVertex3f(r2[0],r2[1]+rr[1],r2[2]+rr[2]);
            glTexCoord2d(1,0.5);
            glVertex3f(r2[0],r2[1]-rr[1],r2[2]+rr[2]);
            glTexCoord2d(1,1);
            glVertex3f(r2[0],r2[1]-rr[1],r2[2]-rr[2]);
            glTexCoord2d(0.5,1);
            glVertex3f(r2[0],r2[1]+rr[1],r2[2]-rr[2]);

        glEnd();
        glPopMatrix();
    }

    void draw::estandarte(float scale[])
    {
        float RAD_90 = 1.570796;
        float RAD_15 = 0.261799;
        float RAD_7_5 = 0.130899;
        float RAD_3_75 = 0.0654498;
        float RAD_12_85 = 0.2242748;
        glPushMatrix();
        //glTranslatef(0.0f, 20.67f, 3.0f);
        glScalef(1.2f, 2.9f, 1.0f);
        glBegin( GL_QUADS );
        glNormal3f(0,0,1);// Start Drawing A Quad
        glTexCoord2f(0.51,0.51); glVertex3f( -16*scale[0], (-16+10.67f)*scale[1], 3.0f*scale[2]);	// Bottom Left
        glTexCoord2f(0.99,0.51); glVertex3f(  16*scale[0], (-16+10.67f)*scale[1], 3.0f*scale[2]);	// Bottom Right
        glTexCoord2f(0.99,0.99); glVertex3f(  16*scale[0],  (16+10.67f)*scale[1], 3.0f*scale[2]);	// Top Right
        glTexCoord2f(0.51,0.99); glVertex3f( -16*scale[0],  (16+10.67f)*scale[1], 3.0f*scale[2]);	// Top Left
        glEnd();
        glPopMatrix();
/*
        float materialDif[] = { 1.0, 0.65, 0.0, 1.0 };
        float materialSpe[] = { 1.0, 0.65, 0.0, 1.0 };
        glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, materialDif);
        //glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, materialSpe);
        //glMaterialf  (GL_FRONT_AND_BACK, GL_SHININESS, 0.0 );
        glDisable( GL_TEXTURE_2D );
*/
        glPushMatrix();
        //glTranslatef(0.0f, 50.67f, 0.0f);
        glScalef(0.4, 0.4, 0.4);
        //glTranslatef(0,80,0);

        float radius = 8;


        for(float j=0; j<24; j++)
        {
            glBegin( GL_TRIANGLE_STRIP );
            for(float i=0; i<=12; i++)
            {

                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j)), 0.5f - 0.5*sin(RAD_90 - RAD_7_5*i));

                glNormal3f( sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1)));
                glVertex3f( radius*sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i)*scale[0], (210.0f + radius*sin(RAD_90 + RAD_15*i))*scale[1], radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1))*scale[2] );


                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j+1)), 0.5f - 0.5*sin(RAD_90 - RAD_7_5*i));

                glNormal3f( sin(RAD_15*j)*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*j));
                glVertex3f( radius*sin(RAD_15*j)*cos(RAD_90 + RAD_15*i)*scale[0], (210.0f + radius*sin(RAD_90 + RAD_15*i))*scale[1], radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*j)*scale[2] );
            }
            glEnd();
        }

        //esfera();
        //glTranslatef(0,-8, 0);
        //glScalef(1.0f, 0.3f, 1.0f );
        //esfera();
        for(float j=0; j<24; j++)
        {
            glBegin( GL_TRIANGLE_STRIP );
            for(float i=0; i<=12; i++)
            {

                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j)), 0.5f - 0.5f*sin(RAD_90 - RAD_7_5*i));

                glNormal3f( sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1)));
                glVertex3f( radius*sin(RAD_15*(j-1))*cos(RAD_90 + RAD_15*i)*scale[0], (670.0f + radius*sinf(RAD_90 + RAD_15*i))*scale[1]*0.3, radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*(j-1))*scale[2] );


                glTexCoord2f(cos(RAD_90 - RAD_3_75*(j+1)), 0.5f - 0.5f*sin(RAD_90 - RAD_7_5*i));

                glNormal3f( sin(RAD_15*j)*cos(RAD_90 + RAD_15*i), sin(RAD_90 + RAD_15*i), cos(RAD_90 + RAD_15*i)*cos(RAD_15*j));
                glVertex3f( radius*sin(RAD_15*j)*cos(RAD_90 + RAD_15*i)*scale[0], (670.0f + radius*sinf(RAD_90 + RAD_15*i))*scale[1]*0.3, radius*cos(RAD_90 + RAD_15*i)*cos(RAD_15*j)*scale[2] );
            }
            glEnd();
        }

        glPopMatrix();
        glScalef(1.3f, 80.0f, 1.3f);
        float inc = 0.1f;
        float r = 1.0f;

        for(float i=-1.0f; i<1.0f; i = i + inc)
        {
            for(float j=0; j<24; j++)
            {
                float p1[] = { (powf(i+0,2)*sinf(RAD_15*(j+0)) + r*sinf(RAD_15*(j+0)))*scale[0], (i+0)*scale[1] ,  (powf(i+0,2)*cosf(RAD_15*(j+0)) + r*cos(RAD_15*(j+0)))*scale[2]};
                float p2[] = { (powf(i+0,2)*sinf(RAD_15*(j+1)) + r*sinf(RAD_15*(j+1)))*scale[0], (i+0)*scale[1] ,  (powf(i+0,2)*cosf(RAD_15*(j+1)) + r*cos(RAD_15*(j+1)))*scale[2]};
                float p3[] = { (powf(i+inc,2)*sinf(RAD_15*(j+0)) + r*sinf(RAD_15*(j+0)))*scale[0], (i+inc)*scale[1] ,  (powf(i+inc,2)*cosf(RAD_15*(j+0)) + r*cosf(RAD_15*(j+0)))*scale[2]};
                float p4[] = { (powf(i+inc,2)*sinf(RAD_15*(j+1)) + r*sinf(RAD_15*(j+1)))*scale[0], (i+inc)*scale[1] ,  (powf(i+inc,2)*cosf(RAD_15*(j+1)) + r*cosf(RAD_15*(j+1)))*scale[2]};

                float n1[] = { powf(i+0,2)*sin(RAD_15*(j+0)) + r*sinf(RAD_15*(j+0)), 0 ,  powf(i+0,2)*cosf(RAD_15*(j+0)) + r*cosf(RAD_15*(j+0))};
                float n2[] = { powf(i+0,2)*sinf(RAD_15*(j+1)) + r*sinf(RAD_15*(j+1)), 0 ,  powf(i+0,2)*cosf(RAD_15*(j+1)) + r*cosf(RAD_15*(j+1))};
                float n3[] = { powf(i+inc,2)*sinf(RAD_15*(j+0)) + r*sinf(RAD_15*(j+0)), inc ,  powf(i+inc,2)*cosf(RAD_15*(j+0)) + r*cosf(RAD_15*(j+0))};
                float n4[] = { powf(i+inc,2)*sinf(RAD_15*(j+1)) + r*sinf(RAD_15*(j+1)), inc ,  powf(i+inc,2)*cosf(RAD_15*(j+1)) + r*cosf(RAD_15*(j+1))};

                float t1[] = { 0.0208333f*j, 0.5f + 0.25f*(1+i)};
                float t2[] = { 0.0208333f*(j+1), 0.5f + 0.25f*(1+i)};
                float t3[] = { 0.0208333f*j, 0.5f + 0.25f*(1+i+inc)};
                float t4[] = { 0.0208333f*(j+1), 0.5f + 0.25f*(1+i+inc)};

                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2fv(t1);
                glNormal3fv(n1);
                glVertex3fv(p1);
                glTexCoord2fv(t2);
                glNormal3fv(n2);
                glVertex3fv(p2);
                glTexCoord2fv(t3);
                glNormal3fv(n3);
                glVertex3fv(p3);
                glTexCoord2fv(t4);
                glNormal3fv(n4);
                glVertex3fv(p4);
                glEnd();
            }
        }


    }

    void draw::torre(float scale[]){
                glEnable(GL_CULL_FACE);
                //pontos casa
                float p1[3] = {1*scale[0], 0, 0};
                float p2[3] = {0, 0, 0};
                float p3[3] = {0, 0, -2*scale[2]};
                float p4[3] = {1*scale[0], 0, -2*scale[2]};
                float p5[3] = {1*scale[0], 1*scale[1], 0};
                float p6[3] = {0, 1*scale[1], 0};
                float p7[3] = {0, 1*scale[1], -2*scale[2]};
                float p8[3] = {1*scale[0], 1*scale[1], -2*scale[2]};

                //pontos telhado
                float t1[3] = {1.2f*scale[0], 1*scale[1], 0};
                float t2[3] = {-0.2f*scale[0], 1*scale[1], 0};
                float t3[3] = {-0.2f*scale[0], 1*scale[1], -2*scale[2]};
                float t4[3] = {1.2f*scale[0], 1*scale[1], -2*scale[2]};
                float t5[3] = {0.5f*scale[0], 1.6f*scale[1], -1*scale[2]};
                float t6[3] = {0.5f*scale[0], 1.6f*scale[1], -1*scale[2]};

                //glEnable(GL_CULL_FACE);
                glPushMatrix();

                glBegin(GL_QUADS);
                //casa
                //frente
                    normal(p1, p5, p6);
                    glTexCoord2d(0,0);
                    glVertex3fv(p1);
                    glTexCoord2d(0,0.5);
                    glVertex3fv(p5);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p6);
                    glTexCoord2d(0.5,0);
                    glVertex3fv(p2);

                    normal(p1, p2, p6);
                    glTexCoord2d(0,0);
                    glVertex3fv(p1);
                    glTexCoord2d(0.5,0);
                    glVertex3fv(p2);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p6);
                    glTexCoord2d(0,0.5);
                    glVertex3fv(p5);
                //lado direito
                    normal(p1, p4, p8);
                    glTexCoord2d(0.5,0);
                    glVertex3fv(p1);
                    glTexCoord2d(1,0);
                    glVertex3fv(p4);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(p8);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p5);

                    normal(p1, p5, p8);
                    glTexCoord2d(0.5,0);
                    glVertex3fv(p1);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p5);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(p8);
                    glTexCoord2d(1,0);
                    glVertex3fv(p4);
                //tras
                    normal(p4, p3, p7);
                    glTexCoord2d(0,0.5);
                    glVertex3fv(p4);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p3);
                    glTexCoord2d(0.5,1);
                    glVertex3fv(p7);
                    glTexCoord2d(0,1);
                    glVertex3fv(p8);

                    normal(p4, p8, p7);
                    glTexCoord2d(0,0.5);
                    glVertex3fv(p4);
                    glTexCoord2d(0,1);
                    glVertex3fv(p8);
                    glTexCoord2d(0.5,1);
                    glVertex3fv(p7);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p3);


                    //lado esquerdo
                    normal(p3, p2, p6); //cálculo da normal da face
                    glTexCoord2d(0.5,0); //mapeamento da textura para o objeto
                    glVertex3fv(p3);
                    glTexCoord2d(1,0);
                    glVertex3fv(p2);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(p6);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p7);

                    normal(p3, p7, p6);
                    glTexCoord2d(0.5,0);
                    glVertex3fv(p3);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(p7);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(p6);
                    glTexCoord2d(1,0);
                    glVertex3fv(p2);


                //baixo
                    normal(p1, p2, p3);
                    glVertex3fv(p1);
                    glVertex3fv(p2);
                    glVertex3fv(p3);
                    glVertex3fv(p4);

                    normal(p1, p4, p3);
                    glVertex3fv(p1);
                    glVertex3fv(p4);
                    glVertex3fv(p3);
                    glVertex3fv(p2);
                //teto
                    //lado direito
                    normal(t1, t4, t6);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(t1);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(t4);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t6);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t5);

                    //lado esquerdo
                    normal(t3, t2, t5);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(t3);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(t2);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t5);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t6);

                    //baixo
                    normal(t1, t2, t3);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(t1);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(t2);
                    glTexCoord2d(1,1);
                    glVertex3fv(t3);
                    glTexCoord2d(0.5,1);
                    glVertex3fv(t4);

                glEnd();

                glBegin(GL_TRIANGLES);
                    //frente
                    normal(t2, t1, t5);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(t2);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(t1);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t5);

                    //tras
                    normal(t4, t3, t6);
                    glTexCoord2d(0.5,0.5);
                    glVertex3fv(t4);
                    glTexCoord2d(1,0.5);
                    glVertex3fv(t3);
                    glTexCoord2d(0.75,1);
                    glVertex3fv(t6);

                glEnd();
                glPopMatrix();
                glDisable(GL_CULL_FACE);
            }


    void draw::arvore(float scale[])
    {
        glEnable(GL_CULL_FACE);
        float delta = 180/3;
        float alfa;
        float R = 2;

        for(alfa=0; alfa<180; alfa=delta+alfa)
        {
            float p1[3] = {R*cos((alfa+180)*M_PI/180)*scale[0], 0, R*sinf((alfa+180)*M_PI/180)*scale[2]};
            float p2[3] = {R*cos((alfa)*M_PI/180)*scale[0], 0, R*sinf((alfa)*M_PI/180)*scale[2]};
            float p3[3] = {R*cos((alfa)*M_PI/180)*scale[0], R*scale[1], R*sinf((alfa)*M_PI/180)*scale[2]};
            float p4[3] = {R*cos((alfa+180)*M_PI/180)*scale[0], R*scale[1], R*sinf((alfa+180)*M_PI/180)*scale[2]};


            glBegin(GL_QUADS);

            //plano da arvore
            normal(p1, p2, p3);
            glTexCoord2d(0,0);
            glVertex3fv(p1);
            glTexCoord2d(1,0);
            glVertex3fv(p2);
            glTexCoord2d(1,1);
            glVertex3fv(p3);
            glTexCoord2d(0,1);
            glVertex3fv(p4);

            normal(p1, p4, p3);
            glTexCoord2d(0,0);
            glVertex3fv(p1);
            glTexCoord2d(0,1);
            glVertex3fv(p4);
            glTexCoord2d(1,1);
            glVertex3fv(p3);
            glTexCoord2d(1,0);
            glVertex3fv(p2);

            glEnd();

        }

        glDisable(GL_CULL_FACE);


    }



    void draw::desenhar( int name, float scale[3] )
    {
        switch( name )
        {
        case PLANO:
            plano(scale);
            break;
        case BLOCO:
            bloco(scale);
            break;
        case ESFERA:
            esfera(scale);
            break;
        case CASA:
            scale[0] *= 10;
            scale[1] *= 10;
            scale[2] *= 10;
            casa(scale);
            break;
        case CARROCA:
            scale[0] *= 10;
            scale[1] *= 10;
            scale[2] *= 10;
            carroca(scale);
            break;
        case ESTANDARTE:
            estandarte(scale);
            break;
        case TORRE:
            scale[0] *= 10;
            scale[1] *= 10;
            scale[2] *= 10;
            torre(scale);
            break;
        case ARVORE:
            arvore(scale);
            break;

        }

    }

