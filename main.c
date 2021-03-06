//Program will be passed three argv: 1. running time 2. number of producers 3. number of consumers
//producers and consumers will access shared buffer at random intervals and output their actions and buffer contents 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


//shared buffer
typedef struct {
    int buffer[5];  
    int in;         	  
    int out;        	  
    sem_t full;     	  
    sem_t empty;    	 
    sem_t mutex;    
} sharedBuffer;

sharedBuffer sharedBuff;

//will be outputting buffer contents frequently for testing purposes
void printBuffer(int inVal, int outVal){
	int i;
	printf("\n");
	for(i=0;i<5;i++){
		if(sharedBuff.buffer[i]>0){
			printf("[%d]", sharedBuff.buffer[i]);
		}
		else{
			printf("[empty]");
		}
	}
	printf("  in=%i, out=%i", inVal,outVal);
}


//remove item at location that 'out' is currently pointing
//return value of item to consumer for output
int remove_item(){
	
	int removedItem;
	//critical section, check if full>0
	sem_wait(&sharedBuff.full);
	//use mutex binary semaphore
	sem_wait(&sharedBuff.mutex);
	removedItem = sharedBuff.buffer[sharedBuff.out];
	sharedBuff.buffer[sharedBuff.out] = -1;
	printf("\nremove_item removed item %i at position %i", removedItem,
	((sharedBuff.out)%5));
	
	sharedBuff.out = (sharedBuff.out + 1) % 5;
	
	printBuffer(sharedBuff.in,sharedBuff.out);
	//release mutex
	sem_post(&sharedBuff.mutex);

	sem_post(&sharedBuff.empty);
	
	return removedItem;
}

//inserts item into buffer at position'in' is pointing too
int insert_item(int item){

	//critical section
	sem_wait(&sharedBuff.empty);
	//use mutex binary semaphore
	sem_wait(&sharedBuff.mutex);
	sharedBuff.buffer[sharedBuff.in] = item;
	printf("\nInsert_item inserted item %i at position %i", item,
	((sharedBuff.in)%5));
	sharedBuff.in = (sharedBuff.in + 1) % 5;
	
	
	printBuffer(sharedBuff.in, sharedBuff.out);
	//release mutex
	sem_post(&sharedBuff.mutex);

	sem_post(&sharedBuff.full);
	return 1;
}

//PRODUCER####
//sleep for rand time
//produce rand item
//call insert_item
//output results of insert
void *producer(void *param){
	printf("\nCreating producer thread id: %ul", pthread_self());

	while(1){
		int sleepTime = rand()%3+1;
		printf("\nproducer thread %ul sleeping for %i seconds", pthread_self(), sleepTime);
		sleep(sleepTime);
		int produceItem = rand()%100+1;
		if(insert_item(produceItem) < 0){
			printf("producer failed\n");
		}
		else{

			printf("\nproducer thread %ul inserted value %i", pthread_self(), produceItem);
		}
	}
}

//CONSUMER####
//sleep for rand time
//call remove_item
//output results of remove
void *consumer(void *param){
	printf("\nCreating consumer thread id: %ul", pthread_self());

	int currentItem;
	while(1){
		int sleepTime = rand()%3+1;
		printf("\nconsumer thread %ul sleeping for %i seconds", pthread_self(), sleepTime);
		sleep(sleepTime);
		currentItem = remove_item();
			printf("\nconsumer thread %ul removed value %i", pthread_self(), currentItem);
		
	}
}


//MAIN####
//check cmd line args
//create threads/semaphores
//sleep
//wake up and kill threads
int main(int argc, char * argv[]){
	
	//---CHECK FOR VALID ARGS---
	if(argc < 3){
		printf("invalid arguments");
		return -1;
	}

	if(atoi(argv[1]) < 1 || atoi(argv[2]) < 1 || atoi(argv[3]) < 1){
		printf("arguments must be greater than 0");
		return -1;
	}

	

	printf("Main thread beginning.\n");
	//translate argv to ints, set up variables
	int sleepTime = atoi(argv[1]);
	int producers = atoi(argv[2]);
	int consumers = atoi(argv[3]);
	int i;
	
	//initialize buffer to -1 for empty checks
	for(i=0;i<5;i++){
		sharedBuff.buffer[i] = -1;
	}
	

	//set up pthread, mutex and semaphores
	pthread_t tid;
	//initialize mutex here
	sem_init(&sharedBuff.mutex,0,1);
	sem_init(&sharedBuff.full,0,0);
	sem_init(&sharedBuff.empty,0,5);

	//create producers
	for(i=0;i<producers;i++){
	pthread_create(&tid,NULL,consumer,0);
	   //%ul for printing tid
	}
	
	//create consumers
	for(i=0;i<consumers;i++){
	pthread_create(&tid,NULL,producer,0);
	
	}
	
	printf("\nMain thread sleeping for %i", sleepTime);
	//sleep
	sleep(sleepTime);

	
	//kill threads
	printf("\nMain thread exiting\n");
	pthread_cancel(tid);
	return 0;
}
