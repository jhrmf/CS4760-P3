#include "sharedMem.h"

sem_t mutex;
char *memString = NULL;

float randomAdd(int seconds, float nano){
    int num = (rand() % (1000000 - 1 + 1)) + 1;
    float added = ((float)num / 1000000) + seconds + nano;
    return added;
}

int getSeconds(){
    key_t key = 66;
    int secID = shmget(key, 1024, 0444);
    char *tempTime = (char*) shmat(secID, (void*)0, 0);
    int seconds = atoi(tempTime);
    shmdt(tempTime);
    return seconds;
}
float getNano(){
    key_t key = 67;
    int nanoID = shmget(key, 1024, 0444);
    char *tempTime = (char*) shmat(nanoID, (void*)0, 0);
    float nano = (float)(atoi(tempTime)) / 1000000000;
    shmdt(tempTime);
    return nano;
}

void* thread(void* arg){
    int hasMessage = 0, seconds = getSeconds();
    float nano = getNano(), time = randomAdd(seconds, nano);
    do {
        seconds = getSeconds();
        nano = getNano();
        sem_wait(&mutex);
        if (hasMessage == 1) {
            hasMessage = 0;
            key_t key = 65;
            int shmid = shmget(key, 1024, 0444);
            char *str = (char *) shmat(shmid, (void *) 0, 0);
            printf("Message: %s\n", str);
            shmdt(str);
            shmctl(shmid, IPC_RMID, NULL);
            memString = NULL;
            break;
        }
        if (memString != NULL) {
            printf("HERE IN NOT NULL\n");
            hasMessage = 1;
        } else if (memString == NULL && (seconds + nano) >= time) {
            key_t key = 65;
            int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
            char *str = (char *) shmat(shmid, (void *) 0, 0);
            printf("HERE IN MEMSTRING\n");
            float tempTime = seconds + nano;
            char timeStr[10];
            sprintf(timeStr, "%f", tempTime);
            strcpy(str, timeStr);
            memString = "BLAH";
            shmdt(str);
        } else if ((seconds + nano) <= time) {
        //    printf("%d and %f is <= %f\n", seconds, nano, time);
        }
        sem_post(&mutex);
    }while(1);

}

main(){

    sem_init(&mutex, 0, 1);
    pthread_t t;
    pthread_create(&t, NULL, thread, NULL);
    pthread_join(t, NULL);
    sem_destroy(&mutex);
}