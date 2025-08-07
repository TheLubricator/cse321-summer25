#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

void main(){
    pid_t pid_child, pid_grandchild,status1,status2;
    pid_child=fork();
    if (pid_child>0){
        wait(&status1);
        printf("I am parent.\n");
    }
    else if (pid_child==0){
        pid_grandchild=fork();
        if (pid_grandchild==0){
            printf("I am grandchild.\n");
        }
        else if(pid_grandchild>0){
            wait(&status2);
            printf("I am child.\n");
        }

    }
}
