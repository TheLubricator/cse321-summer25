#include<stdio.h>
#include <stdlib.h>

int partition(int array[],int array_length,int start,int end){
    int i= start-1;
    int pivot=end;
    int j=start;
    while (j!=pivot){
        if (array[j]>array[pivot]){
            i++;
            int temp=array[j];
            array[j]=array[i];
            array[i]=temp;
        }
        j++;
    }

    i++;
    int temp=array[j];
    array[j]=array[i];
    array[i]=temp;
    return i;
    /*Note to self
     * Since all values left to 'i' is bigger and to right is smaller
     * 'i'-1 becomes the new end for left sub array, and i+1 as start for right subarray
     * this swap method goes on until start and end  is equal ie single length subarray
     */


}
void quicksort(int array[],int array_length,int start,int end){
    if (end<=start){
        return;
    }
    int pivot = partition(array,array_length,start,end);
    quicksort(array,array_length,start,pivot-1);
    quicksort(array,array_length,pivot+1,end);


}

int main (int argc,char *args[]){
    //atoi converts string to int under stdlib library
    int number_arr[argc-1];
    for  (int i=0;i<argc-1;i++){
        number_arr[i]=atoi(args[i+1]);
    }

    int arr_length=sizeof(number_arr)/sizeof(number_arr[0]);
    quicksort(number_arr,arr_length,0,arr_length-1);
    for (int i=0; i<arr_length;i++){
        printf("%d ",number_arr[i]);
    }
    printf("\n");
    return 0;

}
