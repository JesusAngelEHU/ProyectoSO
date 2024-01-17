#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/loader.h"

//Funcion para inicializar la cola de procesos
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


//Funcion para leer los programas
void leer_fichero(struct PCB* new_pcb, int pid){
    // Construimos el nombre del archivo basado en el PID
    int i=0;
    char filename[25];
    char code[20],data[20];
    sprintf(filename, "prometheus/prog%03d.elf", pid);
    FILE *archivo = fopen(filename, "r");
    char linea[512];
    //Guardamos en los punteros el code y el data 
    while (fgets(linea, sizeof(linea), archivo) != NULL){
        if(i==0){
            sscanf(linea,".text %s", code);
            new_pcb->mm->code=atoi(code);
        } 
        if(i==1){
            sscanf(linea, ".data %s",data);
            new_pcb->mm->data=atoi(data);
        }
        i++;
    }
    printf("&code:%i &data:%i\n",new_pcb->mm->code,new_pcb->mm->data);
}

//Funcion para crear un nuevo pcb
struct PCB* crear_pcb(int* pid){
    struct PCB* new_pcb = (struct PCB*)malloc(sizeof(struct PCB));
                new_pcb->PID=(*pid)++;
                strcpy(new_pcb->estado,"Preparado");
                new_pcb->siguiente=NULL;
                new_pcb->quantum=rand()%30 + 1;
                new_pcb->quantum_max=new_pcb->quantum;
                new_pcb->mm=(struct MemoryManagement *)malloc(sizeof(struct MemoryManagement));
    return new_pcb;
}

//Funcion Process Generator
void *loader_thread(void *args){
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
            struct PCB* new_pcb=crear_pcb(&pid);
            leer_fichero(new_pcb,pid);
            addPCB(&lista,new_pcb);
            //imprimirEstadoCola();//Descomentar para vez como se actualiza la cola iteracion a iteracion
            //printf("++Proceso %i creado con Quantum:%i\n", lista.last->PID,new_pcb->quantum);
            clk2=0;
        }
    }
}
