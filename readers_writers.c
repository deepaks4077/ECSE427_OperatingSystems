#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#define NUM_OF_READERS 10
#define NUM_OF_WRITERS 100


static int glob = 0;

static long max_writers = 0;
static long min_writers = 1000;
static long max_readers = 0;
static long min_readers = 1000;
static long avg_writer = 0;
static long avg_reader = 0;

static sem_t rw_mutex;
static sem_t mutex;
static sem_t reader_mux;
static sem_t writer_mux;

static int read_count = 0;


static void *writerFunc(void *arg){
  int loop = *((int*)arg);
  struct timeval tv;
  long before_time;
  long after_time;
  long difference;

  if(gettimeofday(&tv,NULL)==-1){
    printf("Failure getting time");
    exit(1);
  }

  before_time = tv.tv_usec;

  do {
        wait(rw_mutex);
        glob+=10;
        signal(rw_mutex);
  }while(loop--);

  if(gettimeofday(&tv,NULL)==-1){
    printf("Failure getting time");
    exit(1);
  }

  after_time = tv.tv_usec;

  wait(writer_mux);
  
    difference = (after_time-before_time);
    avg_writer += difference;

    if(difference>max_writers){
      max_writers = difference;
    }

    if(difference<min_writers){
      min_writers = difference;
    }

  signal(writer_mux);

}


static void *readerFunc(void *arg){
  int loop = *((int*)arg);
  struct timeval tv;
  long before_time;
  long after_time;
  long difference;
  
  if(gettimeofday(&tv,NULL)==-1){
    printf("Failure getting time");
    exit(1);
  }

  before_time = tv.tv_usec;

  do {
        wait(mutex);
        read_count++;
        
        if (read_count == 1){
          wait(rw_mutex);
        }
  
        signal(mutex);
        //read is performed
        wait(mutex);
        read_count--;
        
        if (read_count == 0){
          signal(rw_mutex);
        }
        signal(mutex);
      
      }while(loop--);

  if(gettimeofday(&tv,NULL)==-1){
    printf("Failure getting time");
    exit(1);
  }

  after_time = tv.tv_usec;

  wait(reader_mux);
  
    difference = (after_time-before_time);
    avg_reader += difference;

    if(difference>max_readers){
      max_readers = difference;
    }

    if(difference<min_readers){
      min_readers = difference;
    }

  signal(reader_mux);

}

int main(int argc, char *argv[]) {

  pthread_t readers[NUM_OF_READERS];
  pthread_t writers[NUM_OF_WRITERS];

  int s;
  int loops;

  if(sscanf(argv[1],"%i",&loops)==-1){
    printf("Couldn't read loop value");
    exit(1);
  } 

  printf("*************************** \n\n\n The value of loop is: %d \n\n\n",loops);

  if (sem_init(&rw_mutex, 0, 1) == -1) {
    printf("Error, init semaphore\n");
    exit(1);
  }

  if(sem_init(&mutex,0,NUM_OF_READERS) == -1){
    printf("Error, init semaphore\n");
    exit(1);
  }

  if(sem_init(&reader_mux,0,1) == -1){
    printf("Error, init semaphore\n");
    exit(1); 
  }

  if(sem_init(&writer_mux,0,1) == -1){
    printf("Error, init semaphore\n");
    exit(1); 
  }

  int i =0;
  
  for(i=0;i<NUM_OF_READERS;i++){
    s = pthread_create(readers+i, NULL, readerFunc, &loops);
    if (s != 0) {
      printf("Error, creating threads\n");
      exit(1);
    }
  }

  for(i=0;i<NUM_OF_WRITERS;i++){
    s = pthread_create(writers+i, NULL, writerFunc, &loops);
    if (s != 0) {
      printf("Error, creating threads\n");
      exit(1);
    }
  }  
  
  for(i=0;i<NUM_OF_READERS;i++){
    s = pthread_join(*(readers+i), NULL);
    if (s != 0) {
      printf("Error, creating threads\n");
      exit(1);
    }
  }

  for(i=0;i<NUM_OF_WRITERS;i++){
    s = pthread_join(*(writers+i), NULL);
    if (s != 0) {
      printf("Error, creating threads\n");
      exit(1);
    }
  }

  printf("glob value %d \n", glob);
  
  printf("The maximum waiting time for writers is: %ld microseconds \n",max_writers);
  printf("The minimum waiting time for writers is: %ld microseconds \n",min_writers);
  printf("The average waiting time for writers is: %ld microseconds \n",avg_writer/NUM_OF_WRITERS);

  printf("The maximum waiting time for readers is: %ld microseconds \n",max_readers);
  printf("The minimum waiting time for readers is: %ld microseconds \n",min_readers);
  printf("The average waiting time for readers is: %ld microseconds \n",avg_reader/NUM_OF_READERS);

  exit(0);
}
