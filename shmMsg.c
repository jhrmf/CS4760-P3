#include "sharedMem.h"

sem_t mutex;
char *memString;

int getSeconds(){
    key_t key = ftok("shmfile", 66);
    int secID = shmget(key, 1024, 0666|IPC_CREAT);
    char *tempTime = (char*) shmat(secID, (void*)0, 0);
    shmdt(tempTime);
    shmctl(secID, IPC_RMID, NULL);
    return 1;
}
int getNano(){
    key_t key = ftok("shmfile", 67);
    int nanoID = shmget(key, 1024, 0666|IPC_CREAT);
    char *tempTime = (char*) shmat(nanoID, (void*)0, 0);
    shmdt(tempTime);
    shmctl(nanoID, IPC_RMID, NULL);
    return 2;
}

void* thread(void* arg){
    int check = 0, hasMessage = 0;
    float time = getSeconds() + (getNano() / 1000000000);
    sem_wait(&mutex);
    do{
        if(hasMessage == 1){
            key_t key = ftok("shmfile",65);
            int shmid = shmget(key, 1024, 0444|IPC_CREAT);
            char *str = (char*) shmat(shmid,(void*)0,0);
            printf("Message: %s\n", str);
            shmdt(str);
            shmctl(shmid, IPC_RMID, NULL);
            break;
        }
        if(memString != NULL){
            hasMessage = 1;
        }
        else if(memString == "" && (getSeconds() + (getNano() / 1000000000)) >= time ){
            key_t key = ftok("testing", 65);
            int shmid = shmget(key, 1024, 0666|IPC_CREAT);
            char *str = (char*) shmat(shmid,(void*)0,0);
            strcpy(str, "TERMINATED\n");
            shmdt(str);
            break;
        }
        else if((getSeconds() + (getNano() / 1000000000)) <= time){
            check = 1;
            break;
        }
    }while(1);
    sem_post(&mutex);
    if(check == 1){
        thread(arg);
    }
}

main(){

    printf("Hello World!\n");
    key_t key = ftok("shmfile",65);
    int shmid = shmget(key, 1024, 0444|IPC_CREAT);
    char *str = (char*) shmat(shmid,(void*)0,0);
    memString = str;
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);

    sem_init(&mutex, 0, 1);
    pthread_t t;
    pthread_create(&t, NULL, thread, NULL);
    pthread_join(t, NULL);
    sem_destroy(&mutex);
    return 0;
}