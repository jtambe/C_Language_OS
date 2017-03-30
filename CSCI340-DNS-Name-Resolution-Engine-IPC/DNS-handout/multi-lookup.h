/*
Jayesh Tambe
Multi threaded program to resolve DNS
*/
#include <stdlib.h>
#include <stdio.h>
//#include "queue.h"
#include "util.h"
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define USAGE2 "<VALID inputFile/Path Required> < VALID outputFile/Path Required>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define QueSize 10

//char array [10][SBUFSIZE]

typedef char * oneHostName;


//int initializeObjects(int argc,char* argv[]);
void* ResolveHostNames(void* voidptr);
void* RequestHostNames(void* voidptr);
