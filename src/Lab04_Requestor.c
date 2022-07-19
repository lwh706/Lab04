#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

void signal_handler(int);
//為了讓signal handler收到而設定為全域變數
int msqid;
void *shmaddr;
int shmid;
 
// structure for message queue
struct msg_buffer{
    long msg_type;
    char msg_key[100];
    char msg_leng[10];
    char msg_rand[10];
    char msg_sent[100];
    char msg_repl[100];
    char shm_sent[100];
    char shm_repl[100];
    int shm_flag;                   //用來切換Requestor/Replyer
}message;
 
int main(){
    
    //sigaction
    struct sigaction act;
    act.sa_handler=signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGINT,&act,NULL);
    sigaction(SIGTSTP,&act,NULL);
    sigaction(SIGQUIT,&act,NULL);

    //Message Queue

    //msgget creates a message queue and returns identifier
    key_t key = (key_t) 0x99990706;
    strcpy(message.msg_key, "0x99990706"); 
    msqid = msgget(key, 0666 | IPC_CREAT);
    //Message Type
    message.msg_type = 2;
    //設定隨機數0~999
    srand(time(NULL));                                                  //設置隨機數種子
    sprintf(message.msg_rand, "%d", rand() % 1000);                     //將隨機數轉為字串存入rand_num
    //將訊息存入msg_sent
    strcat(message.msg_sent, "Request : 23219731 ");                    //固定訊息
    strcat(message.msg_sent, message.msg_rand);                         //固定訊息+隨機數
    //計算訊息長度
    int length = strlen(message.msg_sent);
    sprintf(message.msg_leng, "%d", length);
    //msgsnd()送出訊息
    if(msgsnd(msqid, &message, sizeof(message), 0) < 0){
        printf("Send message queue failed");
        exit(EXIT_FAILURE);
    }
    else{
        //顯示送出畫面
        printf("Send MQ Queue Key : %s\n", message.msg_key);
        printf("Send MQ Message Type : %ld\n", message.msg_type);
        printf("Send MQ Message Length : %s\n", message.msg_leng);
        printf("Send MQ Message Content : %s\n", message.msg_sent);
        printf("------------------------------------------------------------\n");
    }

    //msgrcv()接收訊息
    printf("Waiting message...\n");
    if(msgrcv(msqid, &message, sizeof(message), 1, 0) < 0){
        printf("Send message queue failed");
        exit(EXIT_FAILURE);
    }
    else{
        //顯示接收畫面
        printf("Recieve MQ Queue Key : %s\n", message.msg_key);
        printf("Recieve MQ Message Type : %ld\n", message.msg_type);
        printf("Recieve MQ Message Length : %s\n", message.msg_leng);
        printf("Recieve MQ Message Content : %s\n", message.msg_repl);
        printf("------------------------------------------------------------\n");
    }
    //清除MQ
    msgctl(msqid, IPC_RMID, NULL);
    
    //隨機睡眠
    int j, k;
    k = rand() % 11;
    printf("sleep %d second\n", k);
    for( j=0; j<k; j++){
        sleep(1);
        printf(".\n");
    }

    //Shared Memory
    struct msg_buffer *msgaddr;

    //shmget take sizeof(message) for shared memory
    key = (key_t) 0x99990317;
    shmid = shmget(key, sizeof(message), 0666 | IPC_CREAT);
    if(shmid==-1){
        printf("fail to shmget\n");
        exit(1);
    }
    //attach
    shmaddr = shmat(shmid, NULL, 0);
    if(shmaddr == (void*)-1){
        printf("fail to shmat\n");
        exit(1);
    }
    //change addr
    msgaddr = (struct msg_buffer *)shmaddr;
    //share
    strcpy(msgaddr->msg_key, "0x99990317");
    msgaddr->msg_type = 2;
    sprintf(msgaddr->msg_rand, "%d", rand() % 1000);
    strcat(msgaddr->shm_sent, "Request : 13791232 ");
    strcat(msgaddr->shm_sent, msgaddr->msg_rand);
    length = strlen(msgaddr->shm_sent);
    sprintf(msgaddr->msg_leng, "%d", length);
    msgaddr->shm_flag = 2;
    //顯示share內容
    printf("Send shm Queue Key : %s\n", msgaddr->msg_key);
    printf("Send shm Message Type : %ld\n", msgaddr->msg_type);
    printf("Send shm Message Length : %s\n", msgaddr->msg_leng);
    printf("Send shm Message Content : %s\n", msgaddr->shm_sent);
    printf("------------------------------------------------------------\n");
    
    //接收
    int flag = 1;
    while(flag){
        if(msgaddr->shm_flag == 1){
            printf("Recieve shm Queue Key : %s\n", msgaddr->msg_key);
            printf("Recieve shm Message Type : %ld\n", msgaddr->msg_type);
            printf("Recieve shm Message Length : %s\n", msgaddr->msg_leng);
            printf("Recieve shm Message Content : %s\n", msgaddr->shm_repl);
            printf("------------------------------------------------------------\n");
            flag = 0;
        }
        else{
            printf("Waiting message, retry in 3 sec ...\n");
            sleep(3);
        }
    }
    //detach
    shmdt(shmaddr);
    //清除shm內容
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}
//如果強制中斷馬上清除MQ、shm
void signal_handler(int singo){
    if(singo==2){
        printf("\nSIGINT Caught!\n");
    }
    else if(singo==20){
        printf("\nSIGTSTP Caught!\n");
    }
    else{
        printf("\nSIGNAL Caught!\n");
    }
    //清除MQ、shm
    msgctl(msqid, IPC_RMID, NULL);
    shmdt(shmaddr);
    shmctl(shmid, IPC_RMID, 0);
    exit(0);
}