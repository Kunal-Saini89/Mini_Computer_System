#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "processor.h"

char instruction[NP][256] , data[NP][4096];
int isize = 256 , dsize = 4096;


void initialize(int pid , char *program_byte_path , char *data_byte_path) //Intialize instruction and data memory
{
    if(pid < 0 || pid >= NP)
    {
        fprintf(stderr , "Invalid pid %d in initialize\n" , pid);
        return;
    }

    int op , dest , o1 , o2 , i = 0;
    FILE *pfile = fopen(program_byte_path , "r");
    if(pfile != NULL)
    {
        while(fscanf(pfile , "%X %X %X %X" , &op , &dest , &o1 , &o2) == 4 && i < isize) //Setting up instruction array
        {
            instruction[pid][i++] = (char) op;
            instruction[pid][i++] = (char) dest;
            instruction[pid][i++] = (char) o1;
            instruction[pid][i++] = (char) o2;
        }
        fclose(pfile);
    }
    for(; i < isize ; i++)
    {
        instruction[pid][i] = 0;
    }
    
    i = 0;
    int j0 , j1 , j2 , j3;
    FILE *dfile = fopen(data_byte_path , "r");
    if(dfile != NULL)
    {
        while(fscanf(dfile , "%X %X %X %X" , &j0 , &j1 , &j2 , &j3) == 4 && i < dsize) // setting up data array 
        {
            data[pid][i++] = j0;
            data[pid][i++] = j1;
            data[pid][i++] = j2;
            data[pid][i++] = j3;
        }
        fclose(dfile);
    }
    for(; i < dsize ; i++)
    {
        data[pid][i] = 0;
    }
}

void finalize(int pid , char *data_byte_path) // Finalize data.byte
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

    char out[20];
    for(int i = 0 ; i < dsize ; i += 4)
    {
        snprintf(out , sizeof(out) , "%X %X %X %X\n" , (unsigned char) data[pid][i] , (unsigned char) data[pid][i + 1] , (unsigned char) data[pid][i + 2] , (unsigned char) data[pid][i + 3]);
        fputs(out , dfile);
    }
    fclose(dfile);
}