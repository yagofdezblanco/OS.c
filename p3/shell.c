/*
Sistemas Operativos
Grupo 3.1
Alberto Carreño Piñeiro alberto.carreno.pineiro
Yago Fernandez Blanco yago.fernandez1
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>
#include "listas.h"
#include "listasProcesos.h"
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <grp.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

struct SEN{
	char *nombre;
	int senal;
};
static struct SEN sigstrnum[]={
	"HUP", SIGHUP,
	"INT", SIGINT,
	"QUIT", SIGQUIT,
	"ILL", SIGILL,
	"TRAP", SIGTRAP,
	"ABRT", SIGABRT,
	"IOT", SIGIOT,
	"BUS", SIGBUS,
	"FPE", SIGFPE,
	"KILL", SIGKILL,
	"USR1", SIGUSR1,
	"SEGV", SIGSEGV,
	"USR2", SIGUSR2,
	"PIPE", SIGPIPE,
	"ALRM", SIGALRM,
	"TERM", SIGTERM,
	"CHLD", SIGCHLD,
	"CONT", SIGCONT,
	"STOP", SIGSTOP,
	"TSTP", SIGTSTP,
	"TTIN", SIGTTIN,
	"TTOU", SIGTTOU,
	"URG", SIGURG,
	"XCPU", SIGXCPU,
	"XFSZ", SIGXFSZ,
	"VTALRM", SIGVTALRM,
	"PROF", SIGPROF,
	"WINCH", SIGWINCH,
	"IO", SIGIO,
	"SYS", SIGSYS,
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
	"POLL", SIGPOLL,
#endif
#ifdef SIGPWR
	"PWR", SIGPWR,
#endif
#ifdef SIGEMT
	"EMT", SIGEMT,
#endif
#ifdef SIGINFO
	"INFO", SIGINFO,
#endif
#ifdef SIGSTKFLT
	"STKFLT", SIGSTKFLT,
#endif
#ifdef SIGCLD
	"CLD", SIGCLD,
#endif
#ifdef SIGLOST
	"LOST", SIGLOST,
#endif
#ifdef SIGCANCEL
	"CANCEL", SIGCANCEL,
#endif
#ifdef SIGTHAW
	"THAW", SIGTHAW,
#endif
#ifdef SIGFREEZE
	"FREEZE", SIGFREEZE,
#endif
#ifdef SIGLWP
	"LWP", SIGLWP,
#endif
#ifdef SIGWAITING
	"WAITING", SIGWAITING,
#endif
	NULL,-1,
};    /*fin array sigstrnum */

int var1=10;
char var2='2';
char *var3="tres";

/******************************SENALES ******************************************/

int Senal(char * sen)  /*devuel el numero de senial a partir del nombre*/
{
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
		if (!strcmp(sen, sigstrnum[i].nombre))
			return sigstrnum[i].senal;
	return -1;
}

char *NombreSenal(int sen)  /*devuelve el nombre senal a partir de la senal*/
{                       /* para sitios donde no hay sig2str*/
	int i;
	for (i=0; sigstrnum[i].nombre!=NULL; i++)
		if (sen==sigstrnum[i].senal)
			return sigstrnum[i].nombre;
	return ("SIGUNKNOWN");
}

/******************************ESTADOS*****************************************/

char *nombreEstado(int est){
	switch(est){
		case 0:
			return "Terminated Normaly";
		break;
		case 1:
			return "Terminated by Signal";
		break;
		case 2:
			return "Stopped";
		break;
		case 3:
			return "Running";
		break;
		default: return "";
	}
}

void imprimir(lista direcciones, int i){
	if(i==1){
		posicion pos = primero(direcciones);
		while (pos!=NULL){
			printf("%p: size:%d. malloc %s\n", direccion(pos), tamano(pos), hora(pos));
			pos=siguiente(pos);
		}
	}else if(i==2){
		posicion pos = primero(direcciones);
		while (pos!=NULL){
			printf("%p: size:%d. mmap %s (fd:%d) %s\n", direccion(pos), tamano(pos), nombre(pos), jkr(pos), hora(pos));
			pos=siguiente(pos);
		}
	}else{
		posicion pos = primero(direcciones);
		while (pos!=NULL){
			printf("%p: size:%d. shared memory (key %d) %s\n", direccion(pos), tamano(pos), jkr(pos), hora(pos));
			pos=siguiente(pos);
		}
	}
}


