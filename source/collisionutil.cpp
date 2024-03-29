#include "collisionutil.h"
#include "internal_components/IColliderComponent.h"

#include <cmath>

namespace unboxing_engine {

void CCollisionManager::RegisterMesh(const CMeshBuffer &sceneNode, const SceneComposite &sceneComposite) {
    mMeshes.try_emplace(sceneComposite.id, sceneNode);
    mSceneComposite.try_emplace(sceneComposite.id, sceneComposite);
}

bool CCollisionManager::rayWithObject(const Vector3f& point, const Vector3f& ray, int objectID, Vector3f &intersection, face &poly) {


    auto oct = mSceneComposite.find(objectID);
    std::vector<Vector3f> intersecList;
    std::vector<face> intersecFaceList;

    if (oct == mSceneComposite.end()) {
        return false;
    }


    std::vector<branch> galhos;
    Vector3f intersecbox;
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


            Vector3f vert1(v1[0], v1[1], v1[2]);
            Vector3f vert2(v2[0], v2[1], v2[2]);
            Vector3f vert3(v3[0], v3[1], v3[2]);
            //Vector3f vert4(v4[0], v4[1], v4[2]);


            Vector3f normal = (vert2 - vert1).CrossProduct(vert3 - vert1).Normalized();
            Vector3f ponto = vert1;

            //Coeficientes da equação do plano
            double a = normal.x;
            double b = normal.y;
            double c = normal.z;
            double d = -(a * ponto.x + b * ponto.y + c * ponto.z);

            //Parametro t calculado a aprtir da equação do plano
            double t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

            Vector3f tempIntersec = point + t * ray;


            if (isInsideTriangle(vert1, vert2, vert3, tempIntersec)) {
                Vector3f *intersection = new Vector3f(tempIntersec.x, tempIntersec.y, tempIntersec.z);
                intersecList.add(intersection, ((int) t * 1000000));
                intersecFaceList.add(u->faces[i], ((int) t * 1000000));
            }


            //            if( u->faces[i]->n == 4 && isInsideTriangle(vert3, vert4, vert1, tempIntersec) )
            //            {
            //                Vector3f *intersection = new Vector3f(tempIntersec.x, tempIntersec.y, tempIntersec.z);
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
void CCollisionManager::rayWithAllObjects(Vector3f point, Vector3f ray, List<int> &objectsID, List<Vector3f> &intersections, List<face> &faces) {
}

bool CCollisionManager::rayWithTriangle(Vector3f point, Vector3f ray, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f &intersection) {

    Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();
    Vector3f ponto = v1;

    //Coeficientes da equação do plano
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    double d = -(a * ponto.x + b * ponto.y + c * ponto.z);

    //Parametro t calculado a aprtir da equação do plano
    double t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

    Vector3f tempIntersec = point + t * ray;

    if (isInsideTriangle(v1, v2, v3, tempIntersec)) {
        intersection = tempIntersec;
        return true;
    }

    return false;
}
//Fazer a inversão e pegar através do ID. Fazer retornar a normal e os trés vertices do triangulo
bool CCollisionManager::betweenObjects(int id1, int id2, int parameter, std::vector<Vector3f> &intersections, std::vector<Vector3f> &triangles) {
    auto e1 = mSceneComposite.find(id1);
    auto e2 = mSceneComposite.find(id2);
    if (e1 == mSceneComposite.end() || e2 == mSceneComposite.end()) {
        return false;
    }

    bool coll = false;
    Vector3f intersection;
    if (parameter == NOSIMPLE_SIMPLE && (e1->getType() == NOSIMPLE && e2->getType() == SIMPLE || e1->getType() == SIMPLE && e2->getType() == NOSIMPLE)) {
        if (e1->getType() == SIMPLE) {
            SceneElement *e = e1;
            e1 = e2;
            e2 = e;
        }


        auto oct = e1->second.GetComponent<IColliderComponent>();
        std::vector<Vector3f> intersecList;
        std::vector<face> intersecFaceList;

        if (oct == nullptr) {
            return false;
        }

        sphere reg = (sphere) e2->reg;
        reg.position = e1->getMatrix().inverse() * reg.position;//Checar se essa é a maneira certa de colocar o vetor no sistema de referencia do outro objeto

        std::vector<branch> galhos;
        Vector3f intersecbox;
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

                Vector3f v1(mb->vertices[u->faces[i]->vertex[0]].coord);
                Vector3f v2(mb->vertices[u->faces[i]->vertex[1]].coord);
                Vector3f v3(mb->vertices[u->faces[i]->vertex[2]].coord);
                //printf("%d\n", i);
                //                glColor3f(1,0,0);
                //                glBegin(GL_TRIANGLES);
                //                glVertex3f(v1.x, v1.y, v1.z);
                //                glVertex3f(v2.x, v2.y, v2.z);
                //                glVertex3f(v3.x, v3.y, v3.z);
                //                glEnd();
                //                u->box.drawWireframe();


                if (sphereWithTriangle(reg, v1, v2, v3, intersection)) {
                    intersections.add(new Vector3f(e1->getMatrix() * intersection));
                    triangles.add(new Vector3f(v1));
                    triangles.add(new Vector3f(v2));
                    triangles.add(new Vector3f(v3));
                    coll = true;
                }
            }


            //printf("(%f,%f,%f)\n",  e1->getPosition().x,  e1->getPosition().y,  e1->getPosition().z);
            //        for(int i=0; i<mb->nfaces; i++ )
            //        {
            //            Vector3f v1(mb->vertices[ mb->faces[i].vertex[0] ].coord );
            //            Vector3f v2(mb->vertices[ mb->faces[i].vertex[1] ].coord );
            //            Vector3f v3(mb->vertices[ mb->faces[i].vertex[2] ].coord );
            //            //printf("%d\n", i);
            //
            //            if( sphereWithTriangle(reg, v1,v2,v3, intersection) )
            //            {
            //                intersections.add(new Vector3f(e1->getMatrix()*intersection));
            //                triangles.add(new Vector3f(v1));
            //                triangles.add(new Vector3f(v2));
            //                triangles.add(new Vector3f(v3));
            //                coll = true;
            //            }
            //        }
        }
    }

