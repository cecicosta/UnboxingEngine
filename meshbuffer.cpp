#include "meshbuffer.h"

meshBuffer *meshBuffer::createMeshBufferOBJ( GLMmodel *model )
{
    if(model == NULL)
    {
        printf("Erro ao converter o modelo!\n");
        return NULL;
    }
    int vId = 0;
    int mId = 0;
    int fId = 0;
    int j = 0;
    meshBuffer *mb = new meshBuffer();
    static GLMgroup* group;
    static GLMtriangle* triangle;
    static GLMmaterial* m;
    mb->vertices = new vertexcoord[model->numvertices+1];
    mb->normals = new normalcoord[model->numnormals+1];
    mb->texcoords   = new texcoord[model->numtexcoords+1];
    mb->faces = new face[model->numtriangles+1];
    mb->materials  = new material[model->nummaterials+1];
    mb->nvertices = model->numvertices+1;
    mb->nnormals = model->numnormals+1;
    mb->nfaces = model->numtriangles+1;
    mb->nmaterials = model->nummaterials+1;
    mb->ntexcoords = model->numtexcoords+1;


    vector3D max(-99999999.0f,-99999999.0f,-99999999.0f);
    vector3D min( 99999999.0f, 99999999.0f, 99999999.0f);

    vertexcoord *vertex = new vertexcoord;
    for(int i=0; i<mb->nvertices; i++)
    {
        float *vert = &model->vertices[3*i];
        vertex->id = i;
        vertex->coord[0] = vert[0];
        vertex->coord[1] = vert[1];
        vertex->coord[2] = vert[2];
        mb->vertices[i] = *vertex;

        if( vert[0] > max.x )
            max.x = vert[0];
        if( vert[0] < min.x )
            min.x = vert[0];

        if( vert[1] > max.y )
            max.y = vert[1];
        if( vert[1] < min.y )
            min.y = vert[1];

        if( vert[2] > max.z )
            max.z = vert[2];
        if( vert[2] < min.z )
            min.z = vert[2];
    }
    vector3D center = min + (max - min)/2;
    mb->boundingBox = BoundingBox( center, max, min );
    delete vertex;

    normalcoord *normal = new normalcoord;
    for(int i=0; i<mb->nnormals; i++)
    {
        float *vertnorm = &model->normals[3*i];
        if( vertnorm != NULL )
        {
            normal->id = i;
            normal->coord[0] = vertnorm[0];
            normal->coord[1] = vertnorm[1];
            normal->coord[2] = vertnorm[2];
            mb->normals[i] = *normal;

        }
    }
    delete normal;

    texcoord *texUV = new texcoord;
    for(int i=0; i<mb->ntexcoords; i++)
    {
        float *tcoord = &model->texcoords[2*i];
        if( tcoord != NULL )
        {
            texUV->id = i;
            texUV->coord[0] = tcoord[0];
            texUV->coord[1] = tcoord[1];
            mb->texcoords[i] = *texUV;
        }
    }
    delete texUV;

    for(int i=0; i<mb->nmaterials; i++)
    {
        m = &model->materials[i];
        material *mat = new material( m->diffuse, m->specular, m->ambient, m->emmissive, m->shininess );
        mb->materials[mId++] = *mat;
        delete mat;
    }


    //printf( "Vertice num: %d\n Triangles num: %d\nMaterial num: %d\n", model->numvertices, model->numtriangles, model->nummaterials );
    group = model->groups;
    while (group) {
        //printf("Group: %d\nTriangle num:%d\n", j++, group->numtriangles);
        //printf("teste: %d\n", group->numtriangles);

        for (int i = 0; i < group->numtriangles; i++) {
//            printf("Triangle: %d\n", i);
//            printf("Face: %d\n", fId);

            //printf("index: %d\n", group->triangles[i]);
            triangle = &model->triangles[group->triangles[i]];
            float *norm = &model->facetnorms[3 * triangle->findex];

            face *tri = new face;
            tri->normal[0] = norm[0];
            tri->normal[1] = norm[1];
            tri->normal[2] = norm[2];
            tri->color[0] = 0;
            tri->color[1] = 0;
            tri->color[2] = 0;
            tri->n = 3;
            tri->hasChecked = false;
            tri->matID = group->material;
            tri->id = fId;
            //Referencia os vertices a face
            tri->vertex[0] = triangle->vindices[0];
            tri->vertex[1] = triangle->vindices[1];
            tri->vertex[2] = triangle->vindices[2];
            //Referencia uma normal para cada vertice
            tri->vnormal[0] = triangle->nindices[0];
            tri->vnormal[1] = triangle->nindices[1];
            tri->vnormal[2] = triangle->nindices[2];
            //Referencia uma coordenada UV para cada vertice
            tri->texUV[0] = triangle->tindices[0];
            tri->texUV[1] = triangle->tindices[1];
            tri->texUV[3] = triangle->tindices[2];

            mb->faces[fId++] = *tri;
            delete tri;

        }
        group = group->next;
    }
    return mb;
}

void meshBuffer::draw()
{
    glBegin( GL_TRIANGLES );
    for(int i=0; i<nfaces; i++)
    {
//        vector3D v1 = trans*vector3D(vertices[faces[i].vertex[0]].coord);
//        vector3D v2 = trans*vector3D(vertices[faces[i].vertex[1]].coord);
//        vector3D v3 = trans*vector3D(vertices[faces[i].vertex[2]].coord);
//        glVertex3f(v1.x,v1.y,v1.z);
//        glVertex3f(v2.x,v2.y,v2.z);
//        glVertex3f(v3.x,v3.y,v3.z);
        float *tcoord = texcoords[ faces[i].texUV[0] ].coord;
        glTexCoord2f(tcoord[0], tcoord[1]);

        glVertex3fv(vertices[faces[i].vertex[0]].coord);

        tcoord = texcoords[ faces[i].texUV[1] ].coord;
        glTexCoord2f(tcoord[0], tcoord[1]);

        glVertex3fv(vertices[faces[i].vertex[1]].coord);

        tcoord = texcoords[ faces[i].texUV[3] ].coord;
        glTexCoord2f(tcoord[0], tcoord[1]);

        glVertex3fv(vertices[faces[i].vertex[2]].coord);
        //printf("texn: %d\n", ntexcoords);
        //printf("tex: %d\n", faces[i].texUV[3]);

    }
     glEnd();

}
