#ifndef COMPILER_H
#define COMPILER_H

extern FILE *ifile;
extern FILE *ofile;

char getcurrchar();
char peek();
char advance();
void nextline();
void handlebranch();
void compile();
void compilelable();
void compilebranch();
int getlablecropindex(char * , int );
int getinstcropindex(char * , int );
void comiplevectormath();
void compileread();
void compilewrite();
void compilenewwrite();
void compilemath();
void throwerror();
int readnum();
void output(int , int , int , int);
void startcompiler();

#endif