    if (parameter == SIMPLE_SIMPLE && e1->getType() == SIMPLE && e2->getType() == SIMPLE) {
        sphere reg1 = e1->reg;
        sphere reg2 = e2->reg;
        if ((reg1.position - reg2.position).modulo() <= reg1.radius + reg2.radius) {
            intersections.add(new Vector3f((reg2.position - reg1.position).Normalized() * reg1.radius + reg1.position));
            return true;
        }
    }
    return coll;
}


bool CCollisionManager::isInsideTriangle(Vector3f v1, Vector3f v2, Vector3f v3, Vector3f intersection) {
    //Checa se a coordenada está limitada pelo triangulo
    double area = (v2 - v1).CrossProduct(v3 - v1).modulo() * 0.5;
    Vector3f vet1 = intersection - v1;
    Vector3f vet2 = v2 - v1;
    double area1 = vet1.CrossProduct(vet2).modulo() * 0.5 / area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    double area2 = vet1.CrossProduct(vet2).modulo() * 0.5 / area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    double area3 = vet1.CrossProduct(vet2).modulo() * 0.5 / area;

    //Condição para que o ponto esteja contido no triangulo
    if (area1 + area2 + area3 <= 1.00001)
        return true;

    return false;
}

bool CCollisionManager::sphereWithTriangle(sphere spr, Vector3f v1, Vector3f v2, Vector3f v3, Vector3f &intersection) {
    Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();
    float a = normal.x;
    float b = normal.y;
    float c = normal.z;

    float d = -a * v1.x - b * v1.y - c * v1.z;

    Vector3f p = spr.position;

    float lambda = (-d - p.escalar(normal)) / pow(normal.modulo(), 2.0);

    //Projeção ortogonal do centro da esfera ao plano.
    Vector3f projecao = p + normal * lambda;

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



bool BoundingBox::isInside(const Vector3f &point) {
    Vector3f v0 = vertices[faces[1][0]] - vertices[faces[0][0]];
    Vector3f v1 = vertices[faces[0][1]] - vertices[faces[0][0]];
    Vector3f v2 = vertices[faces[0][3]] - vertices[faces[0][0]];
    Vector3f v3 = v1.CrossProduct(v2);
    float volume = v3.Normalized().DotProduct(v0) * v3.Length();
    float soma = 0;

    for (auto &face: faces) {
        Vector3f normal = (vertices[face[1]] - vertices[face[0]]).CrossProduct(vertices[face[3]] - vertices[face[0]]).Normalized();
        float altura = (point - vertices[face[0]]).DotProduct(normal);
        float base_area = (vertices[face[1]] - vertices[face[0]]).CrossProduct(vertices[face[3]] - vertices[face[0]]).Length();
        float v_piramide = altura * base_area / 3;

        soma += modulo(v_piramide);
    }

    if (modulo(soma) - modulo(volume) > 0.0001)
        return false;

    return true;
}



bool BoundingBox::intersectionWithSphere(sphere esf) {

    for (int i = 0; i < 6; i++) {
        Vector3f v1 = vertices[faces[i][0]];
        Vector3f v2 = vertices[faces[i][1]];
        Vector3f v3 = vertices[faces[i][2]];
        Vector3f v4 = vertices[faces[i][3]];

        Vector3f intersection;
        if (CCollisionManager::sphereWithTriangle(esf, v1, v2, v3, intersection)) {
            return true;
        }
        if (CCollisionManager::sphereWithTriangle(esf, v3, v4, v1, intersection)) {
            return true;
        }
        if (this->isInside(esf.position)) {
            return true;
        }
    }
    return false;
}

}// namespace unboxing_engine