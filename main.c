#include "sharedMem.h"

struct myTime updateClock(struct myTime virtual){     //function for updating the system clock's seconds and nanoseconds
    virtual.nanoseconds = virtual.nanoseconds + 40000;                       //increment nanoseconds by 40000 every call
    if(virtual.nanoseconds >= 1000000000){                          //if the nanoseconds exceeds or is equal to a second
        virtual.seconds++;                                                             //increment the seconds counter
        virtual.nanoseconds = virtual.nanoseconds - 1000000000;             //decrement the nanosecond count by a second
    }
    return virtual;                                           //return the new time stored in the virtual time structure
}

static void myhandler(int s) {                                    //handler for the program to shut down at end of timer
    int errsave;
    errsave = errno;
    shmctl(shmget(66, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(67, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    shmctl(shmget(65, 1024, 0666), IPC_RMID, NULL);                                               //delete shared memory
    exit(0);                                                           //close program in its tracks after timer expires
    errno = errsave;
}
static int setupinterrupt(void) {                                                     /* set up myhandler for SIGPROF */
    struct sigaction act;
    act.sa_handler = myhandler;
    act.sa_flags = 0;
    return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}
static int setupitimer(int n) {                                             /* set ITIMER_PROF for n-second intervals */
    struct itimerval value;
    value.it_interval.tv_sec = n;                //set the timer to however many seconds user enters or default 1 second
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value, NULL));
}

int main(int argc, char *argv[]) {

    int opt = 0;
    int timeInSec = 5;                                                               //default timer is set to 5 seconds
    int maxChildren = 5;                                                       //default max child processes is set to 5
    char fileName[100] = "logFile";                                            //default log file name is set to logFile
    virtual.nanoseconds = 0;                                                            //nanosecond counter is set to 0
    virtual.seconds = 0;                                                                    //second counter is set to 0

    while ((opt = getopt(argc, argv,"hs:l:t:")) != -1) {                                                //GET OPT WOOOOO
        switch (opt) {                                          //start a switch statement for the commandline arguments
            case 'h' :                                                                              //print help message
                printf("Welcome to Log Parse!\n");
                printf("Invocation: ./logParse [-h] [-s maxChildren] [-l logFile] [-t timer]\n");
                printf("h : Display a help message to the user and exit \n");
                printf("s : Set max amount of children to spawn\n");
                printf("l : Use the given file name as the logfile\n");
                printf("t : Limit the program to n amount of seconds for execution\n");
                exit(0);
                break;
            case 's' :                                                             //set max amount of children to spawn
                maxChildren = atoi(optarg);
                break;
            case 'l' :                                                    //use the given file name as the log file name
                strcpy(fileName, "");
                strcpy(fileName, optarg);
                break;
            case 't' :                                                     //set the time entire program can execute for
                timeInSec = atoi(optarg);
                break;
            default:                                                              //user inputted something unrecognized
                perror("Command line argument not recognized.");
                exit(EXIT_FAILURE);
        }

    }

    setupinterrupt();                                                    //start the interrupt and timer for the program
    setupitimer(timeInSec);

    key_t secKey = 66;                                         //the key for the shared memory holding the seconds is 66
    key_t nanoKey = 67;                                    //the key for the shared memory holding the nanoseconds is 67

    int secID = shmget(secKey, 2048, 0666|IPC_CREAT);    //access the shared memory with file write and read permissions
    int nanoID = shmget(nanoKey, 2048, 0666|IPC_CREAT); //same as above, secID is for seconds, nanoID is for nanoseconds
    int childCount;                                      //this is for a loop below to hold the count of child processes
    pid_t childPid;                                            //of course we need one of these guys for child processes
    remove("logFile");                                //remove whatever the default log file name just in case it exists
    FILE *logptr = fopen(fileName, "w");                                               //initialize file ptr for writing
    for(childCount = 0; childCount < maxChildren; childCount++){           //increment loop as many times as maxChildren
        if(childPid != 0){                                                           //as long as its the parent process
            childPid = fork();                                                                    //fork a child process
        }
    }
    do{
        if(childCount != maxChildren-1 && childPid != 0){     //if the count is less than maxAllowed, and its the parent
            childCount++;                                                                //increment child process count
            childPid = fork();                                                                    //fork another process
        }
        if(childPid == 0){                                                           //if the process is a child process
            childCount--;                                                                        //decrement child count
            execl("./user", NULL);                                                                          //run ./user
            break;                                               //break the loop if somehow the process comes back here
        }
        else {                                                                       //otherwise it's the parent process
            virtual = updateClock(virtual);                                               //update the virtual user time
            char temp1[10], temp2[11];                                               //two temp strings for holding time
            sprintf(temp1, "%d", virtual.seconds);                                       //store the seconds in a string
            sprintf(temp2, "%d", virtual.nanoseconds);                               //store the nanoseconds in a string
            char *secStr = (char *) shmat(secID, (void *) 0, 0);         //ptr to the shared memory location for seconds
            strcpy(secStr, temp1);                                      //copy the seconds to the shared memory location
            shmdt(secStr);                                                      //detach from the shared memory location
            char *nanoStr = (char *) shmat(nanoID, (void *) 0, 0);   //ptr to the shared memory location for nanoseconds
            strcpy(nanoStr, temp2);                                 //copy the nanoseconds to the shared memory location
            shmdt(nanoStr);                                                     //detach from the shared memory location
            key_t read = 65;                                               //the key for the shared memory for messaging
            if (shmget(read, 2018, 0444) != -1) {                    //if the memory location has something stored there
                float time = (float)virtual.seconds + ((float)(virtual.nanoseconds) / 1000000000);      //store the time
                wait(NULL);                                                                      //wait for child to die
                int shmid = shmget(read, 2048, 0444);                         //get the memory location in shared memory
                char *str = (char *) shmat(shmid, (void *) 0, 0);            //we got another ptr here for that location
                char msg[100] = "Master: Child is terminating at my time ";           //below is just message formatting
                char temp[100];
                sprintf(temp, "%f", time);
                strcat(msg, temp);
                strcat(msg, " because it reached ");
                strcat(msg, str);
                strcat(msg, " in child process.\n");
                fputs(msg, logptr);;                                                 //store the message in the log file
                shmdt(str);                                                              //detach from the shared memory
                shmctl(shmid, IPC_RMID, NULL);                                                //delete the shared memory
            }
        }
    } while (virtual.seconds < 2 || childCount <= 100);   //this do while loop ends if seconds < 2 or 100 children exist
    fclose(logptr);                                                                                 //close the log file
    shmctl(secID, IPC_RMID, NULL);                                                //delete the shared memory for seconds
    shmctl(nanoID, IPC_RMID, NULL);                                           //delete the shared memory for nanoseconds
    exit(0);                                                                    // end the process, cause its the parent

}