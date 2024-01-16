#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <signal.h> 
#include <string.h>
#include <limits.h>
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t cond2;

int clk = 0,done=0, ntemps=2,proceso_generado=1;
int ncpus,ncores,nthreads;

// Estructura PCB
struct PCB {
    int PID;
    char estado[10];
    int quantum;
    int quantum_max;
    struct PCB* siguiente;
};

// Estructura ProcessQueue
struct ProcessQueue {
    struct PCB* first;
    struct PCB* last;
};

struct Thread{
    int id_thread;
    struct PCB* pcb;
};

struct Core{
    int id_core;
    struct Thread* threads;
};

struct CPU {
    int cpu_id;
    struct Core* cores;
};

struct CPU* cpus;
struct ProcessQueue lista;

void initializeProcessQueue(){
    lista.first = NULL;
    lista.last = NULL;
}
// Función para imprimir el estado de la cola y los PCBs
void imprimirEstadoCola() {
    if (lista.first == NULL) {
        printf("Cola vacía\n");
        return;
    }

    printf("Estado de la cola:\n");

    struct PCB* aux = lista.first;
    do {
        printf("PID: %d, Estado: %s, Quantum: %d\n", aux->PID, aux->estado, aux->quantum);
        aux = aux->siguiente;
    } while (aux != lista.first);

    printf("Fin de la cola\n");
}

void addPCB(struct ProcessQueue* lista, struct PCB* pcb){
    if(lista->first==NULL){
        lista->first=pcb;
        lista->last=pcb;
        pcb->siguiente=pcb;
    }else{
        lista->last->siguiente=pcb;
        lista->last=pcb;
        pcb->siguiente=lista->first;
        }
}


// Función Clock
void *clock_thread(void *args) {
    while (1) {
        usleep(100000);
        pthread_mutex_lock(&mutex);
        while(done < ntemps){
            pthread_cond_wait(&cond,&mutex);
        } 
        done=0;
        pthread_cond_broadcast(&cond2);
        pthread_mutex_unlock(&mutex);
    }
}


//Asigna a cada hilo libre un PCB, si no hay ninguno libre hace el cambio_de_contexto
void round_robin(){
    //Recorrer los hilos
    for(int i=0; i<ncpus; i++){
        for(int j=0; j<ncores; j++){
            for(int k=0; k<nthreads; k++){
                //Si el hilo esta libre se le asigna el pcb
                if(cpus[i].cores[j].threads[k].pcb==NULL){
                    //Recorrer la lista hasta encontrar un PCB que este preparado
                    struct PCB *aux = lista.first;
                    while(aux->PID != lista.last->PID || strcmp(aux->estado,"Preparado") != 0){
                        if(strcmp(aux->estado,"Preparado")==0){ 
                            if(cpus[i].cores[j].threads[k].pcb == NULL){
                                cpus[i].cores[j].threads[k].pcb=aux;
                                strcpy(cpus[i].cores[j].threads[k].pcb->estado,"Ejecucion");
                                printf("Proceso %i añadido a hilo %i\n",aux->PID,cpus[i].cores[j].threads[k].id_thread);
                                break;
                            }
                        }
                    aux = aux->siguiente;
                    }
                }
            }
        }
    }
}

// Función para mover un nodo al final de la cola
void moverAlFinal(struct PCB* nodo) {
    // Actualizar la referencia a lista.first solo si el nodo que se movió era el primer elemento
    if (lista.first->PID == nodo->PID) {
        lista.first = lista.first->siguiente;

    }
    if (nodo->PID != lista.last->PID) {
        // Quitar el nodo de su posición actual
        struct PCB* siguiente = nodo->siguiente;
        struct PCB* anterior = nodo;
        while (anterior->siguiente->PID != nodo->PID) {
            anterior = anterior->siguiente;
        }
        anterior->siguiente = siguiente;

        // Agregar el nodo al final de la cola
        lista.last->siguiente = nodo;
        lista.last = nodo;
        nodo->siguiente = lista.first;
    }
}



//Funcion para baja el quantum de todos los procesos en la proces_queue, si el quantum es 0
//lo quita del hilo y lo pone al final de la cola
void bajar_quantum_threads(){
    struct PCB *aux = lista.first;
    // Recorrer la lista circularmente y reducir en uno el quantum de cada nodo
    while(aux->PID != lista.last->PID){
        if(strcmp(aux->estado,"Ejecucion")==0)aux->quantum--;
        if(aux->quantum<=0){
            for(int i=0; i<ncpus; i++)
            for(int j=0; j<ncores; j++)
            for(int k=0; k<nthreads; k++){
                if(cpus[i].cores[j].threads[k].pcb != NULL && cpus[i].cores[j].threads[k].pcb->PID==aux->PID){
                    cpus[i].cores[j].threads[k].pcb = NULL;
                    printf("Proceso  %i  libera el hilo %i \n",aux->PID,cpus[i].cores[j].threads[k].id_thread);
                }
            }
            struct PCB *final = aux;
            moverAlFinal(final);
            strcpy(aux->estado,"Preparado");
            aux->quantum=aux->quantum_max; 
        }
        aux = aux->siguiente;
    } 
}

