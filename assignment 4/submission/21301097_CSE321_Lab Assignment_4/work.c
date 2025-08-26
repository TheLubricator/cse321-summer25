#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX_USERS 5
#define MAX_RESOURCES 5
#define MAX_NAME_LEN 20
typedef enum{
    READ=1, WRITE=2, EXECUTE=4
}Permission;
//User and Resource Definitions
typedef struct{
    char name[MAX_NAME_LEN];
}User;
typedef struct{
    char file_name[MAX_NAME_LEN];
}Resource;

//ACL Entry
typedef struct{
    //to do
    char name[MAX_NAME_LEN];
    int permissions;
}ACLEntry;

typedef struct{
    char resource[MAX_NAME_LEN];
    ACLEntry acl_entries[MAX_USERS];
    int ACLCount;

}ACLControlledResource;
//Capability Entry
typedef struct{
    //to do
    char resource[MAX_NAME_LEN];
    int permissions;
}Capability;
typedef struct{
    //to do
    char  name[MAX_NAME_LEN];
    Capability capabities[MAX_RESOURCES];
    int CapabilityCount;
}CapabilityUser;
//Utility Functions
void printPermissions(int perm){
    if (perm==READ){
        printf("READ");
    }
    if (perm==WRITE){
        printf("WRITE");
    }
    if (perm==EXECUTE){
        printf("EXECUTE");

    }

}

int hasPermission(int userPerm, int requiredPerm){
    //to do
    if ((userPerm&requiredPerm)==requiredPerm){
        return 1;
    }
    else{
        return 0;
    }
}
//ACL System
void checkACLAccess(ACLControlledResource *res, const char *userName, int perm) {

    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(res->acl_entries[i].name, userName) == 0) {
            if (hasPermission(res->acl_entries[i].permissions, perm)) {
                printf("ACL Check: User %s requests ", userName);
                printPermissions(perm);
                printf(" on %s: Access GRANTED\n", res->resource);
            } else {
                printf("ACL Check: User %s requests ", userName);
                printPermissions(perm);
                printf(" on %s: Access DENIED\n", res->resource);
            }
            return;
        }
    }

    printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n", userName, res->resource);
}

//Capability System
void checkCapabilityAccess(CapabilityUser *user, const char *resourceName,int perm){
    //to do

    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (strcmp(user->capabities[i].resource, resourceName) == 0) {
            if (hasPermission(user->capabities[i].permissions, perm)) {
                printf("Capability Check: User %s requests ", user->name);
                printPermissions(perm);
                printf(" on %s: Access GRANTED\n", user->capabities[i].resource);
            } else {
                printf("Capability Check: User %s requests ", user->name);
                printPermissions(perm);
                printf(" on %s: Access DENIED\n", user->capabities[i].resource);
            }
            return;
        }
    }

    printf("Capability Check: User %s has NO entry for resource %s: Access DENIED\n", user->name, resourceName);
}


