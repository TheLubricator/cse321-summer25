#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

struct msg{
    long int type;
    char text[6];
};
int main(){
    //login
    pid_t otp;
    struct msg s_data;
    char workspace_name[7];
    int msg_id_login2otp,msg_id_mail2login,msg_id_otp2login,msg_id_otp2mail;
    int sent_login2otp;
    // char pidd [100];
    // sprintf(pidd,"%d",getpid());
    // printf("%s",pidd);
    printf("Please enter workspace name: \n");
    read(0,workspace_name,sizeof(workspace_name));
    workspace_name[6]='\0';

    if (strcmp(workspace_name,"cse321")!=0){
        printf("Invalid workspace name.\n");
        return 0;
    }
    strcpy(s_data.text,workspace_name);
    printf("Workspace name sent to OTP Generator from login: %s\n",s_data.text);
    strcpy(s_data.text,workspace_name);
    s_data.type=1;
    msg_id_login2otp=msgget((key_t)12,0666|IPC_CREAT);
    msgsnd(msg_id_login2otp,(void *)&s_data,sizeof(s_data),0);
    otp=fork();
    if (otp>0){
        wait(NULL);
        struct msg r_data_otp2login,r_data_mail2login;
        msg_id_otp2login=msgget((key_t)12,0666|IPC_CREAT);
        msgrcv(msg_id_otp2login,(void *)&r_data_otp2login,sizeof(r_data_otp2login),2,IPC_NOWAIT);

        msg_id_mail2login=msgget((key_t)12,0666|IPC_CREAT);
        msgrcv(msg_id_mail2login,(void *)&r_data_mail2login,sizeof(r_data_mail2login),4,IPC_NOWAIT);
        printf("\nLog in received OTP from OTP Generator: %s\n\n",r_data_otp2login.text);

        printf("Log in received OTP from mail: %s\n\n",r_data_mail2login.text);
        if (strcmp(r_data_otp2login.text,r_data_mail2login.text)==0){
            printf("OTP Verified.\n\n");
        }
        else{
            printf("OTP not matched.\n\n");
        }
         msgctl(msg_id_login2otp,IPC_RMID,0);
         msgctl(msg_id_mail2login,IPC_RMID,0);
         msgctl(msg_id_otp2login,IPC_RMID,0);
         msgctl(msg_id_otp2mail,IPC_RMID,0);
    }
    else if (otp==0){
        pid_t mail;
        struct msg r_data_login2otp;
        msg_id_login2otp=msgget((key_t)12,0666|IPC_CREAT);
        msgrcv(msg_id_login2otp,(void *)&r_data_login2otp,sizeof(r_data_login2otp),1,IPC_NOWAIT);
        printf("OTP Generator received workspace name from log in: %s\n\n",r_data_login2otp.text);
        char pid2otp [6];
        sprintf(pid2otp,"%d",getpid());
        struct msg send_data_otp2mail,send_data_otp2login;
        strcpy(send_data_otp2mail.text,pid2otp);
        strcpy(send_data_otp2login.text,pid2otp);
        send_data_otp2login.type=2;
        send_data_otp2mail.type=3;
        msg_id_otp2login=msgget((key_t)12,0666|IPC_CREAT);
        msgsnd(msg_id_otp2login,(void *)&send_data_otp2login,sizeof(send_data_otp2login),0);
        printf("OTP sent to log in from OTP Generator: %s\n",send_data_otp2login.text);
        msg_id_otp2mail=msgget((key_t)12,0666|IPC_CREAT);
        msgsnd(msg_id_otp2mail,(void *)&send_data_otp2mail,sizeof(send_data_otp2mail),0);
        printf("OTP sent to mail from OTP Generator: %s\n",send_data_otp2mail.text);
        mail=fork();
        if (mail>0){
            wait(NULL);
        }
        else if (mail==0){
            struct msg r_data_otp2mail,send_data_mail2login;
            msg_id_otp2mail=msgget((key_t)12,0666|IPC_CREAT);
            msgrcv(msg_id_otp2mail,(void *)&r_data_otp2mail,sizeof(r_data_otp2mail),3,IPC_NOWAIT);
            printf("Mail received OTP from OTP Generator: %s\n",r_data_otp2mail.text);
            strcpy(send_data_mail2login.text,r_data_otp2mail.text);
            send_data_mail2login.type=4;
            msg_id_mail2login=msgget((key_t)12,0666|IPC_CREAT);
            msgsnd(msg_id_mail2login,(void *)&send_data_mail2login,sizeof(send_data_mail2login),0);
            printf("OTP sent to log in from mail: %s\n",send_data_mail2login.text);

        }


    }

}
