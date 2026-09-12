#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "processor.h"
#include "os.h"

char memory[MEMSIZE];
int isize = 1024 , dsize = 4096;

void initialize(int pid , char *program_byte_path , char *data_byte_path)
{
    if(pid < 0 || pid >= NP)
    {
        fprintf(stderr , "Invalid pid %d in initialize\n" , pid);
        return;
    }

    // Free any old pages if previously allocated
    for(int idx = 0 ; idx < NUM_LOGICAL_PAGES ; idx++)
    {
        int f = (unsigned char) pageTable[pid][idx];
        if(f > 0)
        {
            freePage(f);
            pageTable[pid][idx] = 0;
        }
    }

    // 1. Read program.byte and allocate instruction pages
    int op , dest , o1 , o2 , i = 0;
    FILE *pfile = fopen(program_byte_path , "r");
    if(pfile != NULL)
    {
        while(fscanf(pfile , "%X %X %X %X" , &op , &dest , &o1 , &o2) == 4 && i < isize)
        {
            int page_idx = i / PAGESIZE;
            if((unsigned char) pageTable[pid][page_idx] == 0)
            {
                int frame = getFreePage();
                if(frame == -1) break;
                pageTable[pid][page_idx] = frame;
                for(int b = 0 ; b < PAGESIZE ; b++)
                {
                    memory[frame * PAGESIZE + b] = 0;
                }
            }

            memory[getPhysicallAddress(pid , 1 , i++)] = (char) op;
            memory[getPhysicallAddress(pid , 1 , i++)] = (char) dest;
            memory[getPhysicallAddress(pid , 1 , i++)] = (char) o1;
            memory[getPhysicallAddress(pid , 1 , i++)] = (char) o2;
        }
        fclose(pfile);
    }

    // Ensure at least page 0 is allocated for instructions
    if((unsigned char) pageTable[pid][0] == 0)
    {
        int frame = getFreePage();
        if(frame != -1)
        {
            pageTable[pid][0] = frame;
            for(int b = 0 ; b < PAGESIZE ; b++)
            {
                memory[frame * PAGESIZE + b] = 0;
            }
        }
    }

    // 2. Read data.byte and allocate data pages
    i = 0;
    int j0 , j1 , j2 , j3;
    FILE *dfile = fopen(data_byte_path , "r");
    if(dfile != NULL)
    {
        while(fscanf(dfile , "%X %X %X %X" , &j0 , &j1 , &j2 , &j3) == 4 && i < dsize)
        {
            int page_idx = i / PAGESIZE + 1024 / PAGESIZE;
            if((unsigned char) pageTable[pid][page_idx] == 0)
            {
                int frame = getFreePage();
                if(frame == -1) break;
                pageTable[pid][page_idx] = frame;
                for(int b = 0 ; b < PAGESIZE ; b++)
                {
                    memory[frame * PAGESIZE + b] = 0;
                }
            }

            memory[getPhysicallAddress(pid , 0 , i++)] = (char) j0;
            memory[getPhysicallAddress(pid , 0 , i++)] = (char) j1;
            memory[getPhysicallAddress(pid , 0 , i++)] = (char) j2;
            memory[getPhysicallAddress(pid , 0 , i++)] = (char) j3;
        }
        fclose(dfile);
    }

    // Ensure at least first data page is allocated
    int first_data_page = 1024 / PAGESIZE;
    if((unsigned char) pageTable[pid][first_data_page] == 0)
    {
        int frame = getFreePage();
        if(frame != -1)
        {
            pageTable[pid][first_data_page] = frame;
            for(int b = 0 ; b < PAGESIZE ; b++)
            {
                memory[frame * PAGESIZE + b] = 0;
            }
        }
    }
}

void finalize(int pid , char *data_byte_path)
{
    if(pid < 0 || pid >= NP)
    {
        fprintf(stderr , "Invalid pid %d in finalize\n" , pid);
        return;
    }

    FILE *dfile = fopen(data_byte_path , "w");
    if(dfile == NULL)
    {
        fprintf(stderr , "Failed to open %s for writing in finalize\n" , data_byte_path);
        return;
    }

    char out[30];
    for(int i = 0 ; i < dsize ; i += 4)
    {
        unsigned char c0 = memory[getPhysicallAddress(pid , 0 , i)];
        unsigned char c1 = memory[getPhysicallAddress(pid , 0 , i + 1)];
        unsigned char c2 = memory[getPhysicallAddress(pid , 0 , i + 2)];
        unsigned char c3 = memory[getPhysicallAddress(pid , 0 , i + 3)];
        snprintf(out , sizeof(out) , "%X %X %X %X\n" , c0 , c1 , c2 , c3);
        fputs(out , dfile);
    }
    fclose(dfile);

    // Free all physical pages allocated to this task
    for(int idx = 0 ; idx < NUM_LOGICAL_PAGES ; idx++)
    {
        int frame = (unsigned char) pageTable[pid][idx];
        if(frame > 0)
        {
            freePage(frame);
            pageTable[pid][idx] = 0;
        }
    }
}