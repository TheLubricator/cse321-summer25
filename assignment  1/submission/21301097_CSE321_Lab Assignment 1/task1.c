#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>

int main (int argc,char *args[]){
    int fd;
    FILE *fp;

    fd=open(args[1],O_CREAT|O_WRONLY,0644);


    while(true){
        char input[50];
        printf("Please Enter a string: ");
        scanf("%s",input);
        if (input[strlen(input)-2]=='-' && input[strlen(input)-1]=='1') {
            close(fd);
            break;
        }
        write(fd,input,strlen(input));
        write(fd,"\n",1);
        lseek(fd,0,SEEK_END);

    }
    close(fd);
    return 0;
}
