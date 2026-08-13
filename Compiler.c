#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Compiler.h"

#define maxlablelength 50
#define maxlablesupport 64

long long cchar = 0; // Variable that points to the current character in the file 
int line = 1; // Points to current line
long filesize; //Stores the number of B the file has
long instcount = 0; //Stores number of instruction comiler so far

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
            //advance();
            compilebranch();
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
        str[strcspn(str, "\r\n")] = '\0';
    }
    int i;
    for(i = 0; i < maxlablesupport ; i++)
    {
        if(strcmp(record[i].name , str) == 0) // Strings are equal
        {
            fprintf(stderr, "Multiple lables can't have a same name", line);
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
        str[strcspn(str, "\r\n")] = '\0';
    }

    for(int i = 0; i < maxlablesupport; i++)
    {
        if(strcmp(record[i].name , str) == 0)
        {
            o2 = record[i].offset - instcount;
        }
    }
    output(0x10 + opcode , 0 , 0 , o2);
    cchar = ftell(ifile);
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
    else throwerror;

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
    else throwerror;

    output(6 , dest , 0 , value);
}

void compilenewwrite()
{
    int dest , o2;
    char temp = getcurrchar();
    if(temp == 'X' || temp == 'x')
    {
        temp = advance();
        if(isdigit(temp)) dest = readnum();
        else throwerror();
    } 
    else if(isdigit(temp))
    {
        dest = readnum();
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
        o2 = readnum();
        output(6 , dest , 0 , o2);
        return;
    }
    else if(isdigit(temp)) //address provided by a constant
    {
        o2 = readnum();
        output(14 , dest , 0 , o2);
        return;
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

void throwerror() //Show error in the byte file
{
    fprintf(stderr, "Compilation error in line %d\n", line);
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
    else throwerror();
}

void output(int op , int dest , int o1 , int o2) //Function to output the byte code to program.byte
{
    instcount++;
    char out[20];
    snprintf(out, sizeof(out) , "%X %X %X %X\n", (unsigned char)op , (unsigned char)dest , (unsigned char)o1 , (unsigned char)o2);
    fputs(out , ofile);
}

void startcompiler() //Starts the actual compilation process 
{
    fseek(ifile , 0 , SEEK_END);
    filesize = ftell(ifile);

    while(cchar < filesize)
    {
        compile();
    }
}