void amalloc (int args, char *arg[], lista direcciones_malloc){
	int tam=0;
	void *direc;
	if(args>2)
		sscanf(arg[2],"%d",&tam);
	if(tam!=0){
		direc=malloc(tam);
		if (direc==NULL){
			perror("Error: ");
		}else{
			time_t hora;
			time(&hora);
			insertar(direc, ultimo(direcciones_malloc));
			insertartamano(tam, ultimo(direcciones_malloc));
			insertarhora(ctime(&hora), ultimo(direcciones_malloc));
			printf("allocated %d at %p \n",tam,direc);
		}
	}else
		imprimir(direcciones_malloc, 1);
}

void * MmapFichero (char * fichero, int protection, lista direcciones_mmap){
	int df, map=MAP_PRIVATE,modo=O_RDONLY;
	struct stat s;
	void *p;
	if (protection&PROT_WRITE)  modo=O_RDWR;
	if (stat(fichero,&s)==-1 || (df=open(fichero, modo))==-1)
		return NULL;
	if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
		return NULL;
	time_t hora;
	time(&hora);
	insertar(p, ultimo(direcciones_mmap));
	insertartamano(s.st_size, ultimo(direcciones_mmap));
	insertarhora(ctime(&hora), ultimo(direcciones_mmap));
	insertarjkr(df, ultimo(direcciones_mmap));
	insertarnombre(fichero, ultimo(direcciones_mmap));
	return p;
}

void ammap (int args, char *arg[], lista direcciones_mmap){
	char *perm;
	int protection=0;
	void *direc;
	if(args>2){
		if((perm=arg[3])!=NULL && strlen(perm)<4){
			if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
			if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
			if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
		}
		if ((direc=MmapFichero(arg[2],protection, direcciones_mmap))==NULL)
			perror ("Imposible mapear fichero");
		else{
			printf ("file %s mapped at %p\n", arg[2], direc);
		}
	}else
		imprimir(direcciones_mmap, 2);
}

void * ObtenerMemoriaShmget (key_t clave, off_t tam, lista direcciones_shared){
	void * p;
	int aux,id,flags=0777;
	struct shmid_ds s;
	if (tam)
		flags=flags | IPC_CREAT | IPC_EXCL;
	if (clave==IPC_PRIVATE){
		errno=EINVAL; return NULL;
	}
	if ((id=shmget(clave, tam, flags))==-1)
		return (NULL);
	if ((p=shmat(id,NULL,0))==(void*) -1){
		aux=errno;
		if (tam)
			shmctl(id,IPC_RMID,NULL);
		errno=aux;
		return (NULL);
	}
	shmctl (id,IPC_STAT,&s);
	time_t hora;
	time(&hora);
	insertar(p, ultimo(direcciones_shared));
	insertartamano(tam, ultimo(direcciones_shared));
	insertarhora(ctime(&hora), ultimo(direcciones_shared));
	insertarjkr(clave, ultimo(direcciones_shared));
	return (p);
}

void ashared(int args, char *arg[], lista direcciones_shared){
	key_t k;
	off_t tam=0;
	void *p;
	if (arg[2]==NULL){
		imprimir(direcciones_shared, 3);
		return;
	}
	k=(key_t) atoi(arg[2]);
	if (arg[3]!=NULL)
		tam=(off_t) atoll(arg[3]);
	if ((p=ObtenerMemoriaShmget(k,tam, direcciones_shared))==NULL)
		perror ("Imposible obtener memoria shmget");
	else
		printf ("Allocated shared memory (key %d) at %p\n",k,p);
}

void asignar(int args, char *arg[], lista direcciones_malloc, lista direcciones_mmap, lista direcciones_shared){
	if(args>1){
		if(strstr(arg[1], "malloc")!=NULL){
			amalloc(args, arg, direcciones_malloc);
		}else if(strstr(arg[1], "mmap")!=NULL){
			ammap(args, arg, direcciones_mmap);
		}else if(strstr(arg[1], "shared")!=NULL){
			ashared(args, arg, direcciones_shared);
		}else{
			printf("La instruccion no es correcta");  
		}
	}else{
		imprimir(direcciones_malloc, 1);
		imprimir(direcciones_mmap, 2);
		imprimir(direcciones_shared, 3);
	}
}

