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

#define BUF_SIZE 3 /* size of shared buffer */

int buffer[BUF_SIZE];
int add = 0;	/* place to add the next element */
int rem = 0;	/* place to remove the next element */
int num = 0;	/* number of elements in a buffer */

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;		/* mutex for buffer */
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER;	/* consumer waits on cv */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER;

void *producer(void *params);
void *consumer(void *params);

int main(void) {
	pthread_t tid1, tid2;
	int i;

	if (pthread_create(&tid1, NULL, producer, NULL) !=0){
		fprintf(stderr, "Unable to create producer thread.\n");
		exit(1);
	}

	if (pthread_create(&tid2, NULL, consumer, NULL) !=0){
		fprintf(stderr, "Unable to create consumer thread.\n");
		exit(1);
	}

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	printf("parent quitting.\n");
}

void *producer(void *parms){
	int i;
	for(i = 1; i <= 20; i++){
		pthread_mutex_lock(&m);
			if(num > BUF_SIZE){ //overflow
				exit(1);
			}
			while(num == BUF_SIZE){
				pthread_cond_wait(&c_prod, &m);
			}
			buffer[add] = i; // buffer not full, so add element
			add = (add+1) % BUF_SIZE;
			num++;
		pthread_mutex_unlock(&m);
		pthread_cond_signal(&c_cons);
		printf("producer inserted %d\n", i); fflush(stdout);
	}

	printf("producer quitting\n"); fflush(stdout);
	return 0;
}

void *consumer(void *params){
	int i;

	while(1){
		pthread_mutex_lock(&m);
			if (num < 0){ //underflow
				exit(1);
			}
			while(num == 0){
				pthread_cond_wait(&c_cons, &m);
			}
			i = buffer[rem]; // buffer not empty, so remove element
			rem = (rem+1) % BUF_SIZE;
			num --;
		pthread_mutex_unlock(&m);

		pthread_cond_signal(&c_prod);
		printf("Consume value %d\n", i); fflush(stdout);
	}
}
