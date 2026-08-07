#include <stdio.h>
#include "Compiler.h"
#include "processor.h"
#include "memory.h"

FILE *ifile;
FILE *ofile;
FILE *dfile;
extern int end_of_simulation;

int main()
{
    printf(" 0 for custom code , 1 for sum of n numbers , 2 for multiply 2 complex numbers , 3 for determinat of matrix\n");
    int i = 0;
    scanf("%d" , &i);
    switch(i)
    {
        case 0 :
        {
            ifile = fopen("Input.txt" , "r");
            dfile = fopen("data.byte" , "r+");
            break;
        }

        case 1 :
        {
            ifile = fopen("Test_Cases/Sum n number(source).txt" , "r");
            dfile = fopen("Test_Cases/Sum n number(data).byte" , "r+");
            break;
        }

        case 2 :
        {
            ifile = fopen("Test_Cases/Multiply 2 Complex (sorce).txt" , "r");
            dfile = fopen("Test_Cases/Multiply 2 Complex (data).byte" , "r+");
            break;
        }

        case 3 :
        {
            ifile = fopen("Test_Cases/determinat of matrix(source).txt" , "r");
            dfile = fopen("Test_Cases/determinat of matrix(data).byte" , "r+");
            break;
        }
    }
    ofile = fopen("program.byte" , "w+"); //Program.byte file is common for all type of codes 

    startcompiler();
    initialize();
    reset();

    while(end_of_simulation == 0)
    {
        fetch();
        decode();
        execute();
    }

    finalize();

    fclose(ifile);
    fclose(ofile);
    fclose(dfile);

    return 0;
}