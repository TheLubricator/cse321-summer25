#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

int *served_counter=0;
int *total_students_attempted=0;
int studen_ids[10]={0,1,2,3,4,5,6,7,8,9};
pthread_mutex_t mutex_waiting;
sem_t student_wait;
int chair_queue[3]={-1,-1,-1};

int main(){


}
