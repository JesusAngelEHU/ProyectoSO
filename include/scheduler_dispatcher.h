// scheduler.h
#include "loader.h"
#ifndef SCHEDULER_H
#define SCHEDULER_H

struct Thread{
    int id_thread;
    struct PCB* pcb;
    int PTBR;
    int mmu;
    int tlb;

};

struct Core{
    int id_core;
    struct Thread* threads;
};

struct CPU {
    int cpu_id;
    struct Core* cores;
};

extern int ncpus;
extern int ncores;
extern int nthreads;
extern struct CPU* cpus;
extern struct ProcessQueue lista;
extern struct PCB* new_pcb;
extern int clk;

// Declaración de funciones
void *scheduler_dispatcher_thread(void *args);
void liberarMachine(struct CPU* cpus, int ncpus, int ncores);
struct CPU* inicializarMachine();
void *scheduler_dispatcher_thread(void *args);
void bajar_quantum_threads();
void moverAlFinal(struct PCB** nodo);
void round_robin();

#endif  // SCHEDULER_H
