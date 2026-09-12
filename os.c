#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "os.h"
#include "Compiler.h"
#include "processor.h"
#include "memory.h"

task task_list[maxtasksupport];
int proc_pid[NP];
char pageTable[NP][NUM_LOGICAL_PAGES];
char freePages[NUM_PHYSICAL_PAGES];
int shell_exit = 0;
int next_pid = 1;
char shell_input[100];
int shell_index = 0;
int prompt_done = 0;

int getFreePage()
{
    // OS maintains list of free pages in main memory.
    // Frame 0 is reserved and will never be allocated.
    for(int i = 1 ; i < NUM_PHYSICAL_PAGES ; i++)
    {
        if(freePages[i] == 0)
        {
            freePages[i] = 1;
            return i;
        }
    }
    fprintf(stderr , "ERROR: Out of physical memory frames!\n");
    return -1;
}

void freePage(int frame)
{
    if(frame > 0 && frame < NUM_PHYSICAL_PAGES)
    {
        freePages[frame] = 0;
    }
}

int getPhysicallAddress(int proc_id , int isFetch , int address)
{
    if(proc_id < 0 || proc_id >= NP)
    {
        return 0;
    }
    // Index = (isFetch) ? address / PAGESIZE : address / PAGESIZE + 1024 / PAGESIZE;
    int index = (isFetch) ? (address / PAGESIZE) : (address / PAGESIZE + 1024 / PAGESIZE);
    if(index < 0 || index >= NUM_LOGICAL_PAGES)
    {
        return 0;
    }
    int physical_page = (unsigned char) pageTable[proc_id][index];
    int physical_address = physical_page * PAGESIZE + (address % PAGESIZE);
    return physical_address;
}

void init_os()
{
    for(int i = 0 ; i < maxtasksupport ; i++)
    {
        task_list[i].pid = 0;
        task_list[i].proc_id = -1;
        task_list[i].input_file[0] = '\0';
        task_list[i].program_byte_file[0] = '\0';
        task_list[i].data_file[0] = '\0';
        task_list[i].state = 0;
    }
    for(int i = 0 ; i < NP ; i++)
    {
        proc_pid[i] = -1;
        for(int j = 0 ; j < NUM_LOGICAL_PAGES ; j++)
        {
            pageTable[i][j] = 0;
        }
    }

    // Frame 0 is reserved
    freePages[0] = 1;
    for(int i = 1 ; i < NUM_PHYSICAL_PAGES ; i++)
    {
        freePages[i] = 0;
    }

    shell_exit = 0;
    next_pid = 1;
    shell_index = 0;
    prompt_done = 0;

    int flags = fcntl(STDIN_FILENO , F_GETFL , 0);
    if(flags != -1)
    {
        fcntl(STDIN_FILENO , F_SETFL , flags | O_NONBLOCK);
    }
    init_processor();
}

void loader(char *input_file , char *data_file)
{
    if(input_file == NULL || input_file[0] == '\0') return;

    int free_index = -1;
    for(int i = 0 ; i < maxtasksupport ; i++)
    {
        if(task_list[i].state == 0 || task_list[i].state == 4)
        {
            free_index = i;
            break;
        }
    }
    if(free_index == -1)
    {
        fprintf(stderr , "No free task slots available\n");
        return;
    }

    int cur_pid = next_pid++;
    task_list[free_index].pid = cur_pid;
    strcpy(task_list[free_index].input_file , input_file);

    if(data_file != NULL && data_file[0] != '\0')
    {
        strcpy(task_list[free_index].data_file , data_file);
    }
    else
    {
        strcpy(task_list[free_index].data_file , "data.byte");
    }

    snprintf(task_list[free_index].program_byte_file , sizeof(task_list[free_index].program_byte_file) , "program_%d.byte" , cur_pid);

    if(startcompiler(task_list[free_index].input_file , task_list[free_index].program_byte_file) != 0)
    {
        fprintf(stderr , "Compilation failed for : %s\n" , input_file);
        task_list[free_index].state = 4;
        return;
    }

    int free_proc = -1;
    for(int p = 0 ; p < NP ; p++)
    {
        if(proc_pid[p] == -1)
        {
            free_proc = p;
            break;
        }
    }

    if(free_proc != -1)
    {
        task_list[free_index].proc_id = free_proc;
        task_list[free_index].state = 2; // ready
        proc_pid[free_proc] = cur_pid;
        initialize(free_proc , task_list[free_index].program_byte_file , task_list[free_index].data_file);
        reset(free_proc);
        printf("Task PID %d loaded onto Processor %d\n" , cur_pid , free_proc);
    }
    else
    {
        task_list[free_index].proc_id = -1;
        task_list[free_index].state = 1; // waiting
        printf("All processors busy. Task PID %d queued in waiting state\n" , cur_pid);
    }
}

