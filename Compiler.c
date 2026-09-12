#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "Compiler.h"

FILE *ifile = NULL;
FILE *ofile = NULL;

#define maxlablelength 100
#define maxlablesupport 64
#define maxinstlenght 100
#define byteinstlength 20

long long cchar = 0; // Variable that points to the current character in the file 
int line = 1; // Points to current line
long filesize; //Stores the number of B the file has
long instcount = 0; //Stores number of instruction comiler so far
int branchinst[maxlablesupport] , currinst = 0; // To handle the branch logic 

typedef struct 
{
    char name[maxlablelength];
    long offset;
} labler;

labler record[maxlablesupport];

char getcurrchar() // Get current character -> get the char at a particular poistion in the input file 
{
    if(cchar >= filesize)
    {
        return EOF;
    }
    fseek(ifile , cchar , SEEK_SET);
    return (char) fgetc(ifile);
}

char peek() //Peek -> let's you see the next character from our current poistion in the file 
{
    if(cchar + 1 >= filesize)
    {
        return EOF;
    }
    fseek(ifile , cchar + 1 , SEEK_SET);
    return (char) fgetc(ifile);
}

char advance() //Advacne -> it will move to poistion to the next valid character (skiping all white spaces)
{
    char temp = getcurrchar();
    do
    {
        if(temp =='\n' || temp == '\r' || temp == EOF)
        {
            line++;
            nextline();
        }
        if(temp == '%')
        {
            do
            {
                cchar++;
                temp = getcurrchar();
            }while(temp != '\n' && temp != '\r');
        }
        cchar++;
        temp = getcurrchar();
    } while(isspace((unsigned char)temp));
    return temp;
}

void nextline() // Function to detect that we have gone to the next line / instruction
{
    
}

void handlebranch()
{
    currinst = 0;
    fseek(ifile , 0 , SEEK_SET);
    fseek(ofile , 0 , SEEK_SET);
    cchar = 0;
    char line[maxinstlenght];
    while(fgets(line , sizeof(line) , ifile) != NULL)
    {
        line[getinstcropindex(line , sizeof(line))] = '\0';
        if(line[0] == 'B' || line[0] == 'b')
        {
            char t1 = toupper(advance());
            char t2 = toupper(advance());
            int opcode = 0, o2 = 0;

            switch (t1)
            {
                case 'E': // EQ (0000 = 0)
                    if (t2 == 'Q') opcode = 0;
                    else throwerror();
                        break;

                case 'N': // NE (0001 = 1)
                    if (t2 == 'E') opcode = 1;
                    else throwerror();
                    break;

                case 'C': // CS (0010 = 2), CC (0011 = 3)
                    if (t2 == 'S') opcode = 2;
                    else if (t2 == 'C') opcode = 3;
                    else throwerror();
                    break;

                case 'M': // MI (0100 = 4)
                    if (t2 == 'I') opcode = 4;
                    else throwerror();
                    break;

                case 'P': // PL (0101 = 5)
                    if (t2 == 'L') opcode = 5;
                    else throwerror();
                    break;

                case 'V': // VS (0110 = 6), VC (0111 = 7)
                    if (t2 == 'S') opcode = 6;
                    else if (t2 == 'C') opcode = 7;
                    else throwerror();
                    break;

                case 'H': // HI (1000 = 8)
                    if (t2 == 'I') opcode = 8;
                    else throwerror();
                    break;

                case 'L': // LS (1001 = 9), LT (1011 = 11), LE (1101 = 13)
                    if (t2 == 'S') opcode = 9;
                    else if (t2 == 'T') opcode = 11;
                    else if (t2 == 'E') opcode = 13;
                    else throwerror();
                    break;

                case 'G': // GE (1010 = 10), GT (1100 = 12)
                    if (t2 == 'E') opcode = 10;
                    else if (t2 == 'T') opcode = 12;
                    else throwerror();
                    break;

                case 'A': // AL (1110 = 14)
                    if (t2 == 'L') opcode = 14;
                    else throwerror();
                    break;

                default:
                    throwerror();
                    break;
            }
            t1 = advance();

            if(t1 != '.') throwerror();

            t1 = advance();
            fseek(ifile , cchar , SEEK_SET);
            if(fgets(line , maxlablelength , ifile) != NULL) //A error can be here
            {
                line[getlablecropindex(line , sizeof(line))] = '\0';
            }

            for(int i = 0; i < maxlablesupport; i++)
            {
                if(strcmp(record[i].name , line) == 0)
                {
                    o2 = record[i].offset - branchinst[currinst] - 1;
                }
            }
            int op = 0 , test , offset;
            while(fscanf(ofile , "%X %X %X %X\n" , &op , &test , &test , &test) == 4)
            {
                if(op == (0x10 + opcode))
                {
                    fseek(ofile , offset , SEEK_SET);
                    output(0x10 + opcode , 0 , 0 , o2);
                    currinst++;
                    break;
                }
                offset = ftell(ofile);
            }
        }
        cchar = ftell(ifile);
    }
}

