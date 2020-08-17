#ifndef __LISTAS_H__
#define __LISTAS_H__

struct nodo {
   void *direc;
   char h[50];
   int tam;
   int jkr;
   char nombre[50];
   struct nodo *sig;
};
typedef struct nodo *posicion;
typedef struct nodo *lista;

static struct nodo *crearnodo();

lista crearlista();

int eslistavacia(lista l);

void insertar(void *x, posicion p);

void insertarhora(char hora[], posicion p);

void insertartamano(int t, posicion p);

void insertarnombre(char nombre[], posicion p);

void insertarjkr(int jkr, posicion p);

posicion buscar(lista l, void *e, int (*comp)(const void *x, const void *y));

posicion buscartamano(lista l, int tamano);

posicion buscarnombre(lista l, char *nombre);

static posicion buscarant(lista l, void *x, int (*comp)(const void *, const void *));

posicion ultimo(lista l);

static int esultimoelemento(struct nodo *p);

void borrar(lista l, void *x, int (*comp)(const void *, const void *));

posicion primero(lista l);
posicion siguiente(posicion p);
int esfindelista(posicion p);
void *direccion(posicion p);
char * hora(posicion p);
int tamano(posicion p);
char * nombre(posicion p);
int jkr(posicion p);

#endif
