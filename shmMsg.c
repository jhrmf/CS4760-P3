#include "sharedMem.h"

sem_t mutex;                                                                                      //needed for semaphore

char *memString = NULL;                                    //is an indicator of when there is a message in shared memory

float randomAdd(int seconds, float nano){      //function for adding a random number between 100000 and 1 to passed time
    int num = (rand() % (1000000 - 1 + 1)) + 1;
    float added = ((float)num / 1000000) + seconds + nano;
    return added;
}

int getSeconds(){                               //function for retrieving the second in our time stored in shared memory
    key_t key = 66;
    int secID = shmget(key, 2048, 0444);
    char *tempTime = (char*) shmat(secID, (void*)0, 0);
    int seconds = atoi(tempTime);
    shmdt(tempTime);
    return seconds;
}
float getNano(){                            //function for retrieving the nanosecond our in time stored in shared memory
    key_t key = 67;
    int nanoID = shmget(key, 2048, 0444);
    char *tempTime = (char*) shmat(nanoID, (void*)0, 0);
    float nano = (float)(atoi(tempTime)) / 1000000000;
    shmdt(tempTime);
    return nano;
}

void threadFunc(){                                                                   //function that holds our semaphore
    int hasMessage = 0, seconds = getSeconds();                //hasMessage is used for checking and acting by iteration
    float nano = getNano(), time = randomAdd(seconds, nano);                          //we also retrieve all time values
    do {                                                                   //nice little do while loop for our semaphore
        sem_wait(&mutex);                                                                       //enter critical section
        seconds = getSeconds();                                                          //update second in virtual time
        nano = getNano();                                                            //update nanosecond in virtual time
        if (hasMessage == 1) {                                           //hasMessage is set to 1 in the iteration prior
            break;                                                                //break out of loop and proceed to end
        }
        if (memString != NULL) {                                  //if memString points to not NULL, set hasMessage to 1
            hasMessage = 1;
        }
        else if (memString == NULL && (seconds + nano) >= time) {           //if memString points to NULL and time is up
            key_t key = 65;                                                              //key for holding message is 65
            float tempTime = seconds + nano;                                                     //save the current time
            char timeStr[10];                                                                         //to hold the time
            sprintf(timeStr, "%f", tempTime);                                                    //copy time into string
            memString = "BLAH";                                                          //POINT TO BLAH (just not NULL)
            int shmid = shmget(key, 2048, 0666 | IPC_CREAT);                                     //get the shared memory
            char *str = (char *) shmat(shmid, (void *) 0, 0);                               //point to the shared memory
            strcpy(str, timeStr);                                                   //copy the time to the shared memory
            shmdt(str);                                                                  //detach from the shared memory
        }
        sem_post(&mutex);                                                                      //end of critical section
    }while(1);
}


main(){
    sem_init(&mutex, 1, 1);                                                                       //initialize semaphore
    threadFunc();                                                                              //call semaphore function
    sem_destroy(&mutex);                                                                         //destroy the semaphore
    exit(0);                                                                                                      //exit
}