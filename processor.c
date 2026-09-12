#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "processor.h"
#include "memory.h"
#include "os.h"

int registers[NP][256];
int pc[NP];
int opcode[NP] , dest[NP] , src1[NP] , src2[NP];
uint8_t flags[NP]; // 0 0 0 0 Z N C V
int end_of_simulation[NP] = {0};
FILE *fd_log = NULL;

typedef struct
{
    int i[8];
} vector;

vector vregisters[NP][32];

int get_bit(uint8_t byte_val, int index)
{
    return (byte_val >> index) & 1;
}

void init_processor()
{
    if(fd_log == NULL)
    {
        fd_log = fopen("execution.log" , "a");
    }
}

void close_processor()
{
    if(fd_log != NULL)
    {
        fclose(fd_log);
        fd_log = NULL;
    }
}

void reset(int pid)
{
    if(pid < 0 || pid >= NP) return;
    init_processor();
    for(int i = 0 ; i < 256 ; i++)
    {
        registers[pid][i] = 0;
    }
    for(int i = 0 ; i < 32 ; i++)
    {
        for(int j = 0 ; j < 8 ; j++)
        {
            vregisters[pid][i].i[j] = 0;
        }
    }
    pc[pid] = 0;
    flags[pid] = 0;
    end_of_simulation[pid] = 0;
}

void fetch(int pid)
{
    if(pid < 0 || pid >= NP) return;
    int addr = pc[pid];
    opcode[pid] = (unsigned char) memory[getPhysicallAddress(pid , 1 , addr)];
    dest[pid] = (unsigned char) memory[getPhysicallAddress(pid , 1 , addr + 1)];
    src1[pid] = (unsigned char) memory[getPhysicallAddress(pid , 1 , addr + 2)];
    src2[pid] = (unsigned char) memory[getPhysicallAddress(pid , 1 , addr + 3)];
    pc[pid] += 4;
}

void decode(int pid)
{

}

void process_instructions(int pid, int instruction_count)
{
    if(pid < 0 || pid >= NP) return;
    for(int i = 0 ; i < instruction_count ; i++)
    {
        if(end_of_simulation[pid])
        {
            break;
        }
        fetch(pid);
        decode(pid);
        execute(pid);
    }
    usleep(10);
}

