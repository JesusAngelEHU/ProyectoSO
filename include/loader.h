#ifndef LOADER_H
#define LOADER_H
#include <pthread.h>
// Estructura PCB
struct PCB {
    int PID;
    char estado[10];
    int quantum;
    int quantum_max;    
    struct PCB* siguiente;
    struct MemoryManagement* mm;
    int pc;
    int tamaño;
};

// Estructura ProcessQueue
struct ProcessQueue {
    struct PCB* first;
    struct PCB* last;
};
extern struct ProcessQueue lista;
extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern pthread_cond_t cond2;
extern int done;

// Declaración de funciones
void *loader_thread(void *args);
struct PCB* crear_pcb(int* pid);
void addPCB(struct ProcessQueue* lista, struct PCB* pcb);
void imprimirEstadoCola();
void initializeProcessQueue();

#endif 