/*
https://www.youtube.com/watch?v=lu6gGS9BJSY&t=44s
http://www.unix.com/programming/172517-put-2d-array-shared-memory.html
http://cboard.cprogramming.com/c-programming/165199-initialize-1d-2d-array-shared-memory-posix.html
http://stackoverflow.com/questions/27361763/initialize-a-1d-or-2d-array-in-shared-memory
http://www.cse.psu.edu/~deh25/cmpsc473/notes/OSC/Processes/shm.html
https://macboypro.wordpress.com/2009/05/25/producer-consumer-problem-using-cpthreadsbounded-buffer/
http://stackoverflow.com/questions/21227270/read-write-integer-array-into-shared-memory
http://profile.iiita.ac.in/bibhas.ghoshal/lab_files/shm.c
https://users.pja.edu.pl/~jms/qnx/help/watcom/clibref/qnx/shm_open.html
http://stackoverflow.com/questions/13274786/how-to-share-memory-between-process-fork
Name :Jayesh tambe
Description: DNS resolution using multi threading
// 2 exra credits covered
// matching number of cores with threads
// IPV6 code covered in util.c

*/
#include <stdio.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
/* exit() etc */
#include <unistd.h>
/* shm_* stuff, and mmap() */
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
/* for random() stuff */
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "multi-lookup.h"
// locking queue for critical region
//pthread_mutex_t QueLock;
// locking file for critical region
//pthread_mutex_t FileLock;

#define QUEUEFILL "/QueFill"
#define QUEUEEMPTY "/QueEmpty"
#define MUTEXFILE "/mutexFile_lock"
#define MUTEXQUEUE "/mutexQueue_lock"

#define HOSTARRAY "/hostArray"
#define COUNTER "/counter"
#define THREADSTATUS "/threadStatus"
#define RESULTFILE "/resultFile"

char *hostArray;// = (char *)malloc(r * c * sizeof(char));
int * counter;
int * ThreadStatus;
//FILE* ResultFile = NULL;

// counter and buffer in shared memory
// 2 condition variables in total for queue if it is full or empty
pthread_cond_t* conditionQueFill = NULL;
pthread_cond_t* conditionQueEmpty = NULL;
pthread_mutex_t* QueLock = NULL;
pthread_mutex_t* FileLock = NULL;
int des_condQueFill, des_condQueEmpty, des_QueLock, des_FileLock;
int des_hostArray, des_counter, des_ThreadStatus, des_ResultFile;
int mode = S_IRWXU | S_IRWXG;
int oflag = O_CREAT | O_RDWR | O_TRUNC;


