#include <stdio.h>
#include <stdlib.h>
#include "processor.h"

int registers[256];
int pc;
int opcode , dest , src1 , src2;
uint8_t flags; // 0 0 0 0 Z N C V
extern char instruction[256] , data[4096];
int end_of_simulation = 0;

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
}vector;

vector vregisters[32];

int get_bit(uint8_t byte_val, int index)
{
    return (byte_val >> index) & 1;
}

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
    if(pc < 0 || pc >= 255)
    {
        fprintf(stderr, "pc( %d ) out of bounds\n" , pc);
        exit(EXIT_FAILURE);
    }
    switch(opcode)
    {
        case 0x00 :
        {
            end_of_simulation = 1;
            break;
        }

        case 0x01 :
        {
            int o1 = registers[src1] , o2 = registers[src2];
            int res = o1 + o2;
            registers[dest] = res;
            if(res == 0) flags = flags | 0b00001000;
            if(res < 0) flags = flags | 0b00000100;
            if((unsigned int) res < o1 || (unsigned int) res < o2) flags = flags | 0b00000010;
            if(((o1 ^ o2) >= 0) && ((res ^ o1) < 0)) flags = flags | 0b00000001;
            break;
        }

        case 0x02 :
        {
            int o1 = registers[src1] , o2 = registers[src2];
            int res = o1 - o2;
            registers[dest] = res;
            if(res == 0) flags = flags | 0b00001000;
            if(res < 0) flags = flags | 0b00000100;
            if(o1 > o2) flags = flags | 0b00000010;
            if(((o1 ^ o2) < 0) && ((res ^ o2) >= 0)) flags = flags | 0b00000001;
            break;
        }

        case 0x03 :
        {
            int o1 = registers[src1] , o2 = registers[src2];
            registers[dest] = o1 * o2;
            break;
        }

        case 0x04 :
        {
            int o1 = registers[src1] , o2 = registers[src2];
            int res = o1 / o2;
            registers[dest] = res;
            break;
        }

        case 0x05 :
        {
            src2 = registers[src2];
            unsigned char c0 = data[src2++];
            unsigned char c1 = data[src2++];
            unsigned char c2 = data[src2++];
            unsigned char c3 = data[src2++];

            int o2 = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            registers[dest] = o2;
            break;
        }

        case 0x06 :
        {
            int o2 = registers[src2];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);
            
            dest = registers[dest];
            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;
            break;
        }

        case 0x07 :
        {
            int o2 = registers[src2];
            registers[dest] = o2;
            break;
        }

        case 0x09 :
        {
            int o1 = registers[src1] , o2 = src2;
            int res = o1 + o2;
            registers[dest] = res;
            if(res == 0) flags = flags | 0b00001000;
            if(res < 0) flags = flags | 0b00000100;
            if((unsigned int) res < o1 || (unsigned int) res < o2) flags = flags | 0b00000010;
            if(((o1 ^ o2) >= 0) && ((res ^ o1) < 0)) flags = flags | 0b00000001;
            break;
        }

        case 0x0A :
        {
            int o1 = registers[src1] , o2 = src2;
            int res = o1 - o2;
            registers[dest] = res;
            if(res == 0) flags = flags | 0b00001000;
            if(res < 0) flags = flags | 0b00000100;
            if(o1 > o2) flags = flags | 0b00000010;
            if(((o1 ^ o2) < 0) && ((res ^ o2) >= 0)) flags = flags | 0b00000001;
            break;
        }

        case 0x0B :
        {
            int o1 = registers[src1] , o2 = src2;
            registers[dest] = o1 * o2;
            break;
        }

        case 0x0C :
        {
            int o1 = registers[src1] , o2 = src2;
            int res = o1 / o2;
            registers[dest] = res;
            break;
        }

        case 0x0D :
        {
            unsigned char c0 = data[src2++];
            unsigned char c1 = data[src2++];
            unsigned char c2 = data[src2++];
            unsigned char c3 = data[src2++];

            int o2 = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            registers[dest] = o2;
            break;
        }

        case 0x0E :
        {
            int o2 = registers[src2];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;
            break;
        }

        case 0x0F :
        {
            int o2 = src2;
            registers[dest] = o2;
            break;
        }

        case 0x10 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00001000) == 0b00001000)
            {
                pc += o2;
            }
            break;
        }

        case 0x11 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00001000) != 0b00001000)
            {
                pc += o2;
            }
            break;
        }

        case 0x12 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00000010) == 0b00000010)
            {
                pc += o2;
            }
            break;
        }

        case 0x13 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00000010) != 0b00000010)
            {
                pc += o2;
            }
            break;
        }

        case 0x14 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00000100) == 0b00000100)
            {
                pc += o2;
            }
            break;
        }

        case 0x15 : 
        {
            int o2 = src2 * 4;
            if((flags & 0b00000100) != 0b00000100)
            {
                pc += o2;
            }
            break;
        }

        case 0x16 :
        {
            int o2 = src2 * 4;
            if((flags & 0b00000001) == 0b00000001)
            {
                pc += o2;
            }
            break;
        }

        case 0x17 : 
        {
            int o2 = src2 * 4;
            if((flags & 0b00000001) != 0b00000001)
            {
                pc += o2;
            }
            break;
        }

        case 0x18 :
        {
            int o2 = src2 * 4;
            if(((flags & 0b00001000) != 0b00001000) && ((flags & 0b00000010) == 0b00000010))
            {
                pc += o2;
            }
            break;
        }

        case 0x19 : 
        {
            int o2 = src2 * 4;
            if(((flags & 0b00001000) == 0b00001000) && ((flags & 0b00000010) != 0b00000010))
            {
                pc += o2;
            }
            break;
        }

        case 0x1A :
        {
            int o2 = src2 * 4;
            if(get_bit(flags , 2) == get_bit(flags , 0))
            {
                pc += o2;
            }
            break;
        }

        case 0x1B : 
        {
            int o2 = src2 * 4;
            if(get_bit(flags , 2) != get_bit(flags , 0))
            {
                pc += o2;
            }
            break;
        }

        case 0x1C : 
        {
            int o2 = src2 * 4;
            if((get_bit(flags , 3) == 0) && get_bit(flags , 2) == get_bit(flags , 0))
            {
                pc += o2;
            }
            break;
        }

        case 0x1D :
        {
            int o2 = src2 * 4;
            if((get_bit(flags , 3) == 1) || get_bit(flags , 2) != get_bit(flags , 0))
            {
                pc += o2;
            }
            break;
        }

        case 0x1E :
        {
            int o2 = src2 * 4;
            pc += o2;
            break;
        }

        case 0x21:
        {
            vector o1 = vregisters[src1] , o2 = vregisters[src2];
            vector res;
            res.i0 = o1.i0 + o2.i0;
            res.i1 = o1.i1 + o2.i1;
            res.i2 = o1.i2 + o2.i2;
            res.i3 = o1.i3 + o2.i3;
            res.i4 = o1.i4 + o2.i4;
            res.i5 = o1.i5 + o2.i5;
            res.i6 = o1.i6 + o2.i6;
            res.i7 = o1.i7 + o2.i7;
            vregisters[dest] = res;
            break;
        }

        case 0x31:
        {
            vector o1 = vregisters[src1];
            int o2 = registers[src2];
            vector res;
            res.i0 = o1.i0 + o2;
            res.i1 = o1.i1 + o2;
            res.i2 = o1.i2 + o2;
            res.i3 = o1.i3 + o2;
            res.i4 = o1.i4 + o2;
            res.i5 = o1.i5 + o2;
            res.i6 = o1.i6 + o2;
            res.i7 = o1.i7 + o2;
            vregisters[dest] = res;
            break;
        }

        case 0x29:
        {
            vector o1 = vregisters[src1];
            int o2 = src2;
            vector res;
            res.i0 = o1.i0 + o2;
            res.i1 = o1.i1 + o2;
            res.i2 = o1.i2 + o2;
            res.i3 = o1.i3 + o2;
            res.i4 = o1.i4 + o2;
            res.i5 = o1.i5 + o2;
            res.i6 = o1.i6 + o2;
            res.i7 = o1.i7 + o2;
            vregisters[dest] = res;
            break;
        }

        case 0x22:
        {
            vector o1 = vregisters[src1] , o2 = vregisters[src2];
            vector res;
            res.i0 = o1.i0 - o2.i0;
            res.i1 = o1.i1 - o2.i1;
            res.i2 = o1.i2 - o2.i2;
            res.i3 = o1.i3 - o2.i3;
            res.i4 = o1.i4 - o2.i4;
            res.i5 = o1.i5 - o2.i5;
            res.i6 = o1.i6 - o2.i6;
            res.i7 = o1.i7 - o2.i7;
            vregisters[dest] = res;
            break;
        }

        case 0x32:
        {
            vector o1 = vregisters[src1];
            int o2 = registers[src2];
            vector res;
            res.i0 = o1.i0 - o2;
            res.i1 = o1.i1 - o2;
            res.i2 = o1.i2 - o2;
            res.i3 = o1.i3 - o2;
            res.i4 = o1.i4 - o2;
            res.i5 = o1.i5 - o2;
            res.i6 = o1.i6 - o2;
            res.i7 = o1.i7 - o2;
            vregisters[dest] = res;
            break;
        }

        case 0x2A:
        {
            vector o1 = vregisters[src1];
            int o2 = src2;
            vector res;
            res.i0 = o1.i0 - o2;
            res.i1 = o1.i1 - o2;
            res.i2 = o1.i2 - o2;
            res.i3 = o1.i3 - o2;
            res.i4 = o1.i4 - o2;
            res.i5 = o1.i5 - o2;
            res.i6 = o1.i6 - o2;
            res.i7 = o1.i7 - o2;
            vregisters[dest] = res;
            break;
        }

        case 0x23:
        {
            vector o1 = vregisters[src1] , o2 = vregisters[src2];
            vector res;
            res.i0 = o1.i0 * o2.i0;
            res.i1 = o1.i1 * o2.i1;
            res.i2 = o1.i2 * o2.i2;
            res.i3 = o1.i3 * o2.i3;
            res.i4 = o1.i4 * o2.i4;
            res.i5 = o1.i5 * o2.i5;
            res.i6 = o1.i6 * o2.i6;
            res.i7 = o1.i7 * o2.i7;
            vregisters[dest] = res;
            break;
        }

        case 0x33:
        {
            vector o1 = vregisters[src1];
            int o2 = registers[src2];
            vector res;
            res.i0 = o1.i0 * o2;
            res.i1 = o1.i1 * o2;
            res.i2 = o1.i2 * o2;
            res.i3 = o1.i3 * o2;
            res.i4 = o1.i4 * o2;
            res.i5 = o1.i5 * o2;
            res.i6 = o1.i6 * o2;
            res.i7 = o1.i7 * o2;
            vregisters[dest] = res;
            break;
        }

        case 0x2B:
        {
            vector o1 = vregisters[src1];
            int o2 = src2;
            vector res;
            res.i0 = o1.i0 * o2;
            res.i1 = o1.i1 * o2;
            res.i2 = o1.i2 * o2;
            res.i3 = o1.i3 * o2;
            res.i4 = o1.i4 * o2;
            res.i5 = o1.i5 * o2;
            res.i6 = o1.i6 * o2;
            res.i7 = o1.i7 * o2;
            vregisters[dest] = res;
            break;
        }

        case 0x25:
        {
            vector res;
            src2 = registers[src2];
            unsigned char c0 = data[src2++];
            unsigned char c1 = data[src2++];
            unsigned char c2 = data[src2++];
            unsigned char c3 = data[src2++];

            int a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i0 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i1 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i2 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i3 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i4 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i5 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i6 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i7 = a;

            vregisters[dest] = res;
            break;
        }

        case 0x2C:
        {
            vector res;
            unsigned char c0 = data[src2++];
            unsigned char c1 = data[src2++];
            unsigned char c2 = data[src2++];
            unsigned char c3 = data[src2++];

            int a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i0 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i1 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i2 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i3 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i4 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i5 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i6 = a;

            c0 = data[src2++];
            c1 = data[src2++];
            c2 = data[src2++];
            c3 = data[src2++];

            a = (unsigned int) c0 | ((unsigned int) c1 << 8) | ((unsigned int) c2 << 16) | ((unsigned int) c3 << 24);
            res.i7 = a;

            vregisters[dest] = res;
            break;
        }

        case 0x26:
        {
            vector vec = vregisters[src2];
            int o2 = vec.i0;
            dest = registers[dest];

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i1;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i2;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i3;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i4;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i5;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i6;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i7;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;
            break;
        }

        case 0x2E:
        {
            vector vec = vregisters[src2];
            int o2 = vec.i0;

            unsigned char c0 = o2;
            unsigned char c1 = (o2 >> 8);
            unsigned char c2 = (o2 >> 16);
            unsigned char c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i1;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i2;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i3;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i4;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i5;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i6;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;

            o2 = vec.i7;
            c0 = o2;
            c1 = (o2 >> 8);
            c2 = (o2 >> 16);
            c3 = (o2 >> 24);

            data[dest++] = c0;
            data[dest++] = c1;
            data[dest++] = c2;
            data[dest++] = c3;
            break;
        }
    }
}