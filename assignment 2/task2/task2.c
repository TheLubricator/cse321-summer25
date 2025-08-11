#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

int served_counter=0;
int  chairs_array[3]={-1,-1,-1};
int chairs_array_idx=0;
int chairs_array_idx_st=0;
int waiting=0;
int total_students_attempted=0;
int student_ids[10]={0,1,2,3,4,5,6,7,8,9};
pthread_mutex_t mutex_waiting;
sem_t student_wait,student_chair,st_avail;


void *student_thread(void *args);
void *st_thread(void *args);

int random_number_return(int min, int max){

    int random_num= min+(rand()%(max-min+1));
    return random_num;
}

int main(){
    srand(time(NULL));
    pthread_t students[10];
    pthread_t st;
    sem_init(&student_chair,0,3);
    sem_init(&student_wait,0,0);
    sem_init(&st_avail,0,1);
    pthread_create(&st,NULL,*st_thread,NULL);
    for (int i=0;i<10;i++){
        sleep(random_number_return(0,3));
        pthread_create(&students[i],NULL,*student_thread,&student_ids[i]);

    }
    for (int i=0;i<10;i++){

        pthread_join(students[i],NULL);

    }
    pthread_join(st,NULL);

}

void *st_thread(void *args){
    while (total_students_attempted<10){
        sem_wait(&student_wait);
        sem_wait(&st_avail);
        printf("A waiting student started getting consultation\n");
        printf("Number of students now waiting: %d\n",waiting);
        printf("ST now giving consultation,\n");
        int current_id = chairs_array[chairs_array_idx_st];
        printf("Student %d started getting consultation\n", current_id);
        sleep(random_number_return(1,2));
        total_students_attempted++;
        served_counter++;

        printf("Number of students served: %d\n",served_counter);

        pthread_mutex_lock(&mutex_waiting);
        chairs_array[chairs_array_idx_st]=-1;
        chairs_array_idx_st=(chairs_array_idx_st+1)%3;
        waiting--;
        sem_post(&student_chair);
        pthread_mutex_unlock(&mutex_waiting);
        sem_post(&st_avail);




    }
    pthread_exit(NULL);
}

void *student_thread(void *args){
    int *student_id=(int *)args;
    pthread_mutex_lock(&mutex_waiting);
    if (waiting>=3){
            printf("No chairs remaining in lobby. Student %d is leaving...\n",*student_id);
            total_students_attempted++;
            pthread_mutex_unlock(&mutex_waiting);
            pthread_exit(NULL);

    }
    else{
            sem_wait(&student_chair);
            chairs_array[chairs_array_idx]=*student_id;
            waiting++;
            chairs_array_idx=(chairs_array_idx+1)%3;
            printf("Student %d started waiting for consultation.\n",*student_id);
            sem_post(&student_wait);
            pthread_mutex_unlock(&mutex_waiting);
            pthread_exit(NULL);

    }



}
