#include<stdio.h>
#include <stdlib.h>

int main (int argc,char *args[]){
    //atoi converts string to int under stdlib library
    int number_arr[argc-1];
    for  (int i=0;i<argc-1;i++){
        number_arr[i]=atoi(args[i+1]);
    }

    int arr_length=sizeof(number_arr)/sizeof(number_arr[0]);
    for (int i=0;i<arr_length;i++){
        if (number_arr[i]%2==0){
            printf("%d at index %d is even.\n",number_arr[i],i);
        }
        else{
            printf("%d at index %d is odd.\n",number_arr[i],i);
        }
    }

}
