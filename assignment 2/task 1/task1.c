#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *fibonacci_generator(void *args);
void *fibonacci_searcher(void *args);
void *t_ret;

int main (){
    pthread_t thread1,thread2;
    int fibonacci_sequence_size,search_size;
    printf("Enter the term of the fibonacci sequence:\n");
    scanf("%d",&fibonacci_sequence_size);
    printf("How many numbers are you willing to search?\n");
    scanf("%d",&search_size);




    int array_size_main=fibonacci_sequence_size+1;
    pthread_create(&thread1,NULL,fibonacci_generator,&array_size_main);
    pthread_join(thread1,&t_ret);
    int *ret_fibonaccci_array= (int *)t_ret;
    printf("The fibonacci sequence generation output of %d:\n",fibonacci_sequence_size);
    for (int i=0;i<array_size_main;i++){
        printf("a[%d]= %d\n",i,ret_fibonaccci_array[i]);
    }
    int *thread_args = malloc(2 * sizeof(int));
    thread_args[0] = array_size_main;
    thread_args[1] = search_size;
    pthread_create(&thread2,NULL,fibonacci_searcher,thread_args);
    pthread_join(thread2,NULL);


}

void *fibonacci_generator(void *args){
    int *size_pointer= (int *)args;
    int array_size=*size_pointer;
    int *fibonacci_array=malloc(array_size*sizeof(int));
    if (array_size==1){
        fibonacci_array[0]=0; //base case n=1
    }
    else if (array_size==2){
        fibonacci_array[0]=0; //base case n=2
        fibonacci_array[1]=1; //base case n=2
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
void *fibonacci_searcher(void *args){
    int *int_args = (int *)args;
    int array_size = int_args[0];
    int search_size = int_args[1];
    int *ret_fibonaccci_array= (int *)t_ret;

    for (int i=0;i<search_size;i++){
        int temp;
        int printable;
        printf("Enter search %d:\n",i+1);
        scanf("%d",&temp);
        if (temp>=0 && temp<array_size){
            printable=ret_fibonaccci_array[temp];
        }
        else{
           printable=-1;
        }
        printf("=>result of search #%d : %d\n",i+1,printable);
}
}
