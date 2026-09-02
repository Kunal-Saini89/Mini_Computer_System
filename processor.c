#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "processor.h"

int registers[NP][256];
int pc[NP];
int opcode[NP] , dest[NP] , src1[NP] , src2[NP];
uint8_t flags[NP]; // 0 0 0 0 Z N C V
extern char instruction[NP][256] , data[NP][4096];
int end_of_simulation[NP] = {0};
FILE *fd_log = NULL;

typedef struct
{
    int i0;
    int i1;
    int i2;
    int i3;
    int i4;
    int i5;
    int i6;
    int i7;
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
    for(int i = 0; i < 256; i++)
    {
        registers[pid][i] = 0;
    }
    for(int i = 0; i < 32; i++)
    {
        vregisters[pid][i].i0 = 0;
        vregisters[pid][i].i1 = 0;
        vregisters[pid][i].i2 = 0;
        vregisters[pid][i].i3 = 0;
        vregisters[pid][i].i4 = 0;
        vregisters[pid][i].i5 = 0;
        vregisters[pid][i].i6 = 0;
        vregisters[pid][i].i7 = 0;
    }
    pc[pid] = 0;
    flags[pid] = 0;
    end_of_simulation[pid] = 0;
}

void fetch(int pid)
{
    if(pid < 0 || pid >= NP) return;
    opcode[pid] = (unsigned char) instruction[pid][pc[pid]];
    dest[pid] = (unsigned char) instruction[pid][pc[pid] + 1];
    src1[pid] = (unsigned char) instruction[pid][pc[pid] + 2];
    src2[pid] = (unsigned char) instruction[pid][pc[pid] + 3];
    pc[pid] += 4;
}

void decode(int pid)
{

}