int comp(const void *x, const void *y){
	return x!=y;
}

void dmalloc (int args, char *arg[], lista direcciones_malloc){
	int tam=0;
	void *direc;
	if(args>2){
		sscanf(arg[2],"%d",&tam);
		if(tam!=0){
			posicion pos = buscartamano(direcciones_malloc, tam);
			if(pos!=NULL){
				direc=direccion(pos);
				borrar(direcciones_malloc, direc, comp);
				free(direc);
			}else{
				printf("No se ha encontrada ese tamaño");
				imprimir(direcciones_malloc, 1);
			}
		}
	}else
		imprimir(direcciones_malloc, 1);
}

void dmmap (int args, char *arg[], lista direcciones_mmap){
	void *direc;
	int fd;
	if(args>2){
		posicion pos = buscarnombre(direcciones_mmap, arg[2]);
		if(pos!=NULL){
			direc=direccion(pos);
			fd=jkr(pos);
			borrar(direcciones_mmap, direc, comp);
			close(fd);
		}else{
			printf("No se ha encontrada ese fichero");
		}
	}else
	imprimir(direcciones_mmap, 2);
}

void dshared (int args, char *arg[], lista direcciones_shared){
	int tam=0;
	void *direc;
	if(args>2){
		sscanf(arg[2],"%d",&tam);
		posicion pos = buscartamano(direcciones_shared, tam);
		if(pos!=NULL){
			direc=direccion(pos);
			borrar(direcciones_shared, direc, comp);
			shmdt(direc);
		}else{
			printf("No se ha encontrada ese tamaño\n Disponibles:\n");
			imprimir(direcciones_shared, 3);
		}
	}else
		imprimir(direcciones_shared, 3);
}

void daddr (int args, char *arg[], lista direcciones_malloc, lista direcciones_mmap, lista direcciones_shared){
	void *direc;
	int fd;
	if(args>1){
		sscanf(arg[1], "%p", &direc);
		posicion pos = buscar(direcciones_malloc, direc, comp);
		if(pos!=NULL){
			borrar(direcciones_malloc, direc, comp);
			free(direc);
		}else{
			posicion pos = buscar(direcciones_mmap, direc, comp);
			if(pos!=NULL){
				fd=jkr(pos);
				borrar(direcciones_mmap, direc, comp);
				close(fd);
			}else{
				posicion pos = buscar(direcciones_shared, direc, comp);
				if(pos!=NULL){
					borrar(direcciones_shared, direc, comp);
					shmdt(direc);
				}else{
					printf("No se ha encontrada esa direccion\n Disponibles:\n");
					imprimir(direcciones_malloc, 1);
					imprimir(direcciones_mmap, 2);
					imprimir(direcciones_shared, 3);
				}
			}
		}
	}else{
		imprimir(direcciones_malloc, 1);
		imprimir(direcciones_mmap, 2);
		imprimir(direcciones_shared, 3);
	}
}

void desasignar(int args, char *arg[], lista direcciones_malloc, lista direcciones_mmap, lista direcciones_shared){
	if(args>1){
		if(strstr(arg[1], "malloc")!=NULL){
			dmalloc(args, arg, direcciones_malloc);
		}else if(strstr(arg[1], "mmap")!=NULL){
			dmmap(args, arg, direcciones_mmap);
		}else if(strstr(arg[1], "shared")!=NULL){
			dshared(args, arg, direcciones_shared);
		}else{
			daddr(args, arg, direcciones_malloc, direcciones_mmap, direcciones_shared);
		}
	}else{
		printf("la instruccion esta incompleta");
	}
}

void rmkey (char *arg[]){
	key_t clave;
	int id;
	char *key=arg[1];
	if (key==NULL || (clave=(key_t) strtoul(key,NULL,10))==IPC_PRIVATE){
		printf ("   rmkey  clave_valida\n");
		return;
	}
	if ((id=shmget(clave,0,0666))==-1){
		perror ("shmget: imposible obtener memoria compartida");
		return;
	}
	if (shmctl(id,IPC_RMID,NULL)==-1)
		perror ("shmctl: imposible eliminar memoria compartida\n");
}

void imprimirpid(){
	printf("Pid del proceso: %d\nPid del padre del proceso: %d\n\n", getpid(), getppid());
}

