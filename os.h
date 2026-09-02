#ifndef OS_H
#define OS_H

#include "processor.h"

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

void init_os();
void scheduler();
void shell();
void loader(char * , char *);
int is_os_running();
void run_os();

#endif
