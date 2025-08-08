#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *fibonacci_generator(void *args);
void *t_ret;

int main (){
    pthread_t thread1,thread2;
    int fibonacci_sequence_size,search_size;
    printf("Enter the term of the fibonacci sequence:\n");
    scanf("%d",&fibonacci_sequence_size);
    printf("How many numbers are you willing to search?\n");
    scanf("%d",&search_size);
    int search_array[search_size];
    for (int i=0;i<search_size;i++){
        int temp;
        printf("Enter search %d:\n",i+1);
        scanf("%d",&temp);
        search_array[i]=temp;

    }
    int array_size_main=fibonacci_sequence_size+1;
    pthread_create(&thread1,NULL,fibonacci_generator,&array_size_main);
    pthread_join(thread1,&t_ret);
    int *ret_fibonaccci_array= (int *)t_ret;
    for (int i=0;i<array_size_main;i++){
        printf("a[%d]= %d\n",i,ret_fibonaccci_array[i]);
    }


}

void *fibonacci_generator(void *args){
    int *size_pointer= (int *)args;
    int array_size=*size_pointer;
    int *fibonacci_array=malloc(array_size*sizeof(int));
    if (array_size==1){
        fibonacci_array[0]=0; //base case n=1
    }
    else if (array_size==2){
        fibonacci_array[0]=0; //base case 2
        fibonacci_array[1]=1; //base case 2
    }
    else{
        fibonacci_array[0]=0; //base case n>2
        fibonacci_array[1]=1; //base case n>2
        for (int i=2;i<array_size;i++){
            fibonacci_array[i]=fibonacci_array[i-1]+fibonacci_array[i-2];

        }
    }
    pthread_exit(fibonacci_array);

}
