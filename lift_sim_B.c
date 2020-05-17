/* File: lift_sim_B.c
 * Author: Adil Khokhar (19182405)
 * Purpose: Simulate lift requests using Processes, POSIX Semaphores and POSIX Shared Memory
 * Date Modified: 16/05/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "lift_sim_B.h"
#include "structs.h"
#include "fileIO.h"

/* Have to manually define ftruncate function otherwise doesn't work. */
int ftruncate(int f, off_t length);

sem_t *mutex;
sem_t *empty;
sem_t *full;

sem_t *fileOut;

buffer* liftBuffer;
lifts* liftArray;
int* in;
int* out;

int main(int argc, char *argv[])
{
    int bufferFd;
    int arrayFd;
    int inFd;
    int outFd;
    int jj;
    int bufferSize;
    int sleepTime;
    pid_t lift_R;
    pid_t lift_processes[3];

    if(argc == 3)
    {
        if(atoi(argv[1]) < 1)
        {
            printf("Error! Buffer Size has to be greater than or equal to 1!\n");
        }

        else if(atoi(argv[2]) < 0)
        {
            printf("Error! Time has to be greater than or equal to 0!\n");
        }

        else
        {
            bufferSize = atoi(argv[1]) + 1;
            sleepTime = atoi(argv[2]);

            /*
            * Create Shared Memory for Buffer, Lift Array,
            * and In and Out (Keep Track of Buffer Element)
            */
            bufferFd = shm_open("/liftbuffer", O_CREAT | O_RDWR, 0666);
            ftruncate(bufferFd, bufferSize*sizeof(buffer));
            liftBuffer = (buffer*)mmap(0, bufferSize*sizeof(buffer), PROT_READ | PROT_WRITE, MAP_SHARED, bufferFd, 0);

            arrayFd = shm_open("/liftarray", O_CREAT | O_RDWR, 0666);
            ftruncate(arrayFd, 3*sizeof(lifts));
            liftArray = (lifts*)mmap(0, 3*sizeof(lifts), PROT_READ | PROT_WRITE, MAP_SHARED, arrayFd, 0);

            inFd = shm_open("/in", O_CREAT | O_RDWR, 0666);
            ftruncate(inFd, sizeof(int));
            in = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inFd, 0);

            outFd = shm_open("/out", O_CREAT | O_RDWR, 0666);
            ftruncate(outFd, sizeof(int));
            out = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, outFd, 0);

            /* Run function initalise (lift_sim_B.c) */
            initialise();
            /* Run function openFiles (fileIO.c) */
            openFiles();


            /* Create Buffer Producer Child Process */
            lift_R = fork();

            if(lift_R == 0)
            {
                printf("Lift-R Process Created and Running ...\n");
                request(bufferSize);
                exit(0);
            }

            /* Create Consumer Child Processes */
            for(jj = 0; jj < 3; jj++)
            {
                lift_processes[jj] = fork();

                if(lift_processes[jj] == 0)
                {
                    printf("%s Process Created and Running ...\n", liftArray[jj].name);
                    lift(jj, bufferSize, sleepTime);
                    exit(0);
                }
            }

            /* Wait for all child processes to finish */
            waitpid(lift_R, NULL, 0);

            for(jj = 0; jj<3; jj++)
            {
                waitpid(lift_processes[jj], NULL, 0);
            }

            /* Run function writeResult (fileIO.c) */
            writeResult((liftArray[0].totalMovement+liftArray[1].totalMovement+liftArray[2].totalMovement), (liftArray[0].totalRequests+liftArray[1].totalRequests+liftArray[2].totalRequests));
            /* Run function closeFiles (fileIO.c) */
            closeFiles();

            /* Unmap and Close Shared Memory */
            munmap(liftBuffer, 3*sizeof(buffer));
            close(bufferFd);

            munmap(liftArray, bufferSize*sizeof(lifts));
            close(arrayFd);

            munmap(in, sizeof(int));
            close(inFd);

            munmap(out, sizeof(int));
            close(outFd);

            shm_unlink("/liftbuffer");
            shm_unlink("/liftarray");
            shm_unlink("/int");
            shm_unlink("/out");

            /* Close Shared Semaphores */
            sem_close(mutex);
            sem_close(full);
            sem_close(empty);
            sem_close(fileOut);
        }
    }

    else
    {
        printf("Error! Incorrect Number of Arguments!\n");
    }

    return 0;
}

/*
 * Function: initalise
 * Purpose: Initalise some specific variables for beginning of lift simulator
 */