int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		fprintf(stderr, "Input params are insufficient: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
	}

	des_hostArray = shm_open(HOSTARRAY, oflag, mode);
	//printf("des_hostArray \n");
	if (des_hostArray < 0)
	{
			perror("failure on shm_open on des_hostArray");
			exit(1);
	}
	if (ftruncate(des_hostArray, 10*SBUFSIZE*sizeof(char)) == -1)
	{
			perror("Error on ftruncate to sizeof int\n");
			exit(-1);
	}
	//int rows = 10;
	//int columns = SBUFSIZE;
	hostArray = (char *) mmap(NULL, 10*SBUFSIZE*sizeof(char),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_hostArray, 0);
	if (hostArray == MAP_FAILED )
	{
			perror("Error on mmap on hostArray\n");
			exit(1);
	}



	des_ThreadStatus = shm_open(THREADSTATUS, oflag, mode);
	//printf("des_ThreadStatus \n");
	if (des_ThreadStatus < 0)
	{
			perror("failure on shm_open on des_ThreadStatus");
			exit(1);
	}
	if (ftruncate(des_ThreadStatus, sizeof(int)) == -1)
	{
			perror("Error on ftruncate to sizeof int\n");
			exit(-1);
	}
	ThreadStatus = (int *) mmap(NULL, sizeof(int),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_ThreadStatus, 0);
	if (ThreadStatus == MAP_FAILED )
	{
			perror("Error on mmap on counter\n");
			exit(1);
	}

	*ThreadStatus = 1;

	// counter
	des_counter = shm_open(COUNTER, oflag, mode);
	//printf("des_counter \n");
	if (des_counter < 0)
	{
			perror("failure on shm_open on des_counter");
			exit(1);
	}
	if (ftruncate(des_counter, sizeof(int)) == -1)
	{
			perror("Error on ftruncate to sizeof int\n");
			exit(-1);
	}
	counter = (int *) mmap(NULL, sizeof(int),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_counter, 0);
	if (counter == MAP_FAILED )
	{
			perror("Error on mmap on counter\n");
			exit(1);
	}

	*counter = 0;
	// queue lock
	des_QueLock = shm_open(MUTEXQUEUE, oflag, mode);
	//printf("des_QueLock \n");
	if (des_QueLock < 0)
	{
			perror("failure on shm_open on des_QueLock");
			exit(1);
	}
	if (ftruncate(des_QueLock, sizeof(pthread_mutex_t)) == -1)
	{
			perror("Error on ftruncate to sizeof pthread_mutex_t\n");
			exit(-1);
	}
	QueLock = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_QueLock, 0);
	if (QueLock == MAP_FAILED )
	{
			perror("Error on mmap on QueLock\n");
			exit(1);
	}

	// file lock
	des_FileLock = shm_open(MUTEXFILE, oflag, mode);
	//printf("des_FileLock \n");
	if (des_FileLock < 0)
	{
			perror("failure on shm_open on des_FileLock");
			exit(1);
	}
	if (ftruncate(des_FileLock, sizeof(pthread_mutex_t)) == -1)
	{
			perror("Error on ftruncate to sizeof pthread_mutex_t\n");
			exit(-1);
	}
	FileLock = (pthread_mutex_t*) mmap(NULL, sizeof(pthread_mutex_t),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_FileLock, 0);
	if (FileLock == MAP_FAILED )
	{
			perror("Error on mmap on FileLock\n");
			exit(1);
	}

	//conditional variable
	des_condQueFill = shm_open(QUEUEFILL, oflag, mode);
	//printf("des_condQueFill \n");
	if (des_condQueFill < 0)
	{
			perror("failure on shm_open on des_condQueFill");
			exit(1);
	}
	if (ftruncate(des_condQueFill, sizeof(pthread_cond_t)) == -1)
	{
			perror("Error on ftruncate to sizeof pthread_cond_t\n");
			exit(-1);
	}
	conditionQueFill = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t),
	PROT_READ | PROT_WRITE, MAP_SHARED , des_condQueFill, 0);
	if (conditionQueFill == MAP_FAILED )
	{
			perror("Error on mmap on condition\n");
			exit(1);
	}

	//conditional variable
	des_condQueEmpty = shm_open(QUEUEEMPTY, oflag, mode);
	//printf("des_condQueEmpty \n");
	if (des_condQueEmpty < 0)
	{
			perror("failure on shm_open on des_condQueEmpty");
			exit(1);
	}
	if (ftruncate(des_condQueEmpty, sizeof(pthread_cond_t)) == -1)
	{
			perror("Error on ftruncate to sizeof pthread_cond_t\n");
			exit(-1);
	}
	conditionQueEmpty = (pthread_cond_t*) mmap(NULL, sizeof(pthread_cond_t),
	PROT_READ | PROT_WRITE, MAP_SHARED  , des_condQueEmpty, 0);
	if (conditionQueEmpty == MAP_FAILED )
	{
			perror("Error on mmap on condition\n");
			exit(1);
	}

	//printf("des_condQueEmpty end \n");
		/* set mutex shared between processes */
	pthread_mutexattr_t mutexQueueAttr;
	pthread_mutexattr_init(&mutexQueueAttr);
	if(pthread_mutexattr_setpshared(&mutexQueueAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
		fprintf(stdout,"Error: mutex attribute could not be initialized\n");
		return EXIT_FAILURE;
	}
	pthread_mutexattr_t mutexFileAttr;
	pthread_mutexattr_init(&mutexFileAttr);
	if(pthread_mutexattr_setpshared(&mutexFileAttr, PTHREAD_PROCESS_SHARED) != 0)
	{
		fprintf(stdout,"Error: mutex attribute could not be initialized\n");
		return EXIT_FAILURE;
	}

	//printf("mutex attributes done \n");
	/* set condition shared between processes */
	pthread_condattr_t condQueFillAttr;
	//pthread_cond_init(&condQueFillAttr);
	pthread_condattr_setpshared(&condQueFillAttr, PTHREAD_PROCESS_SHARED);

	pthread_condattr_t condQueEmptyAttr;
	//pthread_cond_init(&condQueEmptyAttr);
	pthread_condattr_setpshared(&condQueEmptyAttr, PTHREAD_PROCESS_SHARED);
	//pthread_cond_init(condition, &condAttr);



	//Initializing mutex objects
	if(((pthread_mutex_init(QueLock, &mutexQueueAttr))!=0)||((pthread_mutex_init(FileLock, &mutexFileAttr))!=0))
	{
		fprintf(stdout,"Error: mutex could not be initialized\n");
		return EXIT_FAILURE;
	}

	//Initializing conditional objects
	if((pthread_cond_init(conditionQueEmpty, &condQueEmptyAttr)) !=0)
	{
		fprintf(stdout,"Error: conditional variable could not be initialized\n");
		return EXIT_FAILURE;
	}

	if((pthread_cond_init(conditionQueFill, &condQueFillAttr)) !=0)
	{
		fprintf(stdout,"Error: conditional variable could not be initialized\n");
		return EXIT_FAILURE;
	}


	//printf("mutex & cond initialized\n");

	int resolveThreads[argc-1];
	int i;

  for(i =1 ;i < (argc); i++)
  {
		if((resolveThreads[i] = fork()) == 0)
		{
			ResolveHostNames(NULL);
			exit(0);
		}
  }


	for(i =1 ;i < (argc); i++)
	{
		RequestHostNames(argv[i]);
	}


	int status;

	*ThreadStatus = 0;

	for(i =0 ;i < (argc ); i++)
	{
		waitpid(resolveThreads[i], &status, WUNTRACED | WCONTINUED);
	}

  //destroy mutex objects
	pthread_condattr_destroy(&condQueEmptyAttr);
	pthread_condattr_destroy(&condQueFillAttr);
	pthread_mutexattr_destroy(&mutexQueueAttr);
	pthread_mutexattr_destroy(&mutexFileAttr);
	pthread_mutex_destroy(QueLock);
	pthread_mutex_destroy(FileLock);
	pthread_cond_destroy(conditionQueFill);
	pthread_cond_destroy(conditionQueEmpty);

	shm_unlink(QUEUEFILL);
	shm_unlink(QUEUEEMPTY);
	shm_unlink(MUTEXQUEUE);
	shm_unlink(MUTEXFILE);

	shm_unlink(HOSTARRAY);
	shm_unlink(COUNTER);
	shm_unlink(THREADSTATUS);
	//shm_unlink(RESULTFILE);

	// pthread_mutex_destroy(&QueLock);
	// pthread_mutex_destroy(&FileLock);

  //close file
	//fclose(ResultFile);
  // free memory from queue
	//queue_cleanup(&q);
	exit(0);
	return EXIT_SUCCESS;
}

