#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <signal.h> 
#include <string.h>
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t cond2;

int clk = 0,done=0, ntemps=2,proceso_generado=1;
int ncpus,ncores,nthreads;
// Estructura PCB
struct PCB {
    int PID;
    char estado[10];
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
//Inicializacion de las estructuras
struct ProcessQueue lista;
struct CPU* cpus = NULL;

void initializeProcessQueue(struct ProcessQueue* lista){
    lista->first = NULL;
    lista->last = NULL;
}

void addPCB(struct ProcessQueue* lista, struct PCB* pcb){
    if(lista->first==NULL){
        lista->first=pcb;
        lista->last=pcb;
    }else{
        lista->last->siguiente=pcb;
        lista->last=pcb;
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

// Función Scheduler/Dispatcher
void *scheduler_dispatcher_thread(void *args) {
    pthread_mutex_lock(&mutex);
    while (1) {
        done++;
        clk++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex);
        if(clk==10){
            if(proceso_generado){
                if (!strcmp(lista.first->estado, "Preparado")){
                    printf("Primer pcb preparado");
                    for(int i=0; i<ncpus;i++){
                        for(int j=0; j<ncores;j++){
                            for(int k=0; k<nthreads;k++){
                                if(cpus[i].cores[j].threads[k].pcb==NULL){
                                    strcpy(lista.first->estado, "Ejecucion");
                                    cpus[i].cores[j].threads[k].pcb=lista.first;
                                }
                            }
                        }
                    }
                }
            }

        }
            clk=0;
    }
}

//Funcion Process Generator
void *process_generator_thread(void *args){
    int frecuencia=*((int *) args);
    int pid=0,clk2=0;
    cpus = (struct CPU*)malloc(ncpus * sizeof(struct CPU));
    initializeProcessQueue(&lista);
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
            addPCB(&lista,new_pcb);
            printf("Proceso generado PID:%i\n",lista.last->PID);
            clk2=0;
        }
    }
}

struct CPU* inicializarMachine(){
     for (int i = 0; i < ncpus; i++) {
        cpus = (struct CPU*)malloc(ncpus * sizeof(struct CPU));
        cpus[i].cpu_id=i;
        cpus[i].cores=NULL;
        cpus[i].cores = (struct Core*)malloc(ncores * sizeof(struct Core));

        for (int j = 0; j < ncores; j++) {
            cpus[i].cores[j].id_core = j;
            cpus[i].cores[j].threads = (struct Thread*)malloc(nthreads * sizeof(struct Thread));
            for (int k = 0; k < nthreads; k++) {
                cpus[i].cores[j].threads[k].id_thread = k;
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
    //Inicializar estructura machine
    struct CPU* cpus = inicializarMachine();

    // Crea el hilo Clock
    pthread_t clock_thread_id;
    if (pthread_create(&clock_thread_id, NULL, clock_thread, NULL) != 0) {
        perror("Error al crear el hilo Clock");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Scheduler/Dispatcher
    pthread_t scheduler_dispatcher_thread_id;
    if (pthread_create(&scheduler_dispatcher_thread_id, NULL, scheduler_dispatcher_thread,NULL) != 0) {
        perror("Error al crear el hilo Scheduler/Dispatcher");
        exit(EXIT_FAILURE);
    }
    // Crea el hilo Process Generator
    pthread_t process_generator_thread_id;
    if (pthread_create(&process_generator_thread_id, NULL, process_generator_thread, &frecuencia) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(EXIT_FAILURE);
    }
    //Liberar las variables de la estructura machine
    //liberarMachine(cpus,ncpus,ncores);

    // //Joins y destroys
    pthread_join(clock_thread_id,NULL);
    pthread_join(scheduler_dispatcher_thread_id,NULL);
    pthread_join(process_generator_thread_id,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    return 0;
}
