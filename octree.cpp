#include "octree.h"

octree::octree( SceneElement *e )
{
    transformation = &e->getMatrix();
    mesh = e->getMesh();
    List<face> faces;
    for(int i=0; i<mesh->nfaces; i++)
    {
        faces.add(&mesh->faces[i]);
    }

    BoundingBox box = mesh->boundingBox;
    root = new branch( box.getCenter(), box.getMax(), box.getMin() );

    createOctree( mesh->vertices, &faces, root );


}

octree::octree( meshBuffer *mesh )
{
    List<face> faces;
    for(int i=0; i<mesh->nfaces; i++)
    {
        faces.add(&mesh->faces[i]);
    }

    BoundingBox box = mesh->boundingBox;
    root = new branch( box.getCenter(), box.getMax(), box.getMin() );

    createOctree( mesh->vertices, &faces, root );
}

void octree::createOctree( vertexcoord vertices[], List<face> *faces, branch *root )
{
    if( faces->size < MAX_FACES )
    {
        root->nfaces = faces->size;
        for(int i=0; i<faces->size; i++)
            root->faces[i] = faces->get(i);
        return;
    }

    root->subdivideOct();

    List<face> oct[8];
    for(int i=0; i<faces->size; i++)
    {
        vector3D vertice[4];
        for(uint j=0; j<faces->get(i)->n; j++)
        {
            vertice[j].x = vertices[faces->get(i)->vertex[j]].coord[0];
            vertice[j].y = vertices[faces->get(i)->vertex[j]].coord[1];
            vertice[j].z = vertices[faces->get(i)->vertex[j]].coord[2];

        }

        for( int j=0; j<8; j++ )
        for( uint k=0; k < faces->get(i)->n; k++)
        {
            if( isInsideOct(vertice[k], root->child[j]) )
            {
                oct[j].add(faces->get(i));
                break;
            }
        }

    }


    for(int i=0; i<8; i++)
    {
        if(oct[i].size > 0)
        {
            createOctree(vertices, &oct[i], root->child[i]);
        }
        else
        {
            delete root->child[i];
            root->child[i] = NULL;
        }
        oct[i].free();
    }

}

bool octree::isInsideOct( vector3D vertice, branch *oct )
{
    if( oct->box.isInside(vertice) )
        return true;

    return false;
}

branch::branch(vector3D center, vector3D max, vector3D min)
{
    this->box = BoundingBox(center, max, min);
    this->nfaces = 0;
    for(int i=0; i<8; i++)
    {
        child[i] = NULL;
    }
}

void branch::subdivideOct()
{
    float xsize, ysize, zsize;
    vector3D omphalos, maximum, minimum;
    vector3D center = box.getCenter();
    vector3D max = box.getMax();
    vector3D min = box.getMin();

    //Criaçao do primeiro octante
    xsize = max.x - center.x;
    ysize = max.y - center.y;
    zsize = max.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(xsize/2, ysize/2, zsize/2);
    minimum = omphalos - vector3D(xsize/2, ysize/2, zsize/2);
    child[0] = new branch(omphalos, maximum, minimum);

    //Criação do segundo octante
    xsize = max.x - center.x;
    ysize = max.y - center.y;
    zsize = min.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(xsize/2, ysize/2, -zsize/2);
    minimum = omphalos - vector3D(xsize/2, ysize/2, -zsize/2);
    child[1] = new branch(omphalos, maximum, minimum);

    //Criação do terceiro octante
    xsize = min.x - center.x;
    ysize = max.y - center.y;
    zsize = min.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(-xsize/2, ysize/2, -zsize/2);
    minimum = omphalos - vector3D(-xsize/2, ysize/2, -zsize/2);
    child[2] = new branch(omphalos, maximum, minimum);

    //Criação do quarto octante
    xsize = min.x - center.x;
    ysize = max.y - center.y;
    zsize = max.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(-xsize/2, ysize/2, zsize/2);
    minimum = omphalos - vector3D(-xsize/2, ysize/2, zsize/2);
    child[3] = new branch(omphalos, maximum, minimum);

    //Criação do quinto octante
    xsize = max.x - center.x;
    ysize = min.y - center.y;
    zsize = max.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(xsize/2, -ysize/2, zsize/2);
    minimum = omphalos - vector3D(xsize/2, -ysize/2, zsize/2);
    child[4] = new branch(omphalos, maximum, minimum);

    //Criação do sexto octante
    xsize = max.x - center.x;
    ysize = min.y - center.y;
    zsize = min.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(xsize/2, -ysize/2, -zsize/2);
    minimum = omphalos - vector3D(xsize/2, -ysize/2, -zsize/2);
    child[5] = new branch(omphalos, maximum, minimum);

    //Criação do setimo octante
    xsize = min.x - center.x;
    ysize = min.y - center.y;
    zsize = min.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(-xsize/2, -ysize/2, -zsize/2);
    minimum = omphalos - vector3D(-xsize/2, -ysize/2, -zsize/2);
    child[6] = new branch(omphalos, maximum, minimum);

    //Criação do oitavo octante
    xsize = min.x - center.x;
    ysize = min.y - center.y;
    zsize = max.z - center.z;
    omphalos = center + vector3D(xsize/2, ysize/2, zsize/2);
    maximum = omphalos + vector3D(-xsize/2, -ysize/2, zsize/2);
    minimum = omphalos - vector3D(-xsize/2, -ysize/2, zsize/2);
    child[7] = new branch(omphalos, maximum, minimum);
}

void octree::draw(meshBuffer* mesh)
{
    List<branch> l;

    l.add(root);



    while( !l.empty() )
    {
        branch *u = l.remove(0);

        for( int j=0; j<8; j++ )
        {
            if( u->child[j] != NULL )
                l.add( u->child[j] );

        }
        u->box.drawWireframe();
        for( int i=0; i< u->nfaces; i++ )
        {
            face *f = u->faces[i];
            glBegin(GL_TRIANGLES);

            for( int j=0; j<f->n; j++ )
                glVertex3fv( mesh->vertices[ f->vertex[j]].coord );

            glEnd();
        }
    }
}
