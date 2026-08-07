#include <stdio.h>
#include "memory.h"

char instruction[256] , data[4096];
int isize = sizeof(instruction) / sizeof(instruction[0]) , dsize = sizeof(data) / sizeof(data[0]);


void initialize() //Intialize instruction and data memory
{
    int op , dest , o1 , o2 , i = 0 , j;
    fseek(ofile , 0 , SEEK_SET); // to bring the file internal pointer to the start of the file for reading of the byte code 
    while(fscanf(ofile , "%d %d %d %d" , &op , &dest , &o1 , &o2) == 4) //Setting up instruction array
    {
        instruction[i++] = (char) op;
        instruction[i++] = (char) dest;
        instruction[i++] = (char) o1;
        instruction[i++] = (char) o2;
    }
    for(; i < isize ; i++)
    {
        instruction[i] = 0;
    }
    
    i = 0;
    while(fscanf(dfile , "%d" , &j) == 1) // setting up data array 
    {
        data[i++] = j;
    }
    for(; i < dsize ;i++)
    {
        data[i] = 0;
    }
}

void finalize() // Finalize data.byte
{
    fseek(dfile , 0 , SEEK_SET); //Bring file's internal pointer to the start of the file to output the data into the data.byte file 
    char out[20];
    for(int i = 0 ; i < dsize ; i++)
    {
        snprintf(out , sizeof(out) , "%d\n" , (unsigned char) data[i]);
        fputs(out , dfile);
    }
}