void compile() // Start compilation of a single instruction
{
    char temp = getcurrchar();
    switch (temp)
    {
        case 'r' :
        case 'R' :
            advance();
            compileread();
            break;

        case 'w':
        case 'W':
            advance();
            compilewrite();
            break;

        case 'x':
        case 'X':
            advance();
            compilemath();
            break;

        case '[':
            advance();
            compilenewwrite();
            break;

        case '%':
            advance();
            break;

        case '.':
            advance();
            compilelable();
            break;

        case 'B':
        case 'b':
            {
                branchinst[currinst] = instcount;
                compilebranch();
                currinst++;
                break;
            }

        case 'V':
        case 'v' :
            advance();
            comiplevectormath();
            break;

        case 'P':
        case 'p':
            advance();
            compileprint();
            break;

        default:
            if(isspace(temp))
            {
                advance();
                compile();
                return;
            }
            printf("Parsing error in line : %d , character %c and peek %c\n" , line , getcurrchar() , peek());
            exit(EXIT_FAILURE);
    }
}

void compilelable()
{
    fseek(ifile , cchar , SEEK_SET);
    char str[maxlablelength];
    if (fgets(str, sizeof(str), ifile) != NULL) 
    {
        str[getlablecropindex(str , sizeof(str))] = '\0';
    }
    int i;
    for(i = 0; i < maxlablesupport ; i++)
    {
        if(strcmp(record[i].name , str) == 0) // Strings are equal
        {
            fprintf(stderr, "Multiple lables can't have a same name in line : %d\n", line);
            exit(EXIT_FAILURE);
        }
        else if(record[i].name[0] == '\0') break;
    }
    strcpy(record[i].name , str);
    record[i].offset = instcount;

    cchar = ftell(ifile);
}