void execute(int pid)
{
    if(pid < 0 || pid >= NP) return;
    if(pc[pid] < 0 || pc[pid] > 1024)
    {
        fprintf(stderr , "pc( %d ) out of bounds for pid %d\n" , pc[pid] , pid);
        end_of_simulation[pid] = 1;
        return;
    }
    int op = opcode[pid];
    int d = dest[pid];
    int s1 = src1[pid];
    int s2 = src2[pid];

    switch(op)
    {
        case 0x00 :
        {
            end_of_simulation[pid] = 1;
            break;
        }

        case 0x01 :
        {
            int o1 = registers[pid][s1] , o2 = registers[pid][s2];
            int res = o1 + o2;
            registers[pid][d] = res;
            if(res == 0) flags[pid] = flags[pid] | 0b00001000;
            if(res < 0) flags[pid] = flags[pid] | 0b00000100;
            if((unsigned int) res < o1 || (unsigned int) res < o2) flags[pid] = flags[pid] | 0b00000010;
            if(((o1 ^ o2) >= 0) && ((res ^ o1) < 0)) flags[pid] = flags[pid] | 0b00000001;
            break;
        }

        case 0x02 :
        {
            int o1 = registers[pid][s1] , o2 = registers[pid][s2];
            int res = o1 - o2;
            registers[pid][d] = res;
            if(res == 0) flags[pid] = flags[pid] | 0b00001000;
            if(res < 0) flags[pid] = flags[pid] | 0b00000100;
            if(o1 > o2) flags[pid] = flags[pid] | 0b00000010;
            if(((o1 ^ o2) < 0) && ((res ^ o2) >= 0)) flags[pid] = flags[pid] | 0b00000001;
            break;
        }

        case 0x03 :
        {
            int o1 = registers[pid][s1] , o2 = registers[pid][s2];
            registers[pid][d] = o1 * o2;
            break;
        }

        case 0x04 :
        {
            int o1 = registers[pid][s1] , o2 = registers[pid][s2];
            if(o2 != 0)
            {
                int res = o1 / o2;
                registers[pid][d] = res;
            }
            break;
        }

        case 0x05 :
        {
            int addr = registers[pid][s2];
            unsigned char c0 = memory[getPhysicallAddress(pid , 0 , addr)];
            unsigned char c1 = memory[getPhysicallAddress(pid , 0 , addr + 1)];
            unsigned char c2 = memory[getPhysicallAddress(pid , 0 , addr + 2)];
            unsigned char c3 = memory[getPhysicallAddress(pid , 0 , addr + 3)];

            int o2 = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            registers[pid][d] = o2;
            break;
        }

        case 0x06 :
        {
            int o2 = registers[pid][s2];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);
            
            int addr = registers[pid][d];
            memory[getPhysicallAddress(pid , 0 , addr)] = c0;
            memory[getPhysicallAddress(pid , 0 , addr + 1)] = c1;
            memory[getPhysicallAddress(pid , 0 , addr + 2)] = c2;
            memory[getPhysicallAddress(pid , 0 , addr + 3)] = c3;
            break;
        }

        case 0x07 :
        {
            int o2 = registers[pid][s2];
            registers[pid][d] = o2;
            break;
        }

        case 0x08 : // Print instruction
        {
            int reg_idx = s2;
            int val = registers[pid][reg_idx];
            if(fd_log != NULL)
            {
                fprintf(fd_log , "Process id: %d x%d : 0x%X\n" , pid , reg_idx , val);
                fflush(fd_log);
            }
            break;
        }

        case 0x09 :
        {
            int o1 = registers[pid][s1] , o2 = s2;
            int res = o1 + o2;
            registers[pid][d] = res;
            if(res == 0) flags[pid] = flags[pid] | 0b00001000;
            if(res < 0) flags[pid] = flags[pid] | 0b00000100;
            if((unsigned int) res < o1 || (unsigned int) res < o2) flags[pid] = flags[pid] | 0b00000010;
            if(((o1 ^ o2) >= 0) && ((res ^ o1) < 0)) flags[pid] = flags[pid] | 0b00000001;
            break;
        }

        case 0x0A :
        {
            int o1 = registers[pid][s1] , o2 = s2;
            int res = o1 - o2;
            registers[pid][d] = res;
            if(res == 0) flags[pid] = flags[pid] | 0b00001000;
            if(res < 0) flags[pid] = flags[pid] | 0b00000100;
            if(o1 > o2) flags[pid] = flags[pid] | 0b00000010;
            if(((o1 ^ o2) < 0) && ((res ^ o2) >= 0)) flags[pid] = flags[pid] | 0b00000001;
            break;
        }

        case 0x0B :
        {
            int o1 = registers[pid][s1] , o2 = s2;
            registers[pid][d] = o1 * o2;
            break;
        }

        case 0x0C :
        {
            int o1 = registers[pid][s1] , o2 = s2;
            if(o2 != 0)
            {
                int res = o1 / o2;
                registers[pid][d] = res;
            }
            break;
        }

        case 0x0D :
        {
            int addr = s2;
            unsigned char c0 = memory[getPhysicallAddress(pid , 0 , addr)];
            unsigned char c1 = memory[getPhysicallAddress(pid , 0 , addr + 1)];
            unsigned char c2 = memory[getPhysicallAddress(pid , 0 , addr + 2)];
            unsigned char c3 = memory[getPhysicallAddress(pid , 0 , addr + 3)];

            int o2 = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            registers[pid][d] = o2;
            break;
        }

        case 0x0E :
        {
            int o2 = registers[pid][s2];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            int addr = d;
            memory[getPhysicallAddress(pid , 0 , addr)] = c0;
            memory[getPhysicallAddress(pid , 0 , addr + 1)] = c1;
            memory[getPhysicallAddress(pid , 0 , addr + 2)] = c2;
            memory[getPhysicallAddress(pid , 0 , addr + 3)] = c3;
            break;
        }

        case 0x0F :
        {
            int o2 = s2;
            registers[pid][d] = o2;
            break;
        }

        case 0x10 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00001000) == 0b00001000)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x11 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00001000) != 0b00001000)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x12 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000010) == 0b00000010)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x13 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000010) != 0b00000010)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x14 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000100) == 0b00000100)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x15 : 
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000100) != 0b00000100)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x16 :
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000001) == 0b00000001)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x17 : 
        {
            int o2 = (signed char)s2 * 4;
            if((flags[pid] & 0b00000001) != 0b00000001)
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x18 :
        {
            int o2 = (signed char)s2 * 4;
            if(((flags[pid] & 0b00001000) != 0b00001000) && ((flags[pid] & 0b00000010) == 0b00000010))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x19 : 
        {
            int o2 = (signed char)s2 * 4;
            if(((flags[pid] & 0b00001000) == 0b00001000) && ((flags[pid] & 0b00000010) != 0b00000010))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x1A :
        {
            int o2 = (signed char)s2 * 4;
            if(get_bit(flags[pid] , 2) == get_bit(flags[pid] , 0))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x1B : 
        {
            int o2 = (signed char)s2 * 4;
            if(get_bit(flags[pid] , 2) != get_bit(flags[pid] , 0))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x1C : 
        {
            int o2 = (signed char)s2 * 4;
            if((get_bit(flags[pid] , 3) == 0) && get_bit(flags[pid] , 2) == get_bit(flags[pid] , 0))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x1D :
        {
            int o2 = (signed char)s2 * 4;
            if((get_bit(flags[pid] , 3) == 1) || get_bit(flags[pid] , 2) != get_bit(flags[pid] , 0))
            {
                pc[pid] += o2;
            }
            break;
        }

        case 0x1E :
        {
            int o2 = (signed char)s2 * 4;
            pc[pid] += o2;
            break;
        }

        case 0x21 : // vector + vector
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] + o2.i[k];
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x31 : // vector + integer reg
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] + o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x29 : // vector + constant
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] + o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x22 : // vector - vector
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] - o2.i[k];
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x32 : // vector - integer reg
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] - o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x2A : // vector - constant
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] - o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x23 : // vector * vector
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] * o2.i[k];
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x33 : // vector * integer reg
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] * o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x2B : // vector * constant
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            for(int k = 0 ; k < 8 ; k++)
            {
                res.i[k] = o1.i[k] * o2;
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x25 : // vector read from [reg]
        {
            vector res;
            int addr = registers[pid][s2];
            for(int k = 0 ; k < 8 ; k++)
            {
                int elem_addr = addr + k * 4;
                unsigned char c0 = memory[getPhysicallAddress(pid , 0 , elem_addr)];
                unsigned char c1 = memory[getPhysicallAddress(pid , 0 , elem_addr + 1)];
                unsigned char c2 = memory[getPhysicallAddress(pid , 0 , elem_addr + 2)];
                unsigned char c3 = memory[getPhysicallAddress(pid , 0 , elem_addr + 3)];
                res.i[k] = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x2C : // vector read from [const]
        {
            vector res;
            int addr = s2;
            for(int k = 0 ; k < 8 ; k++)
            {
                int elem_addr = addr + k * 4;
                unsigned char c0 = memory[getPhysicallAddress(pid , 0 , elem_addr)];
                unsigned char c1 = memory[getPhysicallAddress(pid , 0 , elem_addr + 1)];
                unsigned char c2 = memory[getPhysicallAddress(pid , 0 , elem_addr + 2)];
                unsigned char c3 = memory[getPhysicallAddress(pid , 0 , elem_addr + 3)];
                res.i[k] = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            }
            vregisters[pid][d] = res;
            break;
        }

        case 0x26 : // vector write to [reg]
        {
            vector vec = vregisters[pid][s2];
            int addr = registers[pid][d];
            for(int k = 0 ; k < 8 ; k++)
            {
                int elem_addr = addr + k * 4;
                int val = vec.i[k];
                memory[getPhysicallAddress(pid , 0 , elem_addr)] = (unsigned char) val;
                memory[getPhysicallAddress(pid , 0 , elem_addr + 1)] = (unsigned char) (val >> 8);
                memory[getPhysicallAddress(pid , 0 , elem_addr + 2)] = (unsigned char) (val >> 16);
                memory[getPhysicallAddress(pid , 0 , elem_addr + 3)] = (unsigned char) (val >> 24);
            }
            break;
        }

        case 0x2E : // vector write to [const]
        {
            vector vec = vregisters[pid][s2];
            int addr = d;
            for(int k = 0 ; k < 8 ; k++)
            {
                int elem_addr = addr + k * 4;
                int val = vec.i[k];
                memory[getPhysicallAddress(pid , 0 , elem_addr)] = (unsigned char) val;
                memory[getPhysicallAddress(pid , 0 , elem_addr + 1)] = (unsigned char) (val >> 8);
                memory[getPhysicallAddress(pid , 0 , elem_addr + 2)] = (unsigned char) (val >> 16);
                memory[getPhysicallAddress(pid , 0 , elem_addr + 3)] = (unsigned char) (val >> 24);
            }
            break;
        }
    }
}