void autores(){
	printf("Autores:\nAlberto Carreño Piñeiro, login: alberto.carreno.pineiro\nYago Fernandez Blanco, login: yago.fernandez1\n\n");
}


ssize_t LeerFichero (char *fich, void *p, ssize_t n){
	#define LEERCOMPLETO ((ssize_t)-1)
	ssize_t  nleidos,tam=n;
	int df, aux;
	struct stat s;
	if (stat (fich,&s)==-1 || (df=open(fich,O_RDONLY))==-1)
		return ((ssize_t)-1);
	if (n==LEERCOMPLETO)
		tam=(ssize_t) s.st_size;
	if ((nleidos=read(df,p, tam))==-1){
		aux=errno;
		close(df);
		errno=aux;
		return ((ssize_t)-1);
	}
	close (df);
	return (nleidos);
}

void ioread (char * entrada[]){
	void *addr;
	char *fich;
	int cont;
	struct stat s;

	if(entrada[2]!=NULL) fich=entrada[2];
	if(entrada[3]!=NULL) sscanf(entrada[3], "%p", &addr); 
	if(entrada[4]!=NULL) sscanf(entrada[4], "%d", &cont);
	else {
		stat(fich, &s);
		cont=s.st_size;
	}
	LeerFichero(fich, addr, cont);
}

void iowrite(char * entrada[]){
	int cont, fd;
	void *addr;
	char * fich;
	
	if(entrada[2]!=NULL) fich=entrada[2];
	else return;
	if(entrada[3]!=NULL) sscanf(entrada[3], "%p", &addr);
	else return;
	if(entrada[4]!=NULL) sscanf(entrada[4], "%d", &cont);
	else return;
	if(entrada[5]!=NULL && !strcmp(entrada[5],"-o"))
		fd=open(fich, O_WRONLY | O_CREAT | O_TRUNC, 755);
	else fd=open(fich, O_WRONLY | O_CREAT | O_EXCL);

	if (fd==-1) {
		perror("Error");
		return;
	}
	write(fd, addr, (size_t) cont);
	close(fd); 
}
    

void iofile (int args, char *arg[]){
	if(args>1){
		if(strstr(arg[1],"read") != NULL){
			ioread(arg);
		}else if(strstr(arg[1],"write") != NULL){
			iowrite(arg);
		}
	}
}

void mem (){
	auto int var01;
	auto char var02;
	auto char *var03;
	
	extern int var1;
	extern char var2;
	extern char *var3;
	
	printf("Direccion de la funcion Autores: %p\n",autores);
	printf("Direccion de la funcion Pid: %p\n",imprimirpid);
	printf("Direccion de la funcion Rmkey: %p\n",rmkey);
	
	printf("Direccion de la variable externa var1: %p\n",(void*)&var1);
	printf("Direccion de la variable externa var2: %p\n",(void*)&var2);
	printf("Direccion de la variable externa var3: %p\n",(void*)&var3);
	
	printf("Direccion de la variable automatica var01: %p\n",(void*)&var01);
	printf("Direccion de la variable automatica var02: %p\n",(void*)&var02);
	printf("Direccion de la variable automatica var03: %p\n",(void*)&var03);
}

void recursiva (int n){
	char automatico[2048];
	static char estatico[2048];
	printf ("parametro n:%d en %p\n",n,&n);
	printf ("array estatico en:%p \n",estatico);
	printf ("array automatico en %p\n",automatico);
	if (n>0)
		recursiva(n-1);
}

void hexdump (char * addr,int desde, int hasta){
	int i;
	for(i = desde; i < hasta; i++)
		printf("%02x ", addr[i]);
	printf("\n");
}

void chardump(char * addr,int desde, int hasta){
	int i;
	for(i = desde; i < hasta; i++)
		isprint(((char*)addr)[i])? printf(" %c ",addr[i]): printf(" . ");
	putchar('\n');
}

void memdump (int args, char * entrada[]){
	char *addr;
	int cont;
	unsigned int i;

	if(args>2){ 
		sscanf(entrada[1],"%p",&addr);
		sscanf(entrada[2],"%d",&cont);
      
	}else{
		sscanf(entrada[1],"%p",&addr);
		cont=25;		
	}
	
	for(i=0; i<=cont; i= i + 25){
		chardump(addr, i, (i + 25) < cont ? i + 25 : cont);
		hexdump(addr, i, (i + 25) < cont ? i + 25 : cont);
	}
}
/**********************************************************/
/*******************Practica 3*****************************/
/**********************************************************/