void* RequestHostNames(void* voidptr)
{
	char errorstr[SBUFSIZE];
	char hostname[SBUFSIZE];

  // get length of name
  char file[strlen(voidptr)+1];

  //char *strcpy(char *dest, const char *src);
	strcpy(file,voidptr);

	FILE* inputfp = NULL;
  // open input file with read mode
	inputfp = fopen(file, "r");
  // error in opening file
  if (!inputfp)
	{
		sprintf(errorstr, "Error in opening file: %s", file);
		perror(errorstr);
		fprintf(stderr, "Usage:\n%s\n", USAGE2);
		return 0;
	}

  // get hostname from input file
	while(fscanf(inputfp, INPUTFS, hostname) > 0)
	{
		pthread_mutex_lock(QueLock); // p1
		while((*counter) == (QueSize-1)) // p2
		{
			pthread_cond_wait(conditionQueEmpty, QueLock); // p3
		}
		int tempcount = (*counter);
		// avoiding segfault
		if(tempcount < 0)
		{
			tempcount =0;
		}
		strncpy(&(hostArray[tempcount* SBUFSIZE]),hostname, SBUFSIZE);
		(*counter) = (*counter) +1;
		//fprintf(stderr,"*counter from RequestHostNames after increment %d \n",*counter);
		pthread_cond_signal(conditionQueFill); // p5
		pthread_mutex_unlock(QueLock); // p6
	}
  //close file
	fclose(inputfp);
	return 0;
}

void* ResolveHostNames(void* voidptr)
{

	pthread_mutex_lock(QueLock);
	while((*counter) == 0)
	{
		//conditionQueEmpty = queue count is not zero
		// following means wait till Queue count is not zero
		pthread_cond_wait(conditionQueFill, QueLock);
	}
	while ((*ThreadStatus) == 1 || (*counter != 0)) // term condition
	{
		while((*counter != 0))
		{
			char* hostnameResolve;
			hostnameResolve = (char*)malloc(SBUFSIZE * sizeof(char));
			int tempcount = (*counter);
			// avoiding segfault
			if((*counter) > 0)
			{
				tempcount = (*counter -1) ;
				if(tempcount < 0)
				{
					tempcount = 0;
				}
			}
			strncpy(hostnameResolve, (&hostArray[tempcount*SBUFSIZE]), SBUFSIZE);
			(*counter) = (*counter)- 1;
			//fprintf(stderr,"*counter from ResolveHostNames after decrement %d \n",*counter);
			if(hostnameResolve != NULL)
			{
				// get ip address
				char firstipstr[INET6_ADDRSTRLEN];
				if(dnslookup(hostnameResolve, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
				{
					fprintf(stderr, "Error in dnslookup: %s\n", hostnameResolve);
					strncpy(firstipstr, "", sizeof(firstipstr));
				}
				pthread_mutex_lock(FileLock);
				//fprintf(stdout, "%s,%s\n", hostnameResolve, firstipstr);
				printf( "%s,%s\n", hostnameResolve, firstipstr);
				pthread_mutex_unlock(FileLock);
				//free the string poped from queue
				free(hostnameResolve);
			}
			pthread_cond_signal(conditionQueEmpty); // c5
			pthread_mutex_unlock(QueLock); // c6
		}
		pthread_mutex_lock(QueLock); // c1

	}
	//pthread_mutex_unlock(QueLock);
	return voidptr;
}
