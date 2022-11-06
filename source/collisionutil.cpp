#include "collisionutil.h"

#include <cmath>

namespace unboxing_engine {

void CCollisionManager::RegisterMesh(const CMeshBuffer &sceneNode, const SceneComposite &sceneComposite) {
    mMeshes.try_emplace(sceneComposite.id, sceneNode);
    mSceneComposite.try_emplace(sceneComposite.id, sceneComposite);
}

bool collut::rayWithObject(vector3D point, vector3D ray, int objectID, vector3D &intersection, face &poly) {


    octree *oct = objects.getByID(objectID);
    List<vector3D> intersecList;
    List<face> intersecFaceList;

    if (oct == NULL) {
        intersection = vector3D(99999999, 99999999, 99999999);
        return false;
    }


    List<branch> galhos;
    vector3D intersecbox;
    if (oct->root->box.intersectionWithRay(point, ray, intersecbox))
        galhos.add(oct->root);


    while (!galhos.empty()) {
        branch *u = galhos.remove(0);
        for (int i = 0; i < 8; i++) {
            if (u->child[i] != NULL && u->child[i]->box.intersectionWithRay(point, ray, intersecbox))
                galhos.add(u->child[i]);
        }


        for (int i = 0; i < u->nfaces; i++) {
            meshBuffer *mb = mesh.getByID(objectID);//oct->mesh;

            //if(mb == NULL)

            float *v1, *v2, *v3, *v4;
            v1 = mb->vertices[u->faces[i]->vertex[0]].coord;
            v2 = mb->vertices[u->faces[i]->vertex[1]].coord;
            v3 = mb->vertices[u->faces[i]->vertex[2]].coord;
            //v4 = mb->vertices[u->faces[i]->vertex[3]].coord;


            vector3D vert1(v1[0], v1[1], v1[2]);
            vector3D vert2(v2[0], v2[1], v2[2]);
            vector3D vert3(v3[0], v3[1], v3[2]);
            //vector3D vert4(v4[0], v4[1], v4[2]);


            vector3D normal = (vert2 - vert1).vetorial(vert3 - vert1).normalizado();
            vector3D ponto = vert1;

            //Coeficientes da equação do plano
            double a = normal.x;
            double b = normal.y;
            double c = normal.z;
            double d = -(a * ponto.x + b * ponto.y + c * ponto.z);

            //Parametro t calculado a aprtir da equação do plano
            double t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

            vector3D tempIntersec = point + t * ray;


            if (isInsideTriangle(vert1, vert2, vert3, tempIntersec)) {
                vector3D *intersection = new vector3D(tempIntersec.x, tempIntersec.y, tempIntersec.z);
                intersecList.add(intersection, ((int) t * 1000000));
                intersecFaceList.add(u->faces[i], ((int) t * 1000000));
            }


            //            if( u->faces[i]->n == 4 && isInsideTriangle(vert3, vert4, vert1, tempIntersec) )
            //            {
            //                vector3D *intersection = new vector3D(tempIntersec.x, tempIntersec.y, tempIntersec.z);
            //                intersecList.add(intersection, ((int)t*1000000) );
            //                intersecFaceList.add(u->faces[i], ((int)t*1000000) );
            //            }
        }
    }


    if (!intersecList.empty()) {
        intersection = *intersecList.get(0);
        intersecList.deleteAll();
        poly = *intersecFaceList.get(0);
        intersecFaceList.free();
        return true;
    }

    return false;
}
void collut::rayWithAllObjects(vector3D point, vector3D ray, List<int> &objectsID, List<vector3D> &intersections, List<face> &faces) {
}

bool collut::rayWithTriangle(vector3D point, vector3D ray, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection) {

    vector3D normal = (v2 - v1).vetorial(v3 - v1).normalizado();
    vector3D ponto = v1;

    //Coeficientes da equação do plano
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    double d = -(a * ponto.x + b * ponto.y + c * ponto.z);

    //Parametro t calculado a aprtir da equação do plano
    double t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

    vector3D tempIntersec = point + t * ray;

    if (isInsideTriangle(v1, v2, v3, tempIntersec)) {
        intersection = tempIntersec;
        return true;
    }

    return false;
}
//Fazer a inversão e pegar através do ID. Fazer retornar a normal e os trés vertices do triangulo
bool collut::betweenObjects(int id1, int id2, int parameter, List<vector3D> &intersections, List<vector3D> &triangles) {
    SceneElement *e1 = SEObjects.getByID(id1);
    SceneElement *e2 = SEObjects.getByID(id2);
    bool coll = false;
    vector3D intersection;
    if (parameter == NOSIMPLE_SIMPLE && (e1->getType() == NOSIMPLE && e2->getType() == SIMPLE || e1->getType() == SIMPLE && e2->getType() == NOSIMPLE)) {
        if (e1->getType() == SIMPLE) {
            SceneElement *e = e1;
            e1 = e2;
            e2 = e;
        }


        octree *oct = objects.getByID(e1->getID());
        List<vector3D> intersecList;
        List<face> intersecFaceList;

        if (oct == NULL) {
            intersection = vector3D(99999999, 99999999, 99999999);
            return false;
        }

        sphere reg = (sphere) e2->reg;
        reg.position = e1->getMatrix().inverse() * reg.position;//Checar se essa é a maneira certa de colocar o vetor no sistema de referencia do outro objeto

        List<branch> galhos;
        vector3D intersecbox;
        if (oct->root->box.intersectionWithSphere(reg)) {
            galhos.add(oct->root);
        }


        while (!galhos.empty()) {
            branch *u = galhos.remove(0);
            for (int i = 0; i < 8; i++) {
                if (u->child[i] != NULL && u->child[i]->box.intersectionWithSphere(reg)) {
                    galhos.add(u->child[i]);
                    //printf("entrou\n");
                }
            }

            for (int i = 0; i < u->nfaces; i++) {

                meshBuffer *mb = mesh.getByID(e1->getID());

                //face *f = u->faces[i];

                vector3D v1(mb->vertices[u->faces[i]->vertex[0]].coord);
                vector3D v2(mb->vertices[u->faces[i]->vertex[1]].coord);
                vector3D v3(mb->vertices[u->faces[i]->vertex[2]].coord);
                //printf("%d\n", i);
                //                glColor3f(1,0,0);
                //                glBegin(GL_TRIANGLES);
                //                glVertex3f(v1.x, v1.y, v1.z);
                //                glVertex3f(v2.x, v2.y, v2.z);
                //                glVertex3f(v3.x, v3.y, v3.z);
                //                glEnd();
                //                u->box.drawWireframe();


                if (sphereWithTriangle(reg, v1, v2, v3, intersection)) {
                    intersections.add(new vector3D(e1->getMatrix() * intersection));
                    triangles.add(new vector3D(v1));
                    triangles.add(new vector3D(v2));
                    triangles.add(new vector3D(v3));
                    coll = true;
                }
            }


            //printf("(%f,%f,%f)\n",  e1->getPosition().x,  e1->getPosition().y,  e1->getPosition().z);
            //        for(int i=0; i<mb->nfaces; i++ )
            //        {
            //            vector3D v1(mb->vertices[ mb->faces[i].vertex[0] ].coord );
            //            vector3D v2(mb->vertices[ mb->faces[i].vertex[1] ].coord );
            //            vector3D v3(mb->vertices[ mb->faces[i].vertex[2] ].coord );
            //            //printf("%d\n", i);
            //
            //            if( sphereWithTriangle(reg, v1,v2,v3, intersection) )
            //            {
            //                intersections.add(new vector3D(e1->getMatrix()*intersection));
            //                triangles.add(new vector3D(v1));
            //                triangles.add(new vector3D(v2));
            //                triangles.add(new vector3D(v3));
            //                coll = true;
            //            }
            //        }
        }
    }

    if (parameter == SIMPLE_SIMPLE && e1->getType() == SIMPLE && e2->getType() == SIMPLE) {
        sphere reg1 = e1->reg;
        sphere reg2 = e2->reg;
        if ((reg1.position - reg2.position).modulo() <= reg1.radius + reg2.radius) {
            intersections.add(new vector3D((reg2.position - reg1.position).normalizado() * reg1.radius + reg1.position));
            return true;
        }
    }
    return coll;
}


bool collut::isInsideTriangle(vector3D v1, vector3D v2, vector3D v3, vector3D intersection) {
    //Checa se a coordenada está limitada pelo triangulo
    double area = (v2 - v1).vetorial(v3 - v1).modulo() * 0.5;
    vector3D vet1 = intersection - v1;
    vector3D vet2 = v2 - v1;
    double area1 = vet1.vetorial(vet2).modulo() * 0.5 / area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    double area2 = vet1.vetorial(vet2).modulo() * 0.5 / area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    double area3 = vet1.vetorial(vet2).modulo() * 0.5 / area;

    //Condição para que o ponto esteja contido no triangulo
    if (area1 + area2 + area3 <= 1.00001)
        return true;

    return false;
}

bool collut::sphereWithTriangle(sphere spr, vector3D v1, vector3D v2, vector3D v3, vector3D &intersection) {
    vector3D normal = (v2 - v1).vetorial(v3 - v1).normalizado();
    float a = normal.x;
    float b = normal.y;
    float c = normal.z;

    float d = -a * v1.x - b * v1.y - c * v1.z;

    vector3D p = spr.position;

    float lambda = (-d - p.escalar(normal)) / pow(normal.modulo(), 2.0);

    //Projeção ortogonal do centro da esfera ao plano.
    vector3D projecao = p + normal * lambda;

    if (spr.isInside(projecao)) {
        if (isInsideTriangle(v1, v2, v3, projecao)) {
            intersection = projecao;
            return true;
        }
    }

    //    if( spr.intersectionWithLine(v1, v2, intersection) )
    //        return true;
    //    if( spr.intersectionWithLine(v2, v3, intersection) )
    //        return true;
    //    if( spr.intersectionWithLine(v1, v3, intersection) )
    //        return true;
    //
    //    if( spr.isInside( v1) )
    //    {
    //        intersection = v1;
    //        return true;
    //    }
    //    if( spr.isInside( v2 ) )
    //    {
    //        intersection = v2;
    //        return true;
    //    }
    //    if( spr.isInside( v3 ) )
    //    {
    //        intersection = v3;
    //        return true;
    //    }


    return false;
}
}// namespace unboxing_engine