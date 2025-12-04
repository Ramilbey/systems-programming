//mutual exclusion 
//readers priority
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
sem_t x;
wsem = 1;

int readCount;

void READUNIT(int index){
  printf("Reader #%d is reading\n", index);
  sleep(10);
  printf("Reader #%d finished\n", index);
}
void WRITEUNIT(int index){
  printf("Writer %d is writing \n", index);
  sleep(5);
  printf("Writer #%d finished\n", index);
}
void* reader(){
  while(true){
    sem_wait(&x);
    readCount++;
    if(readCount ==1){
      sem_wait(wsem);
    }
    sem_post(&x);
    READUNIT();
    sem_wait(&x)
    readCount--;
    if(readCount == 0){
      sem_post(wsem);
    }
    sem_post(&x);
  }
}
void* writer(){
  int index = *(int*)arg;
  printf("Writer #%d is tying to write\n", index);
  sem_wait(&x);
  WRITEUNIT(index);
  sem_post(&x);
}

int main(){
  pthread_t readers[10];
  pthread_t writer[10];
  int ids[10];
  sem_init(&x, 0, 1);
  int i;
  for(i = 0; i < 10; i ++){
    ids[i] = i +1;
    pthread_create(&writers[i], NULL, &writer, &ids[i]);
    pthread_create(&readers[i], NULL, &reader, &ids[i]);
  }
  for(i = 0; i < 10; i++){
    pthread_join(readers[i], 0);
    pthread_join(writers[i], 0);
  }
  sem_destroy(&x);
return 0;
}
