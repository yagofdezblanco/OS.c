#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "listas.h"

static struct nodo *crearnodo(){
   struct nodo *tmp = malloc(sizeof(struct nodo));
   if (tmp == NULL) {
      printf("memoria agotada\n"); exit(EXIT_FAILURE);
   }
   return tmp;
}

lista crearlista(){
   struct nodo *l = crearnodo();
   l->sig = NULL;
   return l;
}

int eslistavacia(lista l){
   return (l->sig == NULL);
}

void insertar(void *x, posicion p) {
   struct nodo *tmp = crearnodo();
   tmp->direc = x; tmp->sig = p->sig;
   p->sig = tmp;
}

void insertarhora(char hora[], posicion p) {
   if(p!=NULL){
      strcpy(p->h, hora);
   }
}

void insertartamano(int t, posicion p) {
   if(p!=NULL){
      p->tam = t;
   }
}

void insertarnombre(char nombre[], posicion p) {
   if(p!=NULL){
      strcpy(p->nombre, nombre);
   }
}

void insertarjkr(int jkr, posicion p) {
   if(p!=NULL){
      p->jkr = jkr;
   }
}

posicion buscar(lista l, void *e, int (*comp)(const void *x, const void *y)){
   struct nodo *p = l->sig;
   while (p != NULL && 0!=(*comp)(p->direc, e))
      p = p->sig;
   return p;
}

posicion buscartamano(lista l, int tamano){
   struct nodo *p = l->sig;
   while(p!=NULL && (p->tam)!=tamano){
      p=p->sig;
   }
   return p;
}

posicion buscarnombre(lista l, char *nombre){
   struct nodo *p = l->sig;
   while(p!=NULL && strcmp(p->nombre, nombre)!=0){
      p=p->sig;
   }
   return p;
}

static posicion buscarant(lista l, void *x, int (*comp)(const void *, const void *)) {
   struct nodo *p = l;
   while (p->sig != NULL && 0!=(*comp)(p->sig->direc, x))
      p = p->sig;
   return p;
}

posicion ultimo(lista l){
   struct nodo *p=l;
   while(p->sig!=NULL)
      p=p->sig;
   return p;
}

static int esultimoelemento(struct nodo *p) {
   return (p->sig == NULL);
}

void borrar(lista l, void *x, int (*comp)(const void *, const void *)) {
   struct nodo *tmp, *p = buscarant(l, x, comp);
   if (!esultimoelemento(p)) {
      tmp = p->sig;
      p->sig = tmp->sig;
      free(tmp);
   }
}

posicion primero(lista l) { return l->sig; }
posicion siguiente(posicion p) { return p->sig; }
int esfindelista(posicion p) { return (p==NULL); }
void *direccion(posicion p) { return p->direc; }
char * hora(posicion p) { return p->h; }
int tamano(posicion p) { return p->tam; }
char * nombre(posicion p) { return p->nombre; }
int jkr(posicion p) { return p->jkr; }