void setprio (char *arg[]){
   int pid, prio;
   
	if(arg[2]==NULL) {
		pid = 0;
		if(arg[1]!=NULL){
			sscanf(arg[1], "%d", &pid);
			printf("Prioridad de proceso: %d\n", getpriority(PRIO_PROCESS, pid));
		}else {
			printf("Prioridad de proceso: %d\n", getpriority(PRIO_PROCESS, pid));
		}
	}else {
		sscanf(arg[1], "%d", &pid);
		sscanf(arg[2], "%d", &prio);
		errno=0;
		setpriority(PRIO_PROCESS, pid, prio);
		if(errno!= 0) 
         perror("Error estableciendo prioridad");
		else
			printf("Prioridad establecida en: %d\n", prio);
	}
}

void dofork(char *arg[]){

    pid_t pid;
    int status;


    pid=fork();
    if (pid<0) perror("Error en fork");
    else if (pid>0) waitpid(pid, &status, 0);


}

void doexec(char *arg[]){

   int i, prio;
   char *ejecutable;
   pid_t pid=getpid();
   ejecutable=arg[1];
    
   for(i=1;arg[i]!=NULL;i++) arg[i]=arg[i+1];
   if (( strncmp(arg[i], "@", 1) == 0 )) {
      prio = atoi(arg[i]);
      errno=0;
		setpriority(PRIO_PROCESS, pid, prio);
		if(errno!= 0) 
         perror("Error estableciendo prioridad");
		else
			printf("Prioridad establecida en: %d\n", prio);
	}else{
      execvp(ejecutable, arg);
      perror("Fallo ejecutando el proceso");
   }
}

void runfore(char *arg[]){
	pid_t pid;
	int i, status;
	char *prog;
	
	prog=arg[0];
		
	if ((pid = fork()) < 0) {
		printf("error fork\n");
		exit(1);
	}else if (pid == 0) {
		if (execvp(prog, arg) < 0) {
			printf("Error de ejecucion\n");
			exit(1);
		}
	}else {
		while (wait(&status) != pid);
	}
}

void runforepri(int args, char *arg[]){
	int i, valor, status;
	pid_t pid;
	char *prog, *priorityArguments[20];
	
	prog=arg[0];
	sscanf(arg[args-1], "@%d", &valor);
	arg[args-1]=NULL;
	args--;
	
	/*
	printf("%s ", prog);
	for(i=0;arg[i]!=NULL;i++){
		printf("%s ", arg[i]);
	}
	printf("prioridad %d\n", valor);*/
	
	if ((pid = fork()) < 0) {
		printf("error fork\n");
		exit(1);
	}else if (pid == 0) {
		int res=setpriority(PRIO_PROCESS, pid, valor);
		if(res<0) 
			perror("Error al establecer la prioridad del fork");
		else if(execvp(*arg, arg) < 0) {
			printf("Error de ejecucion\n");
			exit(1);
		}
	}else {
		while (wait(&status) != pid);
	}
}

void runback(char *arg[], listap procesos){
	pid_t pid;
	int i, status;
	char *prog;
	
	prog=arg[0];

	if ((pid=fork())==0) {
		execvp(prog, arg);
	}else if (pid<0) 
		perror("Error en el fork");
	else {
		time_t hora;
		for(i=1;arg[i]!=NULL;i++){
			strcat(prog, (" %s", arg[i]));
		}
		time(&hora);
		status=3;
		insertarProceso(procesos, pid, ctime(&hora), getpriority(PRIO_PROCESS, pid), 0, status, prog);
	}
}

void runbackpri(int args, char *arg[], listap procesos){
	int i, valor, status;
	char *prog;
	pid_t pid;

	prog=arg[0];
	sscanf(arg[args-1], "@%d", &valor);
	arg[args-1]=NULL;
	
	if ((pid = fork()) < 0) {
		printf("error fork\n");
		exit(1);
	}else if (pid == 0) {
		int res=setpriority(PRIO_PROCESS, pid, valor);
		if(res<0) 
			perror("Error al establecer la prioridad del fork");
		else if(execvp(*arg, arg) < 0) {
			printf("Error de ejecucion\n");
			exit(1);
		}
	}else {
		time_t hora;
		for(i=1;arg[i]!=NULL;i++){
			strcat(prog, (" %s", arg[i]));
		}
		time(&hora);
		status=3;
		insertarProceso(procesos, pid, ctime(&hora), valor, 0, status, prog);
	}
}

