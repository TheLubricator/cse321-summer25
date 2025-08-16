#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>

struct shared{
    char sel[100];
    int b;
};

int main(){
    pid_t child;
    int fd[2];
    int p=pipe(fd);
    char input[100];
    void *s_m;
    int sm_id;


    sm_id=shmget((key_t)102,sizeof(struct shared),0666|IPC_CREAT);
    s_m=shmat(sm_id,NULL,0);
    struct shared *sending_data=(struct shared *)s_m;
    sending_data->b=1000;
    printf("Provide you input from the given options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");
    scanf("%c", &input[0]);
    input[1] = '\0';

    strcpy(sending_data->sel,input);
    // printf("sending struct:\n");
    // printf("  id: %d\n", sending_data->b);
    // printf("  text: %s\n", (char *)sending_data->sel);
    child=fork();
    if  (child>0){

        wait(NULL);
        char buff[100];
        close(fd[1]);
        read(fd[0],buff,sizeof(buff));
        printf("%s",buff);
        close(fd[0]);
    }
    else if (child==0){
        close(fd[0]);
        void *s_m;
        int sm_id;

        sm_id = shmget((key_t)102, sizeof(struct shared), 0666);
        s_m = shmat(sm_id, NULL, 0);
        struct shared *receiving_data = (struct shared *)s_m;
        char rec_inp[100];
        int balance=receiving_data->b;
        strcpy(rec_inp,receiving_data->sel);
        printf("\nYour selecttion: %s\n\n",receiving_data->sel);
        if (strcmp(rec_inp, "a") == 0){
            printf("Enter amount to be added: \n");
            int add_balance;
            scanf("%d",&add_balance);
            if  (add_balance<=0){
                printf("\nAdding failed, Invalid amount\n");
            }
            else{
                balance+=add_balance;
                printf("\nBalance added successfully\nUpdated balance after addition:\n%d\n",balance);

            }

        }
        else if(strcmp(rec_inp, "w") == 0){
            printf("Enter amount to be withdrawn: \n");
            int minus_balance;
            scanf("%d",&minus_balance);
            if  (minus_balance<=0 || minus_balance>balance){
                printf("\nWithdraw failed, Invalid amount\n");
            }
            else{
                balance-=minus_balance;
                printf("\nBalance withdrawn successfully\nUpdated balance after withdrawl:\n%d\n",balance);

            }

        }
        else if(strcmp(rec_inp, "c") == 0){
            printf("Your current balance is:\n%d\n",balance);


        }
        else{
            printf("Invalid selection\n");
        }

        char buff[100];
        strcpy(buff,"Thank you for using\n");
        write(fd[1],buff,sizeof(buff));
        close(fd[0]);

    }
    shmctl(sm_id,IPC_RMID, NULL);
}
