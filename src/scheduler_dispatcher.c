#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/scheduler_dispatcher.h"
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
                                printf("|Proceso %i añadido a hilo %i|\n",aux->PID,cpus[i].cores[j].threads[k].id_thread);
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
void moverAlFinal(struct PCB** nodo) {
    struct PCB* aux = *nodo;

    // Actualizar la referencia a lista.first solo si el nodo que se movió era el primer elemento
    if (lista.first->PID == aux->PID) {
        lista.first = lista.first->siguiente;
    }

    if (aux->PID != lista.last->PID) {
        // Quitar el nodo de su posición actual
        struct PCB* siguiente = aux->siguiente;
        struct PCB* anterior = aux;
        while (anterior->siguiente->PID != aux->PID) {
            anterior = anterior->siguiente;
        }
        anterior->siguiente = siguiente;

        // Agregar el nodo al final de la cola
        lista.last->siguiente = aux;
        lista.last = aux;
        aux->siguiente = lista.first;
    }

    *nodo = aux;  // Actualizar el puntero original
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
                    //printf("--Proceso %i libera el hilo %i\n",aux->PID,cpus[i].cores[j].threads[k].id_thread);
                }
            }
            aux->quantum=aux->quantum_max;
            strcpy(aux->estado,"Preparado");
            moverAlFinal(&aux);
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