#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#include<pthread.h>

void *t_funct1(void *args);
void *t_funct2(void *args);

void *t_ret;
int thread_id[]={1,2};


int main (){
    pthread_t t[2];
    pthread_create(&t[0],NULL,t_funct1,NULL);
    pthread_join(t[0],NULL);
    pthread_create(&t[1],NULL,t_funct2,NULL);
    pthread_join(t[1],NULL);
}
void *t_funct1(void *args){
    int n;
    int count=0;
    printf("Enter a value: ");
    scanf("%d",&n);
    printf("\nFor 3 and 5: ");
    for (int i=6;i<=n;i++){
        if(i%3==0 && i%5==0){
            printf("%d ",i);
            count++;
        }
    }

    printf("\nFinal count: %d\n",count);
}
void *t_funct2(void *args){
    int m;
    int sum=0;
    printf("\nEnter a value: ");
    scanf("%d",&m);
    printf("\nAll odd numbers in range:\n");
    for (int i=1;i<=m;i++){
        if(i%2!=0){
            printf("%d ",i);
            sum+=i;
        }
    }
    printf("\nOdd no sum: %d\n",sum);
}
