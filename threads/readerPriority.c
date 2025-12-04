//mutual exclusion 
//readers priority
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

sem_t x, wsem;
int readCount = 0;

void READUNIT(int index){
  printf("Reader #%d is reading\n", index);
  sleep(10);
  printf("Reader #%d finished\n", index);
}

void WRITEUNIT(int index){
  printf("Writer #%d is writing\n", index);
  sleep(5);
  printf("Writer #%d finished\n", index);
}

void* reader(void* arg){
  int index = *(int*)arg;
  while(true){
    sem_wait(&x);
    readCount++;
    if(readCount == 1){
      sem_wait(&wsem);
    }
    sem_post(&x);
    
    READUNIT(index);
    
    sem_wait(&x);
    readCount--;
    if(readCount == 0){
      sem_post(&wsem);
    }
    sem_post(&x);
    
    sleep(1); // Small delay between iterations
  }
  return NULL;
}

void* writer(void* arg){
  int index = *(int*)arg;
  while(true){
    sem_wait(&wsem);
    WRITEUNIT(index);
    sem_post(&wsem);
    
    sleep(1); // Small delay between iterations
  }
  return NULL;
}

int main(){
  pthread_t readers[10];
  pthread_t writers[10];
  int ids[10];
  
  sem_init(&x, 0, 1);
  sem_init(&wsem, 0, 1);
  
  int i;
  for(i = 0; i < 10; i++){
    ids[i] = i + 1;
    pthread_create(&writers[i], NULL, writer, &ids[i]);
    pthread_create(&readers[i], NULL, reader, &ids[i]);
  }
  
  for(i = 0; i < 10; i++){
    pthread_join(readers[i], NULL);
    pthread_join(writers[i], NULL);
  }
  
  sem_destroy(&x);
  sem_destroy(&wsem);
  
  return 0;
}
