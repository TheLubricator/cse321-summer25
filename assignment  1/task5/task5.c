#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/wait.h>


int main(){
    int status;
    pid_t child, grandchild;
    printf("Parent process ID: 0\n");
    child = fork();
    if (child == 0) {
        printf("Child process ID: %d\n", getpid());
        grandchild = fork();
        if (grandchild == 0) {
            printf("Grand Child Process ID: %d\n", getpid());
            return 0; //instant end to grandchild
        }
        wait(&status); //end of grandcchild1
        grandchild = fork();
        if (grandchild == 0) {
            printf("Grand Child Process ID: %d\n", getpid());
            return 0;
        }
        wait(&status);
        grandchild = fork();
        if (grandchild == 0) {
            printf("Grand Child Process ID: %d\n", getpid());
            return 0;
        }
        wait(&status);
    }else {
        wait(&status); //corner ccase for  parent
    }
 return 0;
}