void initialise()
{
    int jj;

    /* Create Shared Named Semaphores */
    mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0, 1);
    full = sem_open("/full", O_CREAT | O_EXCL, 0, 0);
    empty = sem_open("/empty", O_CREAT | O_EXCL, 0, 10);
    fileOut = sem_open("/fileOut", O_CREAT | O_EXCL, 0, 1);

    /* Unlink Shared Named Semaphores */
    sem_unlink("/mutex");
    sem_unlink("/full");
    sem_unlink("/empty");
    sem_unlink("/fileOut");

    /* Add Lift Names to the Structs */
    strcpy(liftArray[0].name, "Lift-1");
    strcpy(liftArray[1].name, "Lift-2");
    strcpy(liftArray[2].name, "Lift-3");

    /* Set initial lift values */
    for(jj = 0; jj < 3; jj++)
    {
        liftArray[jj].prevRequest = 1;
        liftArray[jj].totalMovement = 0;
        liftArray[jj].totalRequests = 0;
        liftArray[jj].finished = 0;
    }

    /* Set initial buffer position */
    *in = 0;
    *out = 0;
}

/*
 * Function: request
 * Purpose: Producer which adds lift requests to a shared buffer
 */
void request(int bufferSize)
{
    int reading[2];
    int* readPointer;
    int finished;
    int requestNo;

    /* finished is used to indicate when producer should finish once no more
       requests are available */
    finished = 0;

    /* Keeps track of number of requests added to buffer */
    requestNo = 0;

    while(finished == 0)
    {
        /* Obtain next request from input file using readNextValue (fileIO.c) */
        readPointer = readNextValue(reading);

        /* When requests are exhausted, readNextValue returns a value of 66,
           which ordinarily is not a possible request */
        if(readPointer[0] == 66)
        {
            /* Breaks while loop and ends producer */
            finished = 1;

            /* Tells each consumer that no more requests are available */
            liftArray[0].finished = 1;
            liftArray[1].finished = 1;
            liftArray[2].finished = 1;

            /* Used as a redundancy. Sometimes, once requests are finished,
               the consumer processes may still be waiting for a signal from the
               producer for more requests. Manually signalling them to continue
               will avoid this almost-deadlock (Need 3 for each consumer) */
            sem_post(full);
            sem_post(full);
            sem_post(full);
        }

        else
        {
            /* Lock mutex and stop if buffer is currently full */
            sem_wait(empty);
            sem_wait(mutex);

            /* Add new request to buffer and change *in which points to current
               request index (for FIFO queue) */
            liftBuffer[*in].source = readPointer[0];
            liftBuffer[*in].destination = readPointer[1];
            requestNo++;
            *in = (*in+1)%bufferSize;

            /* Unlock mutex and signal waiting consumers */
            sem_post(mutex);
            sem_post(full);

            /* Lock file mutex and use writeBuffer (fileIO.c) to write new buffer
               to output file */
            sem_wait(fileOut);
            writeBuffer(readPointer[0], readPointer[1], requestNo);
            sem_post(fileOut);
        }
    }

    printf("Lift-R Process Finished ...\n");

    /* Close all semaphores */
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);
}

/*
 * Function: lift
 * Purpose: Consumer which serves lift request from shared buffer
 */
void lift(int i, int bufferSize, int sleepTime)
{
    int finishLift;

    /* Variable to indicate when consumer should finish */
    finishLift = 0;

    while(finishLift == 0)
    {
        /* Lock mutex and wait if buffer is empty */
        sem_wait(full);
        sem_wait(mutex);

        /* Check if requests from input file have finished and if buffer is
           currently empty (Producer may have signalled that buffer is not empty
           to avoid infinite loop even though buffer is empty */
        if((liftArray[i].finished == 0) || (*in != *out))
        {
            /* Remove request from buffer and server the request */
            liftArray[i].source = liftBuffer[*out].source;
            liftArray[i].destination = liftBuffer[*out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            /* Lock file mutex and write lift request to output using writeLift (fileIO.c) */
            sem_wait(fileOut);
            writeLift(&liftArray[i]);
            sem_post(fileOut);

            /* Make lifts previous request the destination that was just served */
            liftArray[i].prevRequest = liftArray[i].destination;

            /* Change current buffer index */
            *out = (*out+1)%bufferSize;
        }

        else
        {
            /* Change finishLift to indicate that consumer should end loop */
            finishLift = 1;
        }

        /* Unlock mutex and signal waiting producer */
        sem_post(mutex);
        sem_post(empty);

        /* Allow user define time to pass for lift request */
        sleep(sleepTime);
    }

    printf("%s Process Finished ...\n", liftArray[i].name);

    /* Close all semaphores */
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);
}