void AddACLEntry(ACLEntry *entry,char *name,int permit){
    strcpy(entry->name,name);
    entry->permissions=permit;
    printf("ACL Entry Made successfully for %s\n",entry->name);

}
void AddCapability(Capability *entry,char *resource,int permit){
    strcpy(entry->resource,resource);
    entry->permissions=permit;
    printf("Capability Made successfully for %s\n",entry->resource);

}
int DecodeOptionToPermission(char *s){
    if (strcmp("a",s)==0){
        return READ;
    }
    else if(strcmp("b",s)==0){
        printf("%d",WRITE);
        return WRITE;
    }
    else if(strcmp("c",s)==0){
        return EXECUTE;
    }
    else if(strcmp("d",s)==0){
        return READ|WRITE;
    }
    else if(strcmp("e",s)==0){
        return READ|EXECUTE;
    }
    else if(strcmp("f",s)==0){
        return WRITE|EXECUTE;
    }
    else if(strcmp("g",s)==0){
        return READ|WRITE|EXECUTE;
    }
}
int main(){
    //Sample users and resources
    User users[MAX_USERS] = {{"Alice"}, {"Bob"}, {"Charlie"}};
    Resource resources[MAX_RESOURCES] = {{"File1"}, {"File2"}, {"File3"},{"File4"},{"File5"}};
    int current_open=3;

    ACLControlledResource file_list[MAX_RESOURCES];

    for (int i=current_open;i<MAX_USERS;i++){
        char buff[MAX_NAME_LEN];
        if((i%2)==1){
        char buff[MAX_NAME_LEN]="Goku";
        }
        else{
        char buff[MAX_NAME_LEN]="Jiren";
        }
        strcpy(users[current_open].name,buff);
        printf("\nUsername of %s initialized\n",users[current_open].name);
        current_open++;
    }


    if ((READ|EXECUTE)&WRITE==EXECUTE){
        printf("True\n");
    }
    //ACL Setup
    for (int j=0;j<MAX_RESOURCES;j++){
        strcpy(file_list[j].resource,resources[j].file_name);
    for (int i=0;i<MAX_USERS;i++){
        printf("ACL: Please pick permission for %s for Username: %s\n",file_list[j].resource,users[i].name);
        printf("a. Read Only\n");
        printf("b. Write Only\n");
        printf("c. Execute Only\n");
        printf("d. Read-Write\n");
        printf("e. Read-Execute\n");
        printf("f. Write-Execute\n");
        printf("g. Read-Write-Execute\n");
        printf("h. None\n");
        char inp_buff[2];
        scanf(" %c",&inp_buff[0]);
        inp_buff[1]='\0';
        if (strcmp(inp_buff,"h")!=0){


        int  permission_mode=0;
        permission_mode=DecodeOptionToPermission(inp_buff);

        AddACLEntry(&file_list[j].acl_entries[i],users[i].name,permission_mode);
        }




    }
    file_list[j].ACLCount+=1;
    }

    //
    // //to do
    //
    //
    // //to do
    // //Test ACL
    //
    //
    //
        checkACLAccess(&file_list[0],"Alice",READ);
        checkACLAccess(&file_list[0],"Bob",WRITE);
        checkACLAccess(&file_list[0],"Charlie",READ);
        checkACLAccess(&file_list[3],"Goku",EXECUTE);
        checkACLAccess(&file_list[3],"Jiren",READ);
        checkACLAccess(&file_list[2],"Goku",WRITE);
        checkACLAccess(&file_list[3],"Jiren",EXECUTE);
        checkACLAccess(&file_list[4],"Alice",EXECUTE);
        checkACLAccess(&file_list[4],"Bob",READ);


        //Capability Setup
        CapabilityUser users_list[MAX_USERS];
        //usernamee alice
        strcpy(users_list[0].name,users[0].name);
        printf("For  user %s:\n",users_list[0].name);
        AddCapability(&users_list[0].capabities[0],resources[0].file_name,WRITE);
        AddCapability(&users_list[0].capabities[1],resources[1].file_name,WRITE|EXECUTE);
        AddCapability(&users_list[0].capabities[2],resources[2].file_name,READ|WRITE|EXECUTE);
        AddCapability(&users_list[0].capabities[3],resources[3].file_name,READ|EXECUTE);
        AddCapability(&users_list[0].capabities[4],resources[4].file_name,READ);
        //usernamee Bob
        strcpy(users_list[1].name,users[1].name);
        printf("For  user %s:\n",users_list[1].name);
        AddCapability(&users_list[1].capabities[0],resources[0].file_name,READ);
        AddCapability(&users_list[1].capabities[1],resources[1].file_name,WRITE|EXECUTE);
        AddCapability(&users_list[1].capabities[2],resources[2].file_name,EXECUTE);
        AddCapability(&users_list[1].capabities[3],resources[3].file_name,WRITE);
        AddCapability(&users_list[1].capabities[4],resources[4].file_name,READ|WRITE);
        //usernamee Charlie
        strcpy(users_list[2].name,users[2].name);
        printf("For  user %s:\n",users_list[2].name);
        AddCapability(&users_list[2].capabities[0],resources[0].file_name,READ);

        AddCapability(&users_list[2].capabities[2],resources[2].file_name,EXECUTE);
        AddCapability(&users_list[2].capabities[3],resources[3].file_name,WRITE);
        AddCapability(&users_list[2].capabities[4],resources[4].file_name,READ|WRITE);
        //usernamee Goku
        strcpy(users_list[3].name,users[3].name);
        printf("For  user %s:\n",users_list[3].name);
        AddCapability(&users_list[3].capabities[0],resources[0].file_name,READ);
        AddCapability(&users_list[3].capabities[1],resources[1].file_name,WRITE|EXECUTE);
        AddCapability(&users_list[3].capabities[2],resources[2].file_name,EXECUTE);

        AddCapability(&users_list[3].capabities[4],resources[4].file_name,READ|WRITE);

        //usernamee Jiren
        strcpy(users_list[4].name,users[4].name);
        printf("For  user %s:\n",users_list[4].name);
        AddCapability(&users_list[4].capabities[0],resources[0].file_name,READ);
        AddCapability(&users_list[4].capabities[1],resources[1].file_name,WRITE|EXECUTE);
        AddCapability(&users_list[4].capabities[2],resources[2].file_name,EXECUTE);
        AddCapability(&users_list[4].capabities[3],resources[3].file_name,WRITE);
        AddCapability(&users_list[4].capabities[4],resources[4].file_name,READ|WRITE);


    //to do
    //Test Capability
        checkCapabilityAccess(&users_list[0],"File1",WRITE);
         checkCapabilityAccess(&users_list[1],"File1",WRITE);
         checkCapabilityAccess(&users_list[2],"File2",WRITE);

         checkCapabilityAccess(&users_list[3],"File3",EXECUTE);
         checkCapabilityAccess(&users_list[3],"File4",EXECUTE);
         checkCapabilityAccess(&users_list[4],"File3",EXECUTE);
         checkCapabilityAccess(&users_list[4],"File2",WRITE);
         checkCapabilityAccess(&users_list[1],"File2",EXECUTE);
         checkCapabilityAccess(&users_list[0],"File5",EXECUTE);
    //to do
    return 0;
}
