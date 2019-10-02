#include "sharedMem.h"

sem_t mutex;
char *memString;

float randomAdd(int seconds, float nano){
    int num = (rand() % (1000000 - 1 + 1)) + 1;
    float added = ((float)num / 1000000) + seconds + nano;
    return added;
}

int getSeconds(){
    key_t key = 66;
    int secID = shmget(key, 1024, 0666|IPC_CREAT);
    char *tempTime = (char*) shmat(secID, (void*)0, 0);
    int seconds = atoi(tempTime);
    printf("%d seconds\n", seconds);
    shmdt(tempTime);
    return seconds;
}
float getNano(){
    key_t key = 67;
    int nanoID = shmget(key, 1024, 0666|IPC_CREAT);
    char *tempTime = (char*) shmat(nanoID, (void*)0, 0);
    printf("%s nano\n", tempTime);
    float nano = (float)(atoi(tempTime)) / 1000000000;
    shmdt(tempTime);
    return nano;
}

void* thread(void* arg){
    int check = 0, hasMessage = 0, seconds = getSeconds();
    float nano = getNano(), time = randomAdd(seconds, nano);
    key_t key = ftok("shmfile",64);
    int shmid = shmget(key, 1024, 0444|IPC_CREAT);
    char *str = (char*) shmat(shmid,(void*)0,0);
    if(sizeof(str) > 0){
        if(str == "yes"){
            hasMessage = 1;
        }
    }
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);

    sem_wait(&mutex);
    if(hasMessage == 1){
        key_t key = ftok("shmfile",65);
        int shmid = shmget(key, 1024, 0444|IPC_CREAT);
        char *str = (char*) shmat(shmid,(void*)0,0);
        printf("Message: %s\n", str);
        shmdt(str);
        shmctl(shmid, IPC_RMID, NULL);
    }
    if(memString != NULL){
            printf("HERE IN NOT NULL\n");
            key_t checkKey = ftok("key", 64);
            int checkID = shmget(checkKey, 1024, 0666|IPC_CREAT);
            char *str = (char*) shmat(checkID, (void*)0, 0);
            strcpy(str, "yes");
            shmdt(str);
    }
    else if(memString == NULL && (seconds + nano) >= time ){
            key_t key = ftok("testing", 65);
            int shmid = shmget(key, 1024, 0666|IPC_CREAT);
            char *str = (char*) shmat(shmid,(void*)0,0);
            printf("HERE IN MEMSTRING\n");
            strcpy(str, "TERMINATED\n");
            shmdt(str);
    }
    else if((seconds + nano) <= time){
            printf("%d and %f is <= %f\n", seconds, nano, time);
    }
    sem_post(&mutex);
}

main(){

    printf("Hello World!\n");

    sem_init(&mutex, 0, 1);
    pthread_t t;
    pthread_create(&t, NULL, thread, NULL);
    pthread_join(t, NULL);
    sem_destroy(&mutex);
}