// Función Scheduler/Dispatcher
void *scheduler_dispatcher_thread(void *args) {
    int frecuencia=*((int *) args);
    pthread_mutex_lock(&mutex);
    while (1) {
        done++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex);
        if (clk>=frecuencia){
            bajar_quantum_threads();
            round_robin();
            //imprimirEstadoCola();
            clk=0;
        } 
        clk++;
    }
}

//Funcion Process Generator
void *process_generator_thread(void *args){
    int frecuencia=*((int *) args);
    int pid=0,clk2=0;
    initializeProcessQueue();
    pthread_mutex_lock(&mutex);
    while(1){
        done++;
        clk2++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex);
        if(clk2>=frecuencia){
            struct PCB* new_pcb = (struct PCB*)malloc(sizeof(struct PCB));
            new_pcb->PID=pid++;
            strcpy(new_pcb->estado,"Preparado");
            new_pcb->siguiente=NULL;
            new_pcb->quantum=rand()%20;
            new_pcb->quantum_max=new_pcb->quantum;
            addPCB(&lista,new_pcb);
            //printf("Proceso %i añadido a la cola\n", lista.last->PID);
            clk2=0;
        }
    }
}
struct CPU* inicializarMachine(){
    cpus = (struct CPU*)malloc(ncpus * sizeof(struct CPU));
    for (int i = 0; i < ncpus; i++) {
        cpus[i].cpu_id=i;
        cpus[i].cores = (struct Core*)malloc(ncores * sizeof(struct Core));

        for (int j = 0; j < ncores; j++) {
            cpus[i].cores[j].id_core = i*ncores +j;
            cpus[i].cores[j].threads = (struct Thread*)malloc(nthreads * sizeof(struct Thread));
            for (int k = 0; k < nthreads; k++) {
                cpus[i].cores[j].threads[k].id_thread = i*ncores*nthreads +j *nthreads + k;
                cpus[i].cores[j].threads[k].pcb = (struct PCB*)malloc(sizeof(struct PCB));
                cpus[i].cores[j].threads[k].pcb = NULL;
            }
        }
    }
    return cpus;
}

void liberarMachine(struct CPU* cpus, int ncpus, int ncores){
    for (int i = 0; i < ncpus; i++) {
        for (int j = 0; j < ncores; j++) {
            free(cpus[i].cores[j].threads);
        }
        free(cpus[i].cores);
    }
    struct PCB* actual = lista.first;
    struct PCB* siguiente;

    while (actual != NULL) {
        siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
}

int main(int argc, char *argv[]) {

    //Verificacion numero de argumentos
    if (argc != 5){
        printf("USO: %s frecuencia <num_cpus> <num_cores> <num_threads>\n",argv[0]);
        return 1;
    }

    //Argumentos y inicializacion de mutex
    int frecuencia = atoi(argv[1]);
    ncpus=atoi(argv[2]);
    ncores=atoi(argv[3]);
    nthreads=atoi(argv[4]);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond,NULL);
    pthread_cond_init(&cond2,NULL);

    //Inicializar estructuras
    cpus = inicializarMachine();

    // Crea el hilo Clock
    pthread_t clock_thread_id;
    if (pthread_create(&clock_thread_id, NULL, clock_thread, NULL) != 0) {
        perror("Error al crear el hilo Clock");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Scheduler/Dispatcher
    pthread_t scheduler_dispatcher_thread_id;
    if (pthread_create(&scheduler_dispatcher_thread_id, NULL, scheduler_dispatcher_thread,&frecuencia) != 0) {
        perror("Error al crear el hilo Scheduler/Dispatcher");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Process Generator
    pthread_t process_generator_thread_id;
    if (pthread_create(&process_generator_thread_id, NULL, process_generator_thread, &frecuencia) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(EXIT_FAILURE);
    }

    // //Joins y destroys
    pthread_join(clock_thread_id,NULL);
    pthread_join(scheduler_dispatcher_thread_id,NULL);
    pthread_join(process_generator_thread_id,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);

    //Liberar las variables de la estructura machine
    liberarMachine(cpus,ncpus,ncores);
    return 0;
}
