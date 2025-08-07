#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main (){
    int fd,status;
    FILE *file;
    pid_t a,b,c;
    int counter=8; //1 parent + 7 child processes already created so total processes is 4 from init
    file = fopen("counter.txt", "w");
    fprintf(file,"%d",counter);
    fclose(file);
    a=fork();
    b=fork();
    c=fork();

    /*if (a>0 && b>0 && c>0){  //parent
        // parent process
       // wait(NULL);

        int number;
        file = fopen("counter.txt", "r");
        fscanf(file, "%d", &number);
        printf("Final count: %d\n", number);
        fclose(file);
    }*/

    if (a>0 && b==0 && c>0){
        if (getpid()%2!=0){

            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);
            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a>0 && b==0 && c==0){
        if (getpid()%2!=0){

            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);

            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a>0 && b>0 && c==0){
        if (getpid()%2!=0){

            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);

            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a==0 && b>0 && c>0){
        if (getpid()%2!=0){

            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);
            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a==0 && b==0 && c>0){
        if (getpid()%2!=0){
            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);
            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a==0 && b==0 && c==0){
        if (getpid()%2!=0){
            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);
            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
    else if (a==0 && b>0 && c==0){
        if (getpid()%2!=0){
            int number;
            file = fopen("counter.txt", "r");
            fscanf(file, "%d", &number);
            number++;
            fclose(file);
            file = fopen("counter.txt", "w");
            fprintf(file,"%d",number);
            fclose(file);
            fork();
        }
    }
     else if (a>0 && b>0 && c>0){  //parent
        // parent process
       wait(NULL);

        int number;
        file = fopen("counter.txt", "r");
        fscanf(file, "%d", &number);
        printf("Final count: %d\n", number);
        fclose(file);
    }
    return 0;
}
