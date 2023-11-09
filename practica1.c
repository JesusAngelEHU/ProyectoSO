#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <signal.h> 

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t cond2;

int clk = 0,done=0, ntemps=2;

// Estructura PCB
struct PCB {
    int PID;
    // Otros campos relacionados con el PCB
    struct PCB* siguiente;
};

// Estructura ProcessQueue
struct ProcessQueue {
    struct PCB* first;
    struct PCB* last;
};

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
        pthread_mutex_lock(&mutex);
        while(done < ntemps){
            pthread_cond_wait(&cond,&mutex);
        } 
        done=0;
        pthread_cond_broadcast(&cond2);
        pthread_mutex_unlock(&mutex);

    }
}


/* // Función Process Generator
void *timer_thread(void *args) {
    pthread_mutex_lock(&mutex);
    while (1) {
        printf("suma 1\n");
        done++;
        clk2++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex); 
    }
} */

// Función Scheduler/Dispatcher
void *scheduler_dispatcher_thread(void *args) {
    pthread_mutex_lock(&mutex);
    while (1) {
        done++;
        clk++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex);
        if(clk==10000){
            printf("Scheduler \n");
            clk=0;
        }
    }
}

//Funcion Process Generator
void *process_generator_thread(void *args){
    int frecuencia=*((int *) args);
    int pid=0,clk2=0;
    struct ProcessQueue lista;
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
            new_pcb->siguiente=NULL;
            addPCB(&lista,new_pcb);
            printf("Process Generator\n");
            clk2=0;
        }
    }
}

int main(int argc, char *argv[]) {
    // Inicializaciones
    if (argc != 2){
        printf("USO: %s frecuencia\n",argv[0]);
        return 1;
    }
    int frecuencia = atoi(argv[1]);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond,NULL);
    pthread_cond_init(&cond2,NULL);

    // Crea el hilo Clock
    pthread_t clock_thread_id;
    if (pthread_create(&clock_thread_id, NULL, clock_thread, NULL) != 0) {
        perror("Error al crear el hilo Clock");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Scheduler/Dispatcher
    pthread_t scheduler_dispatcher_thread_id;
    if (pthread_create(&scheduler_dispatcher_thread_id, NULL, scheduler_dispatcher_thread, NULL) != 0) {
        perror("Error al crear el hilo Scheduler/Dispatcher");
        exit(EXIT_FAILURE);
    }
    // Crea el hilo Process Generator
    pthread_t process_generator_thread_id;
    if (pthread_create(&process_generator_thread_id, NULL, process_generator_thread, &frecuencia) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(EXIT_FAILURE);
    }


    pthread_join(clock_thread_id,NULL);
    pthread_join(scheduler_dispatcher_thread_id,NULL);
    pthread_join(process_generator_thread_id,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    return 0;
}
