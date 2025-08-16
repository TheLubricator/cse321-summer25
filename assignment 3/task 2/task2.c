#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct msg{
    long int type;
    char text[6];
};
int main(){
    //login
    int msg_id_otp,msg_id_mail;
}
