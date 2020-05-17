/* File: lift_sim_A.c
 * Author: Adil Khokhar (19182405)
 * Purpose: Simulate lift requests using Posix Threads
 * Date Modified: 16/05/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "lift_sim_A.h"
#include "structs.h"
#include "fileIO.h"

pthread_mutex_t lock;
pthread_cond_t full;
pthread_cond_t empty;

buffer liftRequests[10];
lifts liftArray[3];

int bufferSize;
int sleepTime;
int finished;
int finishLift;
int in;
int out;
int isFull;
int isEmpty;

int requestNo;

int main(int argc, char *argv[])
{
    pthread_t lift_R;
    pthread_t lift_1;
    pthread_t lift_2;
    pthread_t lift_3;

    /* Allocate some memory to int pointers to pass to threads */
    int *arg1 = (int*)malloc(sizeof(int));
    int *arg2 = (int*)malloc(sizeof(int));
    int *arg3 = (int*)malloc(sizeof(int));

    if( argc == 3 )
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

            /* Run function initalise (lift_sim_A.c) */
            initialise();
            /* Run function openFiles (fileIO.c) */
            openFiles();

            /* Create POSIX threads and pass specific indexes (tells lift function)
               which lift is running */
            if(pthread_create(&lift_R, NULL, request, NULL) == -1)
            {
                printf("Can't create Lift R\n");
            }

            else
            {
                printf("Lift-R Thread Created and Running ...\n");
            }

            *arg1 = 0;
            if(pthread_create(&lift_1, NULL, lift, arg1) == -1)
            {
                printf("Can't create Lift 1\n");
            }

            else
            {
                printf("Lift-1 Thread Created and Running ...\n");
            }

            *arg2 = 1;
            if(pthread_create(&lift_2, NULL, lift, arg2) == -1)
            {
                printf("Can't create Lift 2\n");
            }

            else
            {
                printf("Lift-2 Thread Created and Running ...\n");
            }

            *arg3 = 2;
            if(pthread_create(&lift_3, NULL, lift, arg3) == -1)
            {
                printf("Can't create Lift 3\n");
            }

            else
            {
                printf("Lift-3 Thread Created and Running ...\n");
            }

            /* Wait for the threads to terminate */
            pthread_join(lift_R,NULL);
            pthread_join(lift_1,NULL);
            pthread_join(lift_2,NULL);
            pthread_join(lift_3,NULL);

            /* Destroy mutexes and condition variables */
            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&full);
            pthread_cond_destroy(&empty);

            /* Run function writeResult (fileIO.c) */
            writeResult((liftArray[0].totalMovement+liftArray[1].totalMovement+liftArray[2].totalMovement), requestNo);
            /* Run function closeFiles (fileIO.c) */
            closeFiles();
        }
    }

    else
    {
        printf("Error! Incorrect Number of Arguments!\n");
    }

    /* Free dynamically allocated memory */
    free(arg1);
    free(arg2);
    free(arg3);

    return 0;
}

/*
 * Function: initalise
 * Purpose: Initalise some specific variables for beginning of lift simulator
 */
void initialise()
{
    int jj;

    /* Initialise mutex lock and condition variables */
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
    }

    if (pthread_cond_init(&full, NULL) != 0)
    {
        printf("\n full init has failed\n");
    }

    if (pthread_cond_init(&empty, NULL) != 0)
    {
        printf("\n empty init has failed\n");
    }

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
    }

    in = 0;
    out = 0;
    finished = 0;
    requestNo = 0;
    isFull = 0;
    isEmpty = 0;
    finishLift = 0;
}

/*
 * Function: request
 * Purpose: Producer which adds lift requests to buffer
 */
void *request(void *param)
{
    int reading[2];
    int* readPointer;

    while(finished == 0)
    {
        /* Lock Mutex */
        pthread_mutex_lock(&lock);

        /* Check if mutex is currently full and wait if it is */
        while(((in+1)%bufferSize) == out)
        {
            pthread_cond_wait(&full, &lock);
        }

        /* Obtain next request from input file using readNextValue (fileIO.c) */
        readPointer = readNextValue(reading);

        /* When requests are exhausted, readNextValue returns a value of 66,
           which ordinarily is not a possible request */
        if(readPointer[0] == 66)
        {
            finished = 1;
        }

        else
        {
            /* Add new request to buffer and change 'in' which points to current
               request index (for FIFO queue) */
            liftRequests[in].source = readPointer[0];
            liftRequests[in].destination = readPointer[1];
            requestNo++;
            /* Use writeBuffer (fileIO.c) to write to output file */
            writeBuffer(readPointer[0],readPointer[1], requestNo);
            in = (in+1)%bufferSize;
        }

        /* Signal that buffer is not empty and unlock mutex */
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);
    }

    printf("Lift-R Thread Finished ...\n");

    return NULL;
}

/*
 * Function: lift
 * Purpose: Consumer which serves lift request from buffer
 */
void *lift(void *param)
{
    /* i is used to obtain specific lift from lifts array */
    int i = *((int *) param);

    while(finishLift == 0)
    {
        /* Lock Mutex */
        pthread_mutex_lock(&lock);

        /* Check if buffer is empty and requests are still available, if so, wait */
        while((in == out) && (finished != 1))
        {
            pthread_cond_wait(&empty, &lock);
        }

        /* Run if buffer is not empty */
        if(in != out)
        {
            /* Serve request from buffer */
            liftArray[i].source = liftRequests[out].source;
            liftArray[i].destination = liftRequests[out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            /* Use writeLift (fileIO.c) to write lift request to output file */
            writeLift(&liftArray[i]);

            /* Make lifts previous request the destination that was just served */
            liftArray[i].prevRequest = liftArray[i].destination;

            /* Change buffer index */
            out = (out+1)%bufferSize;

            /* Signal that buffer is no longer full */
            pthread_cond_signal(&full);
        }

        /* Run if buffer is empty and if no more requests are available. This will
           break the while loop */
        if((in == out) && (finished == 1))
        {
            finishLift = 1;
        }

        /* Unlock the mutex */
        pthread_mutex_unlock(&lock);

        /* Allow user define time to pass for lift request */
        sleep(sleepTime);
    }

    printf("%s Thread Finished ...\n", liftArray[i].name);

    return NULL;
}
