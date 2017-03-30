/*
Name :Jayesh tambe
Description: DNS resolution using multi threading
// 2 exra credits covered
// matching number of cores with threads
// IPV6 code covered in util.c

*/
#include "multi-lookup.h"
// locking queue for critical region
pthread_mutex_t QueLock;
// locking file for critical region
pthread_mutex_t FileLock;
int ThreadStatus = 1;
queue q;
FILE* ResultFile = NULL;

int main(int argc, char* argv[])
{
  // get requesting threads using argc count
	int NumReqThreads=argc-2;

  // get number of cores using sysconfiguration
  // extra: matching number of cores and threads
	int NoOfCores = sysconf(_SC_NPROCESSORS_ONLN);
	if(NoOfCores <= 1)
	{
    // forcing minimum no of threads to be 2
    NoOfCores = 2;
  }

	pthread_t RequestThreads[NumReqThreads];
  //extra
	pthread_t ResolverThreads[NoOfCores];

  //intilize all mutex and file etc.
	int InitSuccess = initializeObjects(argc,argv);

  // Initializing failed so returning with failure
	if(InitSuccess == EXIT_FAILURE)
	{ return EXIT_FAILURE; }

  // create requester threads
	int i=1;
  for(i =1 ;i < (argc -1); i++)
  {
    // creating thread for each input file
    // associate requesting function to each thread
    // argv[i] = input file names
    if(pthread_create(&(RequestThreads[i-1]), NULL, RequestHostNames, argv[i]))
		{
			printf("error occurred in requesting\n");//
			return EXIT_FAILURE;
		}
  }

  // creating resolver threads
	int j=0;
  for(j = 0; j < NoOfCores; j++)
  {
    // create thread with resolution fucntion for each thread
    if(pthread_create(&(ResolverThreads[j]), NULL, ResolveHostNames, NULL))
		{
			printf("error  occurred in resolution\n");
			return EXIT_FAILURE;
		}
  }

  // wait and join all threads
	int k=0;
  for(k = 0; k < NumReqThreads; k++)
  {
    pthread_join(RequestThreads[k], NULL);
  }

	ThreadStatus = 0;
	int l=0;
  for(l = 0; l < NoOfCores; l++)
  {
    pthread_join(ResolverThreads[l], NULL);
  }

  //destroy mutex objects
	pthread_mutex_destroy(&QueLock);
	pthread_mutex_destroy(&FileLock);

  //close file
	fclose(ResultFile);
  // free memory from queue
	queue_cleanup(&q);

	return EXIT_SUCCESS;
}

int initializeObjects(int argc, char* argv[])
{

  // must need minimum 3 arguments
  // 1. program Name
  // 2. input file
  // 3. output file
	if(argc < 3)
	{
		fprintf(stderr, "Input params are insufficient: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
	}

  //error in opeining the result file
	ResultFile = fopen(argv[(argc-1)], "w");
	if(!ResultFile)
	{
		perror("Error: Cannot open result file");
		return EXIT_FAILURE;
	}

	//Initializing mutex objects
	if(((pthread_mutex_init(&QueLock, NULL))!=0)||((pthread_mutex_init(&FileLock, NULL))!=0))
	{
		fprintf(stdout,"Error: mutex could not be initialized\n");
		return EXIT_FAILURE;
	}


  // returns -1 when Initializing queue fails
	if(queue_init(&q, QueSize) == QUEUE_FAILURE)
	{
		perror("Error: Queue could not be initialized");
		return EXIT_FAILURE;
	}

  // successful initialization
	return EXIT_SUCCESS;

}

void* RequestHostNames(void* voidptr)
{
	char errorstr[SBUFSIZE];
	char hostname[SBUFSIZE];
	int flag = 1;
	char* HostString;

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
		flag = 1;
		while(flag)
		{
      // lock the queue
			pthread_mutex_lock(&QueLock);
			if(queue_is_full(&q))
			{
				pthread_mutex_unlock(&QueLock);
        //make a random sleep
        //sleep - takes seconds input
        //usleep - takes microseconds input
        usleep(rand()%100);
				continue;
			}
			else
			{
        // get host name in another variable
				HostString = malloc(strlen(hostname)+1);
				strcpy(HostString, hostname);
        // place host name in requesting queue
				queue_push(&q, HostString);
        // release lock
				pthread_mutex_unlock(&QueLock);
        //break the while loop
				flag = 0;
			}
		}
	}
  //close file
	fclose(inputfp);
	return 0;
}

void* ResolveHostNames(void* voidptr)
{
	char firstipstr[INET6_ADDRSTRLEN];
  //queue returns char
	char* hostnameResolve;

  // lock queue
	pthread_mutex_lock(&QueLock);

  //keep in loop if ThreadStatus is one or queue is not empty
	while(ThreadStatus == 1 || !queue_is_empty(&q))
	{
    //
		if(!queue_is_empty(&q))
		{
			hostnameResolve = queue_pop(&q);
			pthread_mutex_unlock(&QueLock);

			if(hostnameResolve != NULL)
			{
        // get ip address
				if(dnslookup(hostnameResolve, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE)
				{
					fprintf(stderr, "Error in dnslookup: %s\n", hostnameResolve);
					strncpy(firstipstr, "", sizeof(firstipstr));
				}
				pthread_mutex_lock(&FileLock);
				fprintf(ResultFile, "%s,%s\n", hostnameResolve, firstipstr);
				pthread_mutex_unlock(&FileLock);

        //free the string poped from queue
				free(hostnameResolve);
        //lock mutex for next while loop
				pthread_mutex_lock(&QueLock);
        //continue the while loop
				continue;
			}
			else
			{
				fprintf(stdout,"Queue failed\n");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
		pthread_mutex_unlock(&QueLock);
    //lock mutex for next while loop;
		pthread_mutex_lock(&QueLock);
	 }
	}
  //release lock
	pthread_mutex_unlock(&QueLock);
	return voidptr;
}
