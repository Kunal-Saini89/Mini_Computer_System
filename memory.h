#ifndef MEMORY_H
#define MEMORY_H

#include "processor.h"

#define MEMSIZE 8192
#define PAGESIZE 512
#define NUM_PHYSICAL_PAGES (MEMSIZE / PAGESIZE)
#define NUM_LOGICAL_PAGES (1024 / PAGESIZE + 4096 / PAGESIZE)

extern char memory[MEMSIZE];

// Function prototypes
void initialize(int , char * , char *);
void finalize(int , char *);

#endif