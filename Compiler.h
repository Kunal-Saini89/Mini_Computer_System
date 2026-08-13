#ifndef COMPILER_H
#define COMPILER_H

extern FILE *ifile;
extern FILE *ofile;

char getcurrchar();
char peek();
char advance();
void nextline();
void compile();
void compilelable();
void compilebranch();
void compileread();
void compilewrite();
void compilenewwrite();
void compilemath();
void throwerror();
int readnum();
void output(int , int , int , int);
void startcompiler();

#endif