#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

int main (){
    pid_t pid;
    int status;
    int array[]={2,4,1,3,8,7,9,6,5};
    //we will manually enter these valeus in the execl command as this requires string conversion we haven't been taught
    pid=fork();
    if (pid>0){
        wait(&status);
        printf("From parent process odd and even numbers of given array\n");
        execl("./oddeven.exe", "./oddeven.exe", "2", "4", "1", "3", "8", "7", "9", "6", "5", NULL);
        return 0;
    }
    else if (pid==0){
        printf("From child process sorted array in descending order: \n");
        execl("./sort.exe", "./sort.exe", "2", "4", "1", "3", "8", "7", "9", "6", "5", NULL);

    }
    return 0;
}
