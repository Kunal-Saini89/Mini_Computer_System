#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "Compiler.h"

long long cchar = 0; // Variable that points to the current character in the file 
int line = 0; // Points to current line
long filesize; //Stores the number of B the file has 

char getcurrchar() // Get current character -> get the char at a particular poistion in the input file 
{
    if(cchar >= filesize)
    {
        return EOF;
    }
    fseek(ifile , cchar , SEEK_SET);
    return (char) fgetc(ifile);
}

char peek() // peek -> let's you see the next character from our current poistion in the file 
{
    if(cchar + 1 >= filesize)
    {
        return EOF;
    }
    fseek(ifile , cchar + 1 , SEEK_SET);
    return (char) fgetc(ifile);
}

char advance() // advacne -> it will move to poistion to the next valid character (skiping all white spaces)
{
    char temp = getcurrchar();
    do
    {
        if( temp == '\n')
        {
            line++;
            nextline();
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
    }
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

    output(5 , dest , value , 0);
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

    output(6 , dest , value , 0);
}

void compilemath() //Handle math opertaions
{
    int dest; //Variable to store destination varibale(of program)
    int o1; //operand 1
    int o2; //operand 2

    if(isdigit((unsigned char) getcurrchar()))
    {
        dest = readnum();
    }
    else throwerror();

    char temp = getcurrchar();
    if(temp == '=') temp = advance();
    else throwerror();

    if(temp == 'x' || temp == 'X')
    {
        if(!isdigit((unsigned char) peek()))
        {
            throwerror();
        }
        advance();
        o1 = readnum();
    }
    else throwerror();

    char op = getcurrchar(); // we are on the operation by now 
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
    else throwerror();

    switch(op)
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
    char out[20];
    snprintf(out, sizeof(out) , "%d %d %d %d\n",op,dest,o1,o2);
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