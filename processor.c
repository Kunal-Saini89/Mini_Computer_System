#include <stdio.h>
#include "processor.h"

int registers[256];
int pc;
int opcode , dest , src1 , src2;
uint8_t flags; // 0 0 0 0 Z N C V
extern char instruction[256] , data[4096];
int end_of_simulation = 0;

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
            if((o1 ^ o2 < 0) && (res ^ o2 >= 0)) flags = flags | 0b00000001;
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
            registers[dest] = o1 / o2;
            break;
        }

        case 0x05 :
        {
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
            if((o1 ^ o2 < 0) && (res ^ o2 >= 0)) flags = flags | 0b00000001;
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
            registers[dest] = o1 / o2;
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
            int o2 = src2;

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
    }
}