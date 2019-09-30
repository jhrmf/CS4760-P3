#include "sharedMem.h"

struct myTime updateClock(struct myTime virtual){
    virtual.nanoseconds = virtual.nanoseconds + 40000;
    if(virtual.nanoseconds == 1000000000){
        virtual.seconds++;
        virtual.nanoseconds = 0;
    }
    return virtual;
}

static void myhandler(int s) {
    int errsave;
    errsave = errno;
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
    int timeInSec = 2;
    int maxChildren = 5;
    int x = 0;
    char fileName[100];
    virtual.nanoseconds = 0;
    virtual.seconds = 0;

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
            case 's' :
                maxChildren = atoi(optarg);
                break;
            case 'l' :
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

    key_t secKey = ftok("testing2", 66);
    key_t nanoKey = ftok("testing3", 67);

    int secID = shmget(secKey, 1024, 0666|IPC_CREAT); //this
    int nanoID = shmget(nanoKey, 1024, 0666|IPC_CREAT);


    do{
        virtual = updateClock(virtual);
        char temp1[] = "", temp2[] = "";
        sprintf(temp1, "%d", virtual.seconds);
        sprintf(temp2, "%d", virtual.nanoseconds);
        char *secStr = (char*) shmat(secID, (void*)0, 0);
        char *nanoStr = (char*) shmat(nanoID, (void*)0, 0);
        secStr = temp1;
        nanoStr = temp2;
        shmdt(secStr);
        shmdt(nanoStr);
        execv("./shmMsg", NULL);
    }while(virtual.seconds != 2);

}