void compilebranch()
{
    char t1 = toupper(advance());
    char t2 = toupper(advance());
    int opcode = 0, o2 = 0;

    switch (t1)
    {
        case 'E': // EQ (0000 = 0)
            if (t2 == 'Q') opcode = 0;
            else throwerror();
            break;

        case 'N': // NE (0001 = 1)
            if (t2 == 'E') opcode = 1;
            else throwerror();
            break;

        case 'C': // CS (0010 = 2), CC (0011 = 3)
            if (t2 == 'S') opcode = 2;
            else if (t2 == 'C') opcode = 3;
            else throwerror();
            break;

        case 'M': // MI (0100 = 4)
            if (t2 == 'I') opcode = 4;
            else throwerror();
            break;

        case 'P': // PL (0101 = 5)
            if (t2 == 'L') opcode = 5;
            else throwerror();
            break;

        case 'V': // VS (0110 = 6), VC (0111 = 7)
            if (t2 == 'S') opcode = 6;
            else if (t2 == 'C') opcode = 7;
            else throwerror();
            break;

        case 'H': // HI (1000 = 8)
            if (t2 == 'I') opcode = 8;
            else throwerror();
            break;

        case 'L': // LS (1001 = 9), LT (1011 = 11), LE (1101 = 13)
            if (t2 == 'S') opcode = 9;
            else if (t2 == 'T') opcode = 11;
            else if (t2 == 'E') opcode = 13;
            else throwerror();
            break;

        case 'G': // GE (1010 = 10), GT (1100 = 12)
            if (t2 == 'E') opcode = 10;
            else if (t2 == 'T') opcode = 12;
            else throwerror();
            break;

        case 'A': // AL (1110 = 14)
            if (t2 == 'L') opcode = 14;
            else throwerror();
            break;

        default:
            throwerror();
            break;
    }
    
    t1 = advance();

    if(t1 == '.') t1 = advance();
    else throwerror();

    fseek(ifile , cchar , SEEK_SET);
    char str[maxlablelength];
    if (fgets(str, sizeof(str), ifile) != NULL) 
    {
        str[getlablecropindex(str , sizeof(str))] = '\0';
    }

    for(int i = 0; i < maxlablesupport; i++)
    {
        if(strcmp(record[i].name , str) == 0)
        {
            o2 = record[i].offset - instcount - 1;
        }
    }
    output(0x10 + opcode , 0 , 0 , o2);
    cchar = ftell(ifile);
}

int getlablecropindex(char * t , int size)
{
    int i = 0;
    for(; i < size && t[i] != '\0'; i++)
    {
        if(isalnum(t[i]))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    return i;
}

int getinstcropindex(char* t , int size)
{
    int i = 0;
    for(; i < size && t[i] != '\0'; i++)
    {
        if(t[i] == '\r' || t[i] == '\n' || t[i] == '%')
        {
            break;
        }
    }
    return i;
}

void comiplevectormath()
{
    int dest , o1 , o2;
    char temp = getcurrchar();

    if(isdigit(temp))
    {
        dest = readnum();
    }
    else throwerror();

    temp = getcurrchar();
    if(temp == '=') temp = advance();
    else throwerror();

    if(temp == 'V' || temp == 'v') 
    {
        temp = advance();
    }
    else if(temp == '[') //Mem read instruction
    {
        temp = advance();

        if(temp == 'X' || temp == 'x')
        {
            temp = advance();
            if(isdigit(temp))
            {
                o2 = readnum();
                output(0x25 , dest , 0 ,o2);
            }
        }
        else if(isdigit(temp))
        {
            o2 = readnum();
            output(0x2C , dest , 0 , o2);
        }
        else throwerror();
        temp = getcurrchar();
        if(temp != ']') throwerror();
        advance();
        return;
    }
    else throwerror();

    if(isdigit(temp))
    {
        o1 = readnum();
    }

    char op = getcurrchar();
    temp = advance();

    if(temp == 'V' || temp == 'v') //Final output will be here
    {
        temp = advance();
        if(isdigit(temp))
        {
            o2 = readnum();
        }

        switch(op)
        {
            case '+':
            {
                output(0x21 , dest , o1 , o2);
                break;
            }

            case '-':
            {
                output(0x22 , dest , o1 , o2);
                break;
            }

            case '*':
            {
                output(0x23 , dest , o1 , o2);
                break;
            }

            default :
            {
                fprintf(stderr, "Invalid operation in line : %d\n", line);
                exit(EXIT_FAILURE);
            }
        }
        return;
    }
    else if(temp == 'X' || temp == 'x')
    {
        temp = advance();
        if(isdigit(temp))
        {
            o2 = readnum();
        }

        switch(op)
        {
            case '+':
            {
                output(0x31 , dest , o1 , o2);
                break;
            }

            case '-':
            {
                output(0x32 , dest , o1 , o2);
                break;
            }

            case '*':
            {
                output(0x33 , dest , o1 , o2);
                break;
            }

            default :
            {
                fprintf(stderr, "Invalid operation in line : %d\n", line);
                exit(EXIT_FAILURE);
            }
        }
        return;
    }
    else if(isdigit(temp))
    {
        o2 = readnum();

        switch(op)
        {
            case '+':
            {
                output(0x29 , dest , o1 , o2);
                break;
            }

            case '-':
            {
                output(0x2A , dest , o1 , o2);
                break;
            }

            case '*':
            {
                output(0x2B , dest , o1 , o2);
                break;
            }

            default :
            {
                fprintf(stderr, "Invalid operation in line : %d\n", line);
                exit(EXIT_FAILURE);
            }
        }
        return;
    }
    else throwerror();
}

void compileread() //Handle read instruction
{
    char temp = getcurrchar();
    if(temp == 'e') temp = advance();
    else throwerror();

    if(temp == 'a') temp = advance();
    else throwerror();

    if(temp == 'd') temp = advance();
    else throwerror();

    int dest; //Variable to store destination varibale(of program)
    if(temp == 'x' || temp == 'X')
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        dest = readnum();
    }
    else throwerror();

    temp = getcurrchar();
    if(temp == ',') temp = advance();
    else throwerror();

    int value; //Value that will be put into the variable
    if(isdigit((unsigned char) temp))
    {
        value = readnum();
    }
    else throwerror();

    output(5 , dest , 0 , value);
}

