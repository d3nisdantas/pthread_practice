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

//int buffer[BUF_SIZE];
int sharedVar;
int add = 0;	/* place to add the next element */
int rem = 0;	/* place to remove the next element */
int num = 0;	/* number of elements in a buffer */
int readers = 0;
int writers = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;		/* mutex for buffer */
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER;	/* consumer waits on cv */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER;

static void *producer(void *params);
static void *consumer(void *params);

int main(void) {
	pthread_t myThreads[10];
	int *args[5];

	for(int i = 0; i < 5; i++)
	{
		//create writers
		args[i] = (int*)malloc(sizeof(int));
		if(args[i] != NULL)
			*args[i] = i;

		if (pthread_create(&myThreads[i], NULL, producer, args[i]) !=0)
		{
			fprintf(stderr, "Unable to create producer thread.\n");
			exit(1);
		}
	}

	for(int i = 5; i < 10; i++)
	{
		//create readers
		if (pthread_create(&myThreads[i], NULL, consumer, NULL) !=0)
		{
			fprintf(stderr, "Unable to create consumer thread.\n");
			exit(1);
		}
	}

	for(int i = 0; i < 10; i++)
	{
		pthread_join(myThreads[i], NULL);
	}
	for(int i = 0; i < 5; i++)
	{
		free(args[i]);
	}

	printf("parent quitting.\n");
	return 0;
}

void *producer(void *params){
	int myNumber = *((int*)params);
	int startNumber = EXEC_NUM*myNumber;

	for(int i = startNumber; i < startNumber + EXEC_NUM; i++){
		pthread_mutex_lock(&m);
			while(writers || readers){
				// a writer can only write when there are no other writers or readers executing
				pthread_cond_wait(&c_prod, &m);
			}
			writers++;
			sleep(1);
			sharedVar = i;
			writers--;
		pthread_mutex_unlock(&m);
		pthread_cond_broadcast(&c_cons);

		printf("Producer %lu inserted %d\n", pthread_self(), i); fflush(stdout);
		printf("Num of readers %d \n", readers); fflush(stdout);
	}

	printf("producer quitting\n"); fflush(stdout);
	return 0;
}

void *consumer(void *params){
	int readValue = -1;

	for(int j = 0; j < EXEC_NUM; j++){
		pthread_mutex_lock(&m);
			if (num < 0){ //underflow
				exit(1);
			}
			while(writers){
				pthread_cond_wait(&c_cons, &m);
			}
			readers++;
			sleep(1);
			readValue = sharedVar;
			readers--;
		pthread_mutex_unlock(&m);

		if(writers == 0)
			pthread_cond_signal(&c_prod);

		printf("Consumer %lu read %d\n", pthread_self(), readValue); fflush(stdout);
		printf("Num of readers %d \n", readers); fflush(stdout);
	}

	return 0;
}
