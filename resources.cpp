#include "resources.h"


List<Texture> Resources::textures;
List<meshBuffer> Resources::models;
List<int> Resources::fonts;

bool Resources::loadResource(char *name, int type, int &id)
{
    FILE *g;
    Texture *tex, *glTex;
    GLMmodel *model;
    switch(type)
    {
        case TEXTURE:
            g = fopen(name, "r");
            if(g == NULL)
            {
                fclose(g);
                FILE *f;
                f = fopen("debug.txt", "a");
                fprintf( f, "Arquivo '%s' não pode ser carregado\n", name );
                fclose(f);
                return false;
            }
            else
            {
                fclose(g);
                tex = GLAux::LoadTexture(name);
                id = textures.add( tex );
            }
        break;
        case MODEL:
            g = fopen(name, "r");

            if(g == NULL)
            {
                fclose(g);
                FILE *f;
                f = fopen("debug.txt", "a");
                fprintf( f, "Arquivo '%s' não pode ser carregado\n", name );
                fclose(f);
                return false;
            }
            else
            {
                fclose(g);
                model = glmReadOBJ(name);
                glmFacetNormals(model);
                glmVertexNormals(model, 90);
                meshBuffer *mb = meshBuffer::createMeshBufferOBJ( model );
                id = models.add( mb );
            }
        break;
        case FONT:
        break;
    }
    FILE *f;
    f = fopen("debug.txt", "a");
    fprintf( f, "Arquivo '%s' carregado\n", name );
    fclose(f);

    return true;
}

void *Resources::getResource(int type, int id)
{
    switch(type)
    {
        case TEXTURE:
            if(id>=textures.size || id<0)
            {
                printf("Textura %d nao existe na lista\n", id);
                return NULL;
            }
            return textures.get(id);
        case MODEL:


            if(id>=models.size || id<0)
            {
                printf("Objeto %d nao existe na lista\n", id);
                return NULL;
            }
            return models.get(id);
        case FONT:
            if(id>=fonts.size || id<0)
            {
                printf("Font %d nao existe na lista\n", id);
                return NULL;
            }
            return fonts.get(id);
    }
    return NULL;
}