void refrescar(posicionlp pos){
	int pid, estado, senal, valor, nombreEstado;
	
	pid=pos->pid;
	if(waitpid(pos->pid, &estado, WNOHANG | WUNTRACED | WCONTINUED)==pos->pid){
		if(WIFEXITED(estado)){
			nombreEstado=0;
			valor=WEXITSTATUS(estado);
			pos->signalName=valor;
		}else if (WIFSIGNALED(estado)){
			nombreEstado=1;
			senal=WTERMSIG(estado);
			pos->signalName=senal;
		}else if(WCOREDUMP(estado)){
			nombreEstado=2;
		}else if(WIFSTOPPED(estado)){
			nombreEstado=2;
			senal=WSTOPSIG(estado);
			pos->signalName=senal;
		}else{
			nombreEstado=3;
		}
		pos->state=nombreEstado;
	}
}

void imprimirTodoslosProcesos (listap procesos){
	int estado=0;
	char *senal;
	posicionlp pos = primerolp(procesos);
	while (pos!=NULL){
		refrescar(pos);
		if(pos->signalName!=0)
			printf("%d  %d  %s  %s  %s  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), NombreSenal(pos->signalName), horalp(pos));
		else if(pos->state!=3)
			printf("%d  %d  %s  %s  %d  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), pos->signalName, horalp(pos));
		else
			printf("%d  %d  %s  %s  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), horalp(pos));
		pos=siguientelp(pos);
	}
}

void imprimirProceso (int pidp, listap procesos){
	posicionlp pos = primerolp(procesos);
	while (pos!=NULL){
		if ((pos->pid)==pidp){
			refrescar(pos);
			if(pos->signalName!=0)
				printf("%d  %d  %s  %s  %s  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), NombreSenal(pos->signalName), horalp(pos));
			else if(pos->state!=3)
				printf("%d  %d  %s  %s  %d  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), pos->signalName, horalp(pos));
			else
				printf("%d  %d  %s  %s  %s\n",pid(pos), prioridad(pos), commandLine(pos), nombreEstado(pos->state), horalp(pos));
		}
		pos=siguientelp(pos);
	}
}


void dojobs(int args,char *arg[], listap procesos){
   if (args=1 ) imprimirTodoslosProcesos(procesos);
   else printf("Opcion no valida");
}

void doproc (int args,char *arg[], listap procesos){
   int pidp,st=0;   
   posicionlp pos = primerolp(procesos);
   if (args==1){
      imprimirTodoslosProcesos(procesos);
   }else if (args==2){
      sscanf(arg[1], "%d", &pidp);
      while (pos!=NULL){
         if (pid(pos)==pidp) st=1;
         pos=siguientelp(pos);
      }
      if (st==1) imprimirProceso(pidp,procesos);
      else imprimirTodoslosProcesos(procesos);
    }
    else printf("Opcion no valida");
}

void runprog(int args, char *arg[], listap procesos){
	if(strstr(arg[args-1], "&") != NULL){
		if(strstr(arg[args-2], "@") != NULL){
			arg[args-1] = NULL;
			args--;
			runbackpri(args, arg, procesos);
		}else{
			arg[args-1] = NULL;
			args--;
			runback(arg, procesos);
		}			
	}else if(strstr(arg[args-1], "@") != NULL){
		runforepri(args, arg);
	}else{
		runfore(arg);
	}
}

void clearjobs(listap procesos){
	int estado;
	posicionlp pos=primerolp(procesos);
	posicionlp piv;
	while(pos!=NULL){
		refrescar(pos);
		if (3==(pos->state)) {
			pos=siguientelp(pos);
		}else{
			piv=siguientelp(pos);
			borrarlp(procesos, pid(pos));
			pos=piv;
		}
	}
}

void clearsig(char *sin, listap procesos){
	posicionlp pos=primerolp(procesos);
	posicionlp piv;
	while(pos!=NULL){
		refrescar(pos);
		if ((pos->signalName)!=Senal(sin)) {
			pos=siguientelp(pos);
		}else{
			piv=siguientelp(pos);
			borrarlp(procesos, pid(pos));
			pos=piv;
		}
	}
}

void clear(char *arg[], listap procesos){
	if(strcmp(arg[1], "-term")==0){
		clearjobs(procesos);
	}else if(strcmp(arg[1], "-sig")==0){
		clearsig(arg[1], procesos);
	}else{
		printf("Instruccion incorrecta\n");
	}
}

/**********************************************************/
/**********************************************************/

int TrocearCadena(char * cadena, char * trozos[]){
	int i=1;
	if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
		return 0;
	while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
		i++;
	return i;
}

void limpiar_salto (char *cadena){
	char *p;
	p = strchr (cadena, '\n');
	if (p)
		*p = '\0';
}

void liberar(lista direcciones_malloc, lista direcciones_mmap, lista direcciones_shared, listap procesos){
	void *direc;
	int fd, pid;
	posicionlp poslp=primerolp(procesos);
	posicion pos = primero(direcciones_malloc);
	while(pos!=NULL){
		direc=direccion(pos);
		pos=siguiente(pos);
		borrar(direcciones_malloc, direc, comp);
		free(direc);
	}
	free(direcciones_malloc);
	pos=primero(direcciones_mmap);
	while(pos!=NULL){
		direc=direccion(pos);
		pos=siguiente(pos);
		fd=jkr(pos);
		close(fd);
		borrar(direcciones_mmap, direc, comp);
	}
	free(direcciones_mmap);
	pos=primero(direcciones_shared);
	while(pos!=NULL){
		direc=direccion(pos);
		pos=siguiente(pos);
		borrar(direcciones_shared, direc, comp);
		shmdt(direc);
	}
	free(direcciones_shared);
	
	posicionlp piv;
	while(poslp!=NULL){
		piv=siguientelp(poslp);
		borrarlp(procesos, poslp->pid);
		poslp=piv;
	}
	free(procesos);
	
	printf("Todas las direcciones de memoria han sido liberadas\n");
}

void shell(){
	char entrada [100];
	char *arg[20];
	int args;
	int salir=0;
	lista direcciones_malloc=crearlista();
	lista direcciones_mmap=crearlista();
	lista direcciones_shared=crearlista();
	listap procesos=crearlistap();
   
	do{
		printf ("$");
		fgets (entrada, 2000, stdin);
		limpiar_salto(entrada);
		args=TrocearCadena(entrada, arg);
		if (args == 0){
			printf("wrong input\n");
		}else if (strcmp(entrada, "exit")==0 || strcmp(entrada, "end")==0 || strcmp(entrada, "fin")==0)
			salir=1;
		else if (strcmp(entrada, "autores")==0){
			autores();
		}else if(strcmp(arg[0],"pid") == 0){
			imprimirpid();
		}else if(strcmp(arg[0],"assign") == 0){
			asignar(args, arg, direcciones_malloc, direcciones_mmap, direcciones_shared);
		}else if(strcmp(arg[0],"deassign") == 0){
			desasignar(args, arg, direcciones_malloc, direcciones_mmap, direcciones_shared);
		}else if(strcmp(arg[0],"rmkey") == 0){
			rmkey(arg); 
		}else if(strcmp(arg[0],"mem") == 0){
			mem();
		}else if(strcmp(arg[0],"memdump") == 0){
			memdump(args,arg);
		}else if(strcmp(arg[0],"recursiva") == 0){
			int n;
			if (arg[1]!=NULL){
				sscanf(arg[1],"%d",&n);
				recursiva(n);
			}else ;
		}else if(strcmp(arg[0], "iofile")==0){
			iofile(args,arg);
		}else if(strcmp(arg[0], "setpriority")==0){
			setprio(arg);
		}else if(strcmp(arg[0], "fork")==0){
			dofork(arg);
		}else if(strcmp(arg[0], "exec")==0){
			doexec(arg);
      }else if(strcmp(arg[0], "jobs")==0){
			dojobs(args,arg, procesos);
      }else if(strcmp(arg[0], "proc")==0){
			doproc(args,arg, procesos);
		}else if(strcmp(arg[0], "clearjobs")==0){
			clear(arg, procesos);
		}else{
			runprog(args, arg, procesos);
		}
	}while (salir!=1);
	liberar(direcciones_malloc, direcciones_mmap, direcciones_shared, procesos);
}

int main(){
	shell();
	return 0;
}