void shell()
{
    if(shell_exit == 1) return;

    if(prompt_done == 0)
    {
        printf("$ ");
        fflush(stdout);
        prompt_done = 1;
    }

    char ch;
    ssize_t n;
    while((n = read(STDIN_FILENO , &ch , 1)) > 0)
    {
        if(ch == '\n' || ch == '\r')
        {
            shell_input[shell_index] = '\0';
            int crop = getinstcropindex(shell_input , sizeof(shell_input));
            shell_input[crop] = '\0';
            prompt_done = 0;

            char *cmd = shell_input;
            while(isspace((unsigned char) *cmd)) cmd++;

            if(cmd[0] != '\0')
            {
                if(strcmp(cmd , "exit") == 0)
                {
                    printf("Shell exit requested. Waiting for active tasks to finish...\n");
                    shell_exit = 1;
                    shell_index = 0;
                    return;
                }
                else
                {
                    char prog[100] = {0} , dfile[100] = {0};
                    int count = sscanf(cmd , "%99s %99s" , prog , dfile);
                    if(count == 1)
                    {
                        loader(prog , "data.byte");
                    }
                    else if(count >= 2)
                    {
                        loader(prog , dfile);
                    }
                }
            }
            shell_index = 0;
            break;
        }
        else if(ch == 127 || ch == '\b')
        {
            if(shell_index > 0)
            {
                shell_index--;
            }
        }
        else
        {
            if(shell_index < (int)sizeof(shell_input) - 1)
            {
                shell_input[shell_index++] = ch;
            }
        }
    }

    if(n == 0) // EOF
    {
        shell_exit = 1;
    }
}

void scheduler()
{
    for(int i = 0 ; i < maxtasksupport ; i++)
    {
        if(task_list[i].state == 2 || task_list[i].state == 3) // ready or running
        {
            int p = task_list[i].proc_id;
            if(p >= 0 && p < NP)
            {
                task_list[i].state = 3;
                process_instructions(p , 10);
                if(end_of_simulation[p] == 1)
                {
                    finalize(p , task_list[i].data_file);
                    task_list[i].state = 4; // finished
                    proc_pid[p] = -1;
                    printf("Task PID %d finished on Processor %d. Data written to %s\n" , task_list[i].pid , p , task_list[i].data_file);

                    for(int w = 0 ; w < maxtasksupport ; w++)
                    {
                        if(task_list[w].state == 1) // waiting
                        {
                            task_list[w].proc_id = p;
                            task_list[w].state = 2; // ready
                            proc_pid[p] = task_list[w].pid;
                            initialize(p , task_list[w].program_byte_file , task_list[w].data_file);
                            reset(p);
                            printf("Waiting task PID %d assigned to Processor %d\n" , task_list[w].pid , p);
                            break;
                        }
                    }
                }
                else
                {
                    task_list[i].state = 2; // back to ready
                }
            }
        }
    }
    shell();
}

int is_os_running()
{
    if(shell_exit == 0) return 1;

    for(int i = 0 ; i < maxtasksupport ; i++)
    {
        if(task_list[i].state == 1 || task_list[i].state == 2 || task_list[i].state == 3)
        {
            return 1;
        }
    }
    return 0;
}

void run_os()
{
    init_os();
    while(is_os_running())
    {
        scheduler();
        sleep(20);
    }
    close_processor();
}
