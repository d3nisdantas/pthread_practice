/*
 ============================================================================
 Name        : ReaderWriter_ex.c
 Author      : denis
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUF_SIZE 3 /* size of shared buffer */
#define EXEC_NUM 5 /* number of times the threads will execute */
#define READ_NUM 5
#define WRITE_NUM 5

//int buffer[BUF_SIZE];
int sharedVar;
int add = 0;	/* place to add the next element */
int rem = 0;	/* place to remove the next element */
int num = 0;	/* number of elements in a buffer */
int gReaders = 0;
int gWaitingReaders = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;		/* mutex for buffer */
pthread_cond_t readPhase = PTHREAD_COND_INITIALIZER;	/* consumer waits on cv */
pthread_cond_t writePhase = PTHREAD_COND_INITIALIZER;

static void *producer(void *params);
static void *consumer(void *params);

int main(void) {
	int readIds[READ_NUM];
	int writeIds[WRITE_NUM];
	pthread_t readerThreads[READ_NUM];
	pthread_t writerThreads[WRITE_NUM];

	for(int i = 0; i < WRITE_NUM; i++)
	{
		//create writers
		writeIds[i] = i;

		if (pthread_create(&writerThreads[i], NULL, producer, &writeIds[i]) !=0)
		{
			fprintf(stderr, "Unable to create producer thread.\n");
			exit(1);
		}
	}

	for(int i = 0; i < READ_NUM; i++)
	{
		//create readers
		readIds[i] = i;

		if (pthread_create(&readerThreads[i], NULL, consumer, &readIds[i]) !=0)
		{
			fprintf(stderr, "Unable to create consumer thread.\n");
			exit(1);
		}
	}

	for(int i = 0; i < READ_NUM; i++)
	{
		pthread_join(readerThreads[i], NULL);
	}
	for(int i = 0; i < WRITE_NUM; i++)
	{
		pthread_join(writerThreads[i], NULL);
	}

	printf("parent quitting.\n");
	return 0;
}

static void *producer(void *params){
	int id = *((int*)params);
	int numReaders = 0;

	for(int i = 0; i < EXEC_NUM; i++)
	{
		// Wait so that all threads don't take the lock at the same time
		usleep(1000 * (random() % 5 + 5));
		// Enter critical session
		pthread_mutex_lock(&m);
			while(gReaders)
			{
				// a writer can only write when there are no other writers or readers executing
				pthread_cond_wait(&writePhase, &m);
			}
			gReaders = -1;
			numReaders = gReaders;
		pthread_mutex_unlock(&m);

		printf("Producer %lu writing %d. [Readers: %d]\n", id, ++sharedVar, numReaders); fflush(stdout);

		// Exit critical session
		pthread_mutex_lock(&m);
			gReaders = 0;
			if(gWaitingReaders > 0)
				pthread_cond_broadcast(&readPhase);
			else
				pthread_cond_signal(&writePhase);

		pthread_mutex_unlock(&m);
	}

	printf("producer %d, quitting\n", id); fflush(stdout);
	return 0;
}

static void *consumer(void *params)
{
	int id = *((int*)params);
	int numReaders = 0;

	for(int j = 0; j < EXEC_NUM; j++)
	{
		// Wait so that all threads don't take the lock at the same time
		usleep(1000 * (random() % 5 + 5));
		// Enter critical session
		pthread_mutex_lock(&m);
			gWaitingReaders++;
			while(gReaders == -1)
			{
				pthread_cond_wait(&readPhase, &m);
			}
			gWaitingReaders--;
			numReaders = ++gReaders;
		pthread_mutex_unlock(&m);

		printf("Consumer %lu read %d. [Readers: %d]\n", id, sharedVar, numReaders); fflush(stdout);

		//Exit critical session
		pthread_mutex_lock(&m);
			gReaders--;
			if(gReaders == 0)
				pthread_cond_signal(&writePhase);

		pthread_mutex_unlock(&m);
	}

	return 0;
}