void process_instructions(int pid, int instruction_count)
{
    if(pid < 0 || pid >= NP) return;
    for(int i = 0; i < instruction_count; i++)
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
    if(pc[pid] < 0 || pc[pid] > 256)
    {
        fprintf(stderr, "pc( %d ) out of bounds for pid %d\n" , pc[pid] , pid);
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
            unsigned char c0 = data[pid][addr++];
            unsigned char c1 = data[pid][addr++];
            unsigned char c2 = data[pid][addr++];
            unsigned char c3 = data[pid][addr++];

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
            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;
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
                fprintf(fd_log, "Process id: %d x%d : 0x%X\n", pid, reg_idx, val);
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
            unsigned char c0 = data[pid][addr++];
            unsigned char c1 = data[pid][addr++];
            unsigned char c2 = data[pid][addr++];
            unsigned char c3 = data[pid][addr++];

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
            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;
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

        case 0x21:
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            res.i0 = o1.i0 + o2.i0;
            res.i1 = o1.i1 + o2.i1;
            res.i2 = o1.i2 + o2.i2;
            res.i3 = o1.i3 + o2.i3;
            res.i4 = o1.i4 + o2.i4;
            res.i5 = o1.i5 + o2.i5;
            res.i6 = o1.i6 + o2.i6;
            res.i7 = o1.i7 + o2.i7;
            vregisters[pid][d] = res;
            break;
        }

        case 0x31:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            res.i0 = o1.i0 + o2;
            res.i1 = o1.i1 + o2;
            res.i2 = o1.i2 + o2;
            res.i3 = o1.i3 + o2;
            res.i4 = o1.i4 + o2;
            res.i5 = o1.i5 + o2;
            res.i6 = o1.i6 + o2;
            res.i7 = o1.i7 + o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x29:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            res.i0 = o1.i0 + o2;
            res.i1 = o1.i1 + o2;
            res.i2 = o1.i2 + o2;
            res.i3 = o1.i3 + o2;
            res.i4 = o1.i4 + o2;
            res.i5 = o1.i5 + o2;
            res.i6 = o1.i6 + o2;
            res.i7 = o1.i7 + o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x22:
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            res.i0 = o1.i0 - o2.i0;
            res.i1 = o1.i1 - o2.i1;
            res.i2 = o1.i2 - o2.i2;
            res.i3 = o1.i3 - o2.i3;
            res.i4 = o1.i4 - o2.i4;
            res.i5 = o1.i5 - o2.i5;
            res.i6 = o1.i6 - o2.i6;
            res.i7 = o1.i7 - o2.i7;
            vregisters[pid][d] = res;
            break;
        }

        case 0x32:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            res.i0 = o1.i0 - o2;
            res.i1 = o1.i1 - o2;
            res.i2 = o1.i2 - o2;
            res.i3 = o1.i3 - o2;
            res.i4 = o1.i4 - o2;
            res.i5 = o1.i5 - o2;
            res.i6 = o1.i6 - o2;
            res.i7 = o1.i7 - o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x2A:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            res.i0 = o1.i0 - o2;
            res.i1 = o1.i1 - o2;
            res.i2 = o1.i2 - o2;
            res.i3 = o1.i3 - o2;
            res.i4 = o1.i4 - o2;
            res.i5 = o1.i5 - o2;
            res.i6 = o1.i6 - o2;
            res.i7 = o1.i7 - o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x23:
        {
            vector o1 = vregisters[pid][s1] , o2 = vregisters[pid][s2];
            vector res;
            res.i0 = o1.i0 * o2.i0;
            res.i1 = o1.i1 * o2.i1;
            res.i2 = o1.i2 * o2.i2;
            res.i3 = o1.i3 * o2.i3;
            res.i4 = o1.i4 * o2.i4;
            res.i5 = o1.i5 * o2.i5;
            res.i6 = o1.i6 * o2.i6;
            res.i7 = o1.i7 * o2.i7;
            vregisters[pid][d] = res;
            break;
        }

        case 0x33:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = registers[pid][s2];
            vector res;
            res.i0 = o1.i0 * o2;
            res.i1 = o1.i1 * o2;
            res.i2 = o1.i2 * o2;
            res.i3 = o1.i3 * o2;
            res.i4 = o1.i4 * o2;
            res.i5 = o1.i5 * o2;
            res.i6 = o1.i6 * o2;
            res.i7 = o1.i7 * o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x2B:
        {
            vector o1 = vregisters[pid][s1];
            int o2 = s2;
            vector res;
            res.i0 = o1.i0 * o2;
            res.i1 = o1.i1 * o2;
            res.i2 = o1.i2 * o2;
            res.i3 = o1.i3 * o2;
            res.i4 = o1.i4 * o2;
            res.i5 = o1.i5 * o2;
            res.i6 = o1.i6 * o2;
            res.i7 = o1.i7 * o2;
            vregisters[pid][d] = res;
            break;
        }

        case 0x25:
        {
            vector res;
            int addr = registers[pid][s2];
            unsigned char c0 = data[pid][addr++];
            unsigned char c1 = data[pid][addr++];
            unsigned char c2 = data[pid][addr++];
            unsigned char c3 = data[pid][addr++];

            int a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i0 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i1 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i2 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i3 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i4 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i5 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i6 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i7 = a;

            vregisters[pid][d] = res;
            break;
        }

        case 0x2C:
        {
            vector res;
            int addr = s2;
            unsigned char c0 = data[pid][addr++];
            unsigned char c1 = data[pid][addr++];
            unsigned char c2 = data[pid][addr++];
            unsigned char c3 = data[pid][addr++];

            int a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i0 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i1 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i2 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i3 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i4 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i5 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i6 = a;

            c0 = data[pid][addr++];
            c1 = data[pid][addr++];
            c2 = data[pid][addr++];
            c3 = data[pid][addr++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i7 = a;

            vregisters[pid][d] = res;
            break;
        }

        case 0x26:
        {
            vector vec = vregisters[pid][s2];
            int o2 = vec.i0;
            int addr = registers[pid][d];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i1;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i2;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i3;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i4;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i5;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i6;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i7;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;
            break;
        }

        case 0x2E:
        {
            vector vec = vregisters[pid][s2];
            int o2 = vec.i0;

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            int addr = d;
            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i1;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i2;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i3;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i4;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i5;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i6;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;

            o2 = vec.i7;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[pid][addr++] = c0;
            data[pid][addr++] = c1;
            data[pid][addr++] = c2;
            data[pid][addr++] = c3;
            break;
        }
    }
}