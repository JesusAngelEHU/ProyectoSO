#ifndef LOADER_H
#define LOADER_H
#define TAMAÑO_MEMORIA (1<<24)
#define TAMAÑO_TABLA_PAGINAS (1<<22)
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
    char nombre[20];
};

// Estructura ProcessQueue
struct ProcessQueue {
    struct PCB* first;
    struct PCB* last;
};

//Estructura de memoria
struct PhysicalMemory{
    int mem[TAMAÑO_MEMORIA - TAMAÑO_TABLA_PAGINAS];
    int tabladepaginas[TAMAÑO_TABLA_PAGINAS];
};

//Estructura memoryManagement
struct MemoryManagement{
    int code;
    int data;
    int pgb;
};

extern struct ProcessQueue lista;
extern pthread_mutex_t mutex;
extern pthread_cond_t cond;
extern pthread_cond_t cond2;
extern int done;
extern struct PhysicalMemory mp;

// Declaración de funciones
void *loader_thread(void *args);
struct PCB* crear_pcb(int* pid);
void addPCB(struct ProcessQueue* lista, struct PCB* pcb);
void imprimirEstadoCola();
void initializeProcessQueue();
void initializeMP();

#endif 