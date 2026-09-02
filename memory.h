#ifndef MEMORY_H
#define MEMORY_H

#include "processor.h"

extern char instruction[NP][256] , data[NP][4096];

// Function prototypes
void initialize(int , char * , char *);
void finalize(int , char *);

#endif