#include <stdio.h>
#include <stdlib.h>
#include "Compiler.h"
#include "processor.h"
#include "memory.h"

FILE *ifile;
FILE *ofile;
FILE *dfile;
extern int end_of_simulation;

int main()
{
    printf(" 0 for custom code\n 1 for sum of n numbers\n 2 for FIR Filter\n");
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
            ifile = fopen("Test_Cases/Sum_of_N_dynamic(source).txt" , "r");
            dfile = fopen("Test_Cases/Sum_of_N_dynamic(data).byte" , "r+");
            break;
        }

        case 2 :
        {
            ifile = fopen("Test_Cases/FIR_Filter(source).txt" , "r");
            dfile = fopen("Test_Cases/FIR_Filter(data).byte" , "r+");
            break;
        }

        default :
        {
            fprintf(stderr, "Invalide Operation , exiting program....");
            exit(EXIT_FAILURE);
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