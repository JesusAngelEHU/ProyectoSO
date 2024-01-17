#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../include/loader.h"

//Funcion para inicializar la cola de procesos
void initializeProcessQueue(){
    lista.first = NULL;
    lista.last = NULL;
}

//Funcion para inicializar la memoria
void initializeMP(){
    int i;
    for(i=0;i<TAMAÑO_MEMORIA;i++){
        mp.mem[i]=0;
    }
    for(i=0;i<TAMAÑO_TABLA_PAGINAS;i++){
        mp.tabladepaginas[i]=0;
    }
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

//Convertir la direccion data en una direccion de nuestra memoria
int convertirDireccionEnMemoria(char direccion[20]){
    int decimal = 0;
    int length = strlen(direccion);

    // Iterar a través de los dígitos hexadecimales
    for (int i = 0; i < length; i++) {
        char currentChar = direccion[i];

        // Convertir el carácter hexadecimal a su valor decimal
        int digitValue;
        if (currentChar >= '0' && currentChar <= '9') {
            digitValue = currentChar - '0';
        } else if (currentChar >= 'A' && currentChar <= 'F') {
            digitValue = 10 + (currentChar - 'A');
        } else if (currentChar >= 'a' && currentChar <= 'f') {
            digitValue = 10 + (currentChar - 'a');
        }
        // Calcular el valor decimal acumulado
        decimal += digitValue * pow(16, length - i - 1);
    }
    if(decimal==0) return 0;
    else return decimal / 4 - 1;
}
// Función para leer los programas
int leer_fichero(struct PCB* new_pcb) {
    // Construimos el nombre del archivo basado en el PID
    int i = 0;
    char filename[25];
    char direccion[20];
    sprintf(filename, "prometheus/prog%03d.elf", new_pcb->PID);
    FILE* archivo = fopen(filename, "r");
    strcpy(new_pcb->nombre,filename);
    // Verificar si la apertura del archivo fue exitosa
    if (archivo != NULL) {
        char linea[512];
        // Guardamos en los punteros el code y el data
            while (fgets(linea, sizeof(linea), archivo) != NULL) {
            if (i == 0){
                sscanf(linea, ".text %s", direccion);
                new_pcb->mm->code = convertirDireccionEnMemoria(direccion);
            }
            if (i == 1){
                sscanf(linea, ".data %s", direccion);
                new_pcb->mm->data=convertirDireccionEnMemoria(direccion);
            }
            mp.mem[new_pcb->mm->pgb+i]=atoi(linea);
            mp.tabladepaginas[new_pcb->mm->pgb + i]=1;
            i++;
            }
    }else{
        printf("Se han leido todos los archivos\n");
        return 1;
    } 
    return 0;
    fclose(archivo);
}



//Funcion para encontrar un hueco en la tabla de paginas
int encontrarHueco(){
    for (int i=0; i<TAMAÑO_TABLA_PAGINAS; i++){
        if (mp.tabladepaginas[i]==0) return i;
    }
    return 0;
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
                new_pcb->mm->pgb=encontrarHueco();
    return new_pcb;
}

//Funcion Process Generator
void *loader_thread(void *args){
    int frecuencia=*((int *) args);
    int pid=0,clk2=0,fin=0;
    initializeProcessQueue();
    pthread_mutex_lock(&mutex);
    while(1){
        done++;
        clk2++;
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond2,&mutex);
        if(clk2>=frecuencia){
            if(fin==0){
                struct PCB* new_pcb=crear_pcb(&pid);
                fin=leer_fichero(new_pcb);
                if(fin==0)addPCB(&lista,new_pcb);
                //imprimirEstadoCola(); //Descomentar para vez como se actualiza la cola iteracion a iteracion
                if(fin==0)printf("++Proceso %s con pid %i creado con Quantum:%i pgb:%i\n", new_pcb->nombre,new_pcb->PID,new_pcb->quantum,new_pcb->mm->pgb);
            }
            clk2=0;
        }
    }
}
