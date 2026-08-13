#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>

int get_bit(uint8_t , int );
void reset();
void fetch();
void decode();
void execute();

#endif