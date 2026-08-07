#include <stdio.h>
#include "processor.h"

int registers[256];
int pc;
int opcode , dest , src1 , src2;
int Z , N , C , V;
extern char instruction[256] , data[4096];
int end_of_simulation = 0;

void reset()
{
    for(int i = 0; i < 256; i++)
    {
        registers[i] = 0;
    }
    pc = 0;
}

void fetch()
{
    opcode = instruction[pc];
    dest = instruction[pc + 1];
    src1 = instruction[pc + 2];
    src2 = instruction[pc + 3];
    pc += 4;
}

void decode()
{

}

void execute()
{
    switch(opcode)
    {
        case 0 :
        {
            end_of_simulation = 1;
            break;
        }

        case 1 :
        {
            registers[dest] = registers[src1] + registers[src2];
            break;
        }

        case 2 :
        {
            registers[dest] = registers[src1] - registers[src2];
            break;
        }

        case 3 :
        {
            registers[dest] = registers[src1] * registers[src2];
            break;
        }

        case 4 :
        {
            registers[dest] = registers[src1] / registers[src2];
            break;
        }

        case 5 :
        {
            registers[dest] = data[src1];
            break;
        }

        case 6 :
        {
            data[src1] = registers[dest];
            break;
        }
    }
}