void compilewrite() //Handle write instruction
{
    char temp = getcurrchar();
    if(temp == 'r') temp = advance();
    else throwerror();

    if(temp == 'i') temp = advance();
    else throwerror();

    if(temp == 't') temp = advance();
    else throwerror();

    if(temp == 'e') temp = advance();
    else throwerror(); 

    int dest , value; //Variable to store destination varibale(of program)
    if(temp == 'x' || temp == 'X')
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        value = readnum();
    }
    else throwerror();

    temp = getcurrchar();
    if(temp == ',') temp = advance();
    else throwerror();

    if(isdigit((unsigned char) temp))
    {
        dest = readnum();
    }
    else throwerror();

    output(6 , dest , 0 , value);
}

void compilenewwrite()
{
    int dest , o2 ;
    bool isconstant;
    char temp = getcurrchar();
    if(temp == 'X' || temp == 'x')
    {
        temp = advance();
        if(isdigit(temp)) dest = readnum();
        else throwerror();
        isconstant = false;
    } 
    else if(isdigit(temp))
    {
        dest = readnum();
        isconstant = true;
    }
    else throwerror();

    temp = getcurrchar();
    if(temp == ']') temp = advance();
    else throwerror();

    if(temp == '=') temp = advance();
    else throwerror();

    if(temp == 'X' || temp == 'x') //address provided by the register
    {
        temp = advance();
        if(isdigit(temp))
        {
            o2 = readnum();
            output((isconstant ? 0x0E : 6) , dest , 0 , o2);
            return;
        }
        else throwerror();
    }
    else if(temp == 'V' || temp == 'v')
    {
        temp = advance();
        if(isdigit(temp))
        {
            o2 = readnum();
            output((isconstant ?  0x2E : 0x26), dest , 0 ,o2); // A doubt
            return;
        }
        else throwerror();
    }
    else throwerror();
}

void compilemath() //Handle math opertaions
{
    int dest; //Variable to store destination varibale(of program)
    int o1; //operand 1
    int o2; //operand 2
    int a = line; 

    if(isdigit((unsigned char) getcurrchar()))
    {
        dest = readnum();
    }
    else throwerror();

    char temp = getcurrchar();
    if(temp == '=') temp = advance();
    else throwerror();

    if(temp == 'x' || temp == 'X')//for variable
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        o1 = readnum();

        if(a < line) // still remaining
        {
            output(7 , dest , 0 , o1);
            return;
        }
    }
    else if(isdigit(temp)) //Data Movement
    {
        o2 = readnum();
        output(15 , dest , 0 , o2);
        return;
    }
    else if(temp == '[') //Read from memory 
    {
        temp = advance();
        if(temp == 'X' || temp == 'x') //address is in register
        {
            temp = advance();
            if(isdigit(temp))
            {
                o2 = readnum();
                output(5 , dest , 0 , o2);
            }
        }
        else if(isdigit(temp)) //Address is a constant value
        {
            o2 = readnum();
            output(13 , dest , 0 , o2);
        }
        else throwerror();
        advance();
        return;
    }
    else throwerror();

    char op = getcurrchar(); //We are on the operation by now 
    temp = advance();

    if(temp == 'x' || temp == 'X')
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        o2 = readnum();
    }
    else if(isdigit(temp)) //Arithmetic with a constant operand
    {
        o2 = readnum();
        switch(op)
        {
            case '+':
                output(9 , dest , o1 , o2);
                return;
            case '-':
                output(10 , dest , o1 , o2);
                return;
            case '*':
                output(11 , dest , o1 , o2);
                return;
            case '/':
                output(12 , dest , o1 , o2);
                return;
        }
    }
    else throwerror();

    switch(op) //Arithmatic when both operands are registers
    {
        case '+':
            output(1 , dest , o1 , o2);
            break;
        case '-':
            output(2 , dest , o1 , o2);
            break;
        case '*':
            output(3 , dest , o1 , o2);
            break;
        case '/':
            output(4 , dest , o1 , o2);
            break;
    }
}

void compileprint() //Handle print instruction
{
    char temp = getcurrchar();
    if(temp == 'r' || temp == 'R') temp = advance();
    else throwerror();

    if(temp == 'i' || temp == 'I') temp = advance();
    else throwerror();

    if(temp == 'n' || temp == 'N') temp = advance();
    else throwerror();

    if(temp == 't' || temp == 'T') temp = advance();
    else throwerror();

    int o2 = 0;
    if(temp == 'x' || temp == 'X')
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        o2 = readnum();
    }
    else throwerror();

    output(8 , 0 , 0 , o2);
}

void throwerror() //Show error in the byte file
{
    fprintf(stderr, "Compilation error in line %d , character : %c , peek : %c\n", line , getcurrchar() , peek());
    exit(EXIT_FAILURE);
}

int readnum() //Return int from the variable name
{
    int res = 0;
    char temp = getcurrchar();

    while(isdigit((unsigned char) temp))
    {
        int n = temp - '0';
        res = res * 10 + n;
        temp = advance();
    }
    if(res <= 255) return res;
    else 
    {
        throwerror();
        return 0;
    }
}

void output(int op , int dest , int o1 , int o2) //Function to output the byte code to program.byte
{
    instcount++;
    char out[byteinstlength];
    snprintf(out, sizeof(out) , "%X %X %X %X\n", (unsigned char)op , (unsigned char)dest , (unsigned char)o1 , (unsigned char)o2);
    fputs(out , ofile);
}

int startcompiler(char *input_path, char *output_path) //Starts the actual compilation process 
{
    ifile = fopen(input_path , "r");
    if(ifile == NULL)
    {
        fprintf(stderr, "Failed to open input file: %s\n", input_path);
        return -1;
    }
    ofile = fopen(output_path , "w+");
    if(ofile == NULL)
    {
        fprintf(stderr, "Failed to open output file: %s\n", output_path);
        fclose(ifile);
        ifile = NULL;
        return -1;
    }

    cchar = 0;
    line = 1;
    instcount = 0;
    currinst = 0;
    for(int i = 0; i < maxlablesupport; i++)
    {
        record[i].name[0] = '\0';
        record[i].offset = 0;
        branchinst[i] = 0;
    }

    fseek(ifile , 0 , SEEK_END);
    filesize = ftell(ifile);
    fseek(ifile , 0 , SEEK_SET);

    while(cchar < filesize)
    {
        compile();
    }
    output(0 , 0 , 0 , 0);
    handlebranch();

    fclose(ifile);
    fclose(ofile);
    ifile = NULL;
    ofile = NULL;
    return 0;
}