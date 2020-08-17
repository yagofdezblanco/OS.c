#ifndef __LISTASP_H__
#define __LISTASP_H__

struct nodolp {
   int pid;
   char h[50];
   int prioridad;
   int signalName;
   int state;
   char commandLine[50];
   struct nodolp *sig;
};
typedef struct nodolp *posicionlp;
typedef struct nodolp *listap;

static struct nodolp *crearnodolp();
listap crearlistap();
int eslistapvacia(listap l);
void insertarlp(int x, posicionlp p);
void insertarhoralp(char hora[], posicionlp p);
void insertarPrioridad(int prioridad, posicionlp p);
void insertarSignal(int signalName, posicionlp p);
void insertarState(int state, posicionlp p);
void insertarCommandLine(char commandLine[], posicionlp p);
void insertarProceso(listap l, int pid, char hora[], int prioridad, int signalName, int state, char commandLine[]);
posicionlp buscarlp(listap l, int e);
static posicionlp buscarantlp(listap l, int x);
posicionlp ultimolp(listap l);
static int esultimoelementolp(struct nodolp *p);
void borrarlp(listap l, int x);
posicionlp primerolp(listap l);
posicionlp siguientelp(posicionlp p);
int esfindelistap(posicionlp p);
int pid(posicionlp p);
char * horalp(posicionlp p);
int prioridad(posicionlp p);
int signalName(posicionlp p);
int state(posicionlp p);
char * commandLine(posicionlp p);

#endif
