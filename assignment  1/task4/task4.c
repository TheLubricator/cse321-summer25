#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

int main (int argc,char *args[]){
    pid_t pid;
    int status;
    int array[]={2,4,1,3,8,7,9,6,5};
    pid=fork();
    if (pid>0){
        wait(&status);
        return 0;
    }
    else if (pid==0){
        char *args[size+2];

        execv("./","sort.exe",)
    }

}
