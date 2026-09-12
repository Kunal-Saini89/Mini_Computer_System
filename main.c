#include <stdio.h>
#include <stdlib.h>
#include "os.h"

int main(int argc , char *argv[])
{
    init_os();

    if(argc > 1)
    {
        for(int i = 1 ; i < argc ; i += 2)
        {
            if(i + 1 < argc)
            {
                loader(argv[i] , argv[i + 1]);
            }
            else
            {
                loader(argv[i] , "data.byte");
            }
        }
    }

    while(is_os_running())
    {
        scheduler();
    }

    close_processor();

    return 0;
}