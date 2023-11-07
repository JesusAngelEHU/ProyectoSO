#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <signal.h> 

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t cond2;

int clk = 0,done=0, ntemps=2;

//Estructura PCBs
struct PCB{
    int PID;
};

struct ProcessQueue {
    struct PCB* pcb; // Puntero al PCB en este nodo de la cola
    struct PCB* siguiente; // Puntero al siguiente nodo de la cola
};

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


// Función Process Generator
void *timer_thread(void *args) {
    pthread_mutex_lock(&mutex);
    while (1) {
        printf("suma 1\n");
        done++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex); 
    }
}

// Función Scheduler/Dispatcher
void *scheduler_dispatcher_thread(void *args) {
    pthread_mutex_lock(&mutex);
    while (1) {
        printf("suma 2\n");
        done++;
        clk++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex); 
    }
}


//Funcion Process Generator
void *process_generator_thread(void *args){
    struct ProcessQueue* pcb_list = NULL;
    int pid=1;
    while (1) {
        //Crear nuevo pcb
        struct PCB* new_pcb = (struct PCB*)malloc(sizeof(struct PCB));
        new_pcb->PID=pid++;

        // Agrega el nuevo PCB a la lista
        if (pcb_list == NULL) {
            pcb_list->pcb=new_pcb; // La lista estaba vacía, el nuevo PCB se convierte en la cabeza de la lista
            pcb_list->siguiente=NULL;
        } else {
            // Encuentra el último PCB en la lista y agrega el nuevo PCB al final
            while(pcb_list!=NULL) pcb_list=pcb_list->siguiente;
            pcb_list->pcb=new_pcb;
            pcb_list->siguiente=NULL;
        }
    }
}

int main() {
    // Inicializaciones
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond,NULL);
    pthread_cond_init(&cond2,NULL);

    // Crea el hilo Clock
    pthread_t clock_thread_id;
    if (pthread_create(&clock_thread_id, NULL, clock_thread, NULL) != 0) {
        perror("Error al crear el hilo Clock");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo timer
    pthread_t timer_thread_id;
    if (pthread_create(&timer_thread_id, NULL, timer_thread, NULL) != 0) {
        perror("Error al crear el hilo timer");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo timer
    pthread_t timer2_thread_id;
    if (pthread_create(&timer2_thread_id, NULL, timer2_thread, NULL) != 0) {
        perror("Error al crear el hilo timer2");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Process Generator
    pthread_t process_generator_thread_id;
    if (pthread_create(&process_generator_thread_id, NULL, process_generator_thread, NULL) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(EXIT_FAILURE);
    }

    // Crea el hilo Scheduler/Dispatcher
    pthread_t scheduler_dispatcher_thread_id;
    if (pthread_create(&scheduler_dispatcher_thread_id, NULL, scheduler_dispatcher_thread, NULL) != 0) {
        perror("Error al crear el hilo Scheduler/Dispatcher");
        exit(EXIT_FAILURE);
    }

    pthread_join(clock_thread_id,NULL);
    pthread_join(timer_thread_id,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
    return 0;
}
