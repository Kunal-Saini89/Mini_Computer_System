#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>
#include <stdio.h>

#define NP 2

extern int registers[NP][256];
extern int pc[NP];
extern uint8_t flags[NP];
extern int end_of_simulation[NP];
extern FILE *fd_log;

int get_bit(uint8_t , int );
void reset(int );
void fetch(int );
void decode(int );
void execute(int );
void process_instructions(int , int );
void init_processor();
void close_processor();

#endif