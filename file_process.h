/********************************************************
 * Creatint for practice, free learning purpose.
 * 
 * filename: file_process.h
 * author: Martin
 * date: Wed Jan 30 2019
 * description: 
 * 
 ********************************************************/

// #ifndef FILE_PROCESS
// #define FILE_PROCESS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus 
extern "C" { 
#endif

extern void *read_wav (char *name, int *len);
extern void *read_pcm (char *name, int *len);

#ifdef __cplusplus 
} 
#endif
// #endif
