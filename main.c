#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include "include/clock.h"
#include "include/scheduler_dispatcher.h"
#include "include/loader.h"
#define MEM_PHYSICAL_SIZE (1 << 24) // 2^24
#define TABLEPAGE (1<<22)

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_cond_t cond2;

int clk = 0,done=0, ntemps=2,proceso_generado=1;
int ncpus,ncores,nthreads;

//Estructura de memoria
struct PhysicalMemory{
    int mem[(1<<24) - (1<< 22)];
    int tabladepaginas[1<<22];
};

//Estructura memoryManagement
struct MemoryManagement{
    int code;
    int data;
    int pgb;
};

struct CPU* cpus;
struct ProcessQueue lista;
struct PhysicalMemory mp;

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
    pthread_t loader_thread_id;
    if (pthread_create(&loader_thread_id, NULL, loader_thread, &frecuencia) != 0) {
        perror("Error al crear el hilo Process Generator");
        exit(EXIT_FAILURE);
    }

    // //Joins y destroys
    pthread_join(clock_thread_id,NULL);
    pthread_join(scheduler_dispatcher_thread_id,NULL);
    pthread_join(loader_thread_id,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);

    //Liberar todo
    //liberarMachine(cpus,ncpus,ncores);
    return 0;
}
