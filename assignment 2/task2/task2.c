#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

int *served_counter=0;
int  *chairs_array[3]={-1,-1,-1};
int *chairs_array_idx=0;
int *waiting=0;
int *total_students_attempted=0;
int student_ids[10]={0,1,2,3,4,5,6,7,8,9};
pthread_mutex_t mutex_waiting;
sem_t student_wait,student_chair;


void student_thread(void *args);

int random_number_return(int min, int max){
    int random_num= min+(rand()%(max-min+1));
    return random_num;
}

int main(){
    pthread_t students[10];
    for (int i=0;i<10;i++){
        sleep(random_number_return(0,3));
        pthread_create(&students[i],NULL,fibonacci_generator,&student_ids[0]);

    }


}

void student_thread(void *args){
    int *student_id=(int *)args;
    pthread_mutex_lock(&mutex_waiting);
    if waiting>3:
            printf("No chairs remaining in lobby. Studen %d is leaving...\n",student_id);
            total_students_attempted++;
            pthread_mutex_unlock(&mutex_waiting);
            pthread_exit(NULL);

}
