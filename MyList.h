#ifndef LISTA_H
#define LISTA_H
#include <stdio.h>
#include <stdlib.h>

template<class T> class List
{
    typedef struct listNode
    {
        int priority;
        T data;
        struct listNode *next;
    }listNode;

    protected:
    listNode *first;
    listNode *last;
    listNode *interator;
    int interatorInd;

    public:
    int size;

    List()
    {
        interatorInd = -1;
        first=NULL;
        last=NULL;
        interator = NULL;
        size=0;
    }

    int add( T v )
    {
         listNode *novo = new listNode;
         novo->next=NULL;
         novo->data = v;
         novo->priority = 0;
         interator=novo;
         interatorInd = size;
         size++;
         if(first==NULL)
         {
             first=novo;
             last=novo;
             interator=novo;
         }
         else
         {
             last->next=novo;
             last=last->next;
         }
         return size - 1;
    }

    int add( T obj, int priority)
    {
        listNode *novo = new listNode;
        novo->next=NULL;
        novo->data = obj;
        novo->priority = priority;
        interator=novo;
        interatorInd = size;
        size++;
        int count = 0;
         if(first==NULL)
         {
             first=novo;
             last=novo;
             return count;

         }
         else
         {
             listNode *frente = first;
             listNode *traz = frente;
             while( frente != NULL && frente->priority <= novo->priority )
             {
                traz = frente;
                frente = frente->next;
                count++;
             }

             if( frente == first )
             {
                 first = novo;
                 novo->next = frente;
             }
             else
             {
                 traz->next = novo;
                 novo->next = frente;
             }
             if( frente == NULL )
                last = novo;
         }
         return count;
    }

    T remove( int pos )
    {

        if(first==NULL || pos >= size || pos < 0)
            return NULL;

        size--;
        listNode *aux, *ant;

        if( pos == 0 )
        {
            aux = first;
            first = first->next;
            ant = first;
        }
        else
        {
            aux = first;

            for( int i=0; i<pos; i++ )
            {
                ant = aux;
                aux = aux->next;
            }

            ant->next = aux->next;
        }

 
        interator = first;
        if(interator == NULL)
            interatorInd = -1;
        else
            interatorInd = 0;
        

        if( aux == last )
            last = ant;

        T v = aux->data;
        delete aux;
        return v;
    }

    int search( T data )
    {
        listNode *node;
        node = first;
        int i = 0;
        while( node != NULL && node->data != data )
        {
            node = node->next;
            i++;
        }

        if( node == NULL )
            return -1;
        return i;
    }

    T getFirst()
    {
        return first->data;
    }

    T getLast()
    {
        return last->data;
    }

    int getID(int pos)
    {
        if( first == NULL || pos >= size || pos < 0 )
        {
            return NULL;
        }

        if( pos < interatorInd )
        {
            interator = first;
            interatorInd = 0;
        }

        for( int i= interatorInd ; i<pos; i++ ){
            interator = interator->next;
            interatorInd++;
        }
        return interator->priority;
    }

    T get( int pos )
    {
        if( first == NULL || pos >= size || pos < 0 )
        {
            return NULL;
        }

        if( pos < interatorInd )
        {
            interator = first;
            interatorInd = 0;
        }

        for( int i= interatorInd ; i<pos; i++ ){
            interator = interator->next;
            interatorInd++;
        }

        return interator->data;
    }

    T getByID(int id)
    {
        if(first == NULL)
            return NULL;

        for( int i=0; i<size; i++ )
            if(getID(i) == id)
                return get(i);

        return NULL;
    }

    bool empty( )
    {
        if( first == NULL )
            return true;
        else
            return false;
    }


    //Salva os elementos de cena da lista.
    void saveList( FILE *f )
    {
        fwrite( &size,  1,sizeof(int), f );
        for( int i = 0; i<size; i++ )
        {
            //Garantia. Se nao ha mais elemento a ser checado na lista, saia
            if( get(i) == NULL )
                break;
            int ind = getID(i);
            fwrite( &ind, 1, sizeof(int),f  );
            fwrite( get(i), 1, sizeof(T),f  );
        }
    }
    List<T> *loadList( FILE *f )
    {

        List<T> *lista = new List<T>();
        int size;
        int id;
        fread( &size,1,sizeof(size), f );

        for( int i = 0; i<size; i++ )
        {
            T element = (T)malloc(sizeof(T));
            fread(&id, 1, sizeof(int), f);
            fread(element, 1, sizeof(T), f);
            lista->adicionar(element, id);
        }
        lista->size = size;
        return lista;
    }

    //Salva os elementos de cena da lista.
    void saveList( const char *filename )
    {
        FILE *f;
        f = fopen( filename, "wb" );

        fwrite( &size,  1,sizeof(int), f );
        for( int i = 0; i<size; i++ )
        {
            //Garantia. Se nao ha mais elemento a ser checado na lista, saia
            if( get(i) == NULL )
                break;
            int ind = getID(i);
            fwrite( &ind, 1, sizeof(int),f  );
            fwrite( get(i), 1, sizeof(T),f  );
        }
        fclose(f);
    }

    List<T> *loadList( const char* filename )
    {
        FILE *f;
        int id;
        f = fopen(filename, "rb");

        if(f == NULL)
            return NULL;

        List<T> *lista = new List<T>();
        int size;
        fread( &size,1,sizeof(size), f );

        for( int i = 0; i<size; i++ )
        {
            T element = (T)malloc(sizeof(T));
            fread(&id, 1, sizeof(int), f);
            fread(element, 1, sizeof(T), f);
            lista->adicionar(element, id);
        }
        fclose(f);
        lista->size = size;
        return lista;
    }


    void free()
    {
        while( !this->empty() )
        {
            remove(0);
        }
    }

    void deleteAll()
    {
        while( !this->empty() )
        {
            delete remove(0);
        }
    }

};


#endif // LISTA_H
