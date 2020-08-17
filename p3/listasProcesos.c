#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "listasProcesos.h"

static struct nodolp *crearnodolp(){
   struct nodolp *tmp = malloc(sizeof(struct nodolp));
   if (tmp == NULL) {
      printf("memoria agotada\n"); exit(EXIT_FAILURE);
   }
   return tmp;
}

listap crearlistap(){
   struct nodolp *l = crearnodolp();
   l->sig = NULL;
   return l;
}

int eslistapvacia(listap l){
   return (l->sig == NULL);
}

void insertarlp(int x, posicionlp p) {
   struct nodolp *tmp = crearnodolp();
   tmp->pid = x; tmp->sig = p->sig;
   p->sig = tmp;
}

void insertarhoralp(char hora[], posicionlp p) {
   if(p!=NULL){
      strcpy(p->h, hora);
   }
}

void insertarPrioridad(int prioridad, posicionlp p) {
   if(p!=NULL){
      p->prioridad=prioridad;
   }
}

void insertarSignal(int signalName, posicionlp p) {
   if(p!=NULL){
      p->signalName=signalName;
   }
}

void insertarState(int state, posicionlp p) {
   if(p!=NULL){
      p->state=state;
   }
}

void insertarCommandLine(char commandLine[], posicionlp p) {
   if(p!=NULL){
      strcpy(p->commandLine, commandLine);
   }
}

void insertarProceso(listap l, int pid, char hora[], int prioridad, int signalName, int state, char commandLine[]){
   posicionlp pos=ultimolp(l);
   insertarlp(pid, pos);
   pos=ultimolp(l);
   insertarhoralp(hora, pos);
   insertarCommandLine(commandLine, pos);
   insertarSignal(signalName, pos);
   insertarState(state, pos);
   insertarPrioridad(prioridad, pos);
}

posicionlp buscarlp(listap l, int e){
   struct nodolp *p = l->sig;
   while (p != NULL && p->pid!=e)
      p = p->sig;
   return p;
}

static posicionlp buscarantlp(listap l, int x) {
   struct nodolp *p = l;
   while (p->sig != NULL && p->sig->pid!=x)
      p = p->sig;
   return p;
}

posicionlp ultimolp(listap l){
   struct nodolp *p=l;
   while(p->sig!=NULL)
      p=p->sig;
   return p;
}

static int esultimoelementolp(struct nodolp *p) {
   return (p->sig == NULL);
}

void borrarlp(listap l, int x) {
   struct nodolp *tmp, *p = buscarantlp(l, x);
   if (!esultimoelementolp(p)) {
      tmp = p->sig;
      p->sig = tmp->sig;
      free(tmp);
   }
}

posicionlp primerolp(listap l) { return l->sig; }
posicionlp siguientelp(posicionlp p) { return p->sig; }
int esfindelistap(posicionlp p) { return (p==NULL); }
int pid(posicionlp p) { return p->pid; }
char * horalp(posicionlp p) { return p->h; }
int prioridad(posicionlp p) { return p->prioridad; }
int signalName(posicionlp p) { return p->signalName; }
int state(posicionlp p) { return p->state; }
char * commandLine(posicionlp p) { return p->commandLine; }
