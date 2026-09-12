#ifndef OS_H
#define OS_H

#include "processor.h"
#include "memory.h"

#define maxtasksupport 64

typedef struct
{
    int pid;
    int proc_id;
    char input_file[100];
    char program_byte_file[100];
    char data_file[100];
    int state;
} task;

extern task task_list[maxtasksupport];
extern int proc_pid[NP];
extern char pageTable[NP][NUM_LOGICAL_PAGES];
extern char freePages[NUM_PHYSICAL_PAGES];

void init_os();
void scheduler();
void shell();
void loader(char * , char *);
int is_os_running();
void run_os();

// Memory management functions
int getFreePage();
void freePage(int );
int getPhysicallAddress(int , int , int );

#endif
