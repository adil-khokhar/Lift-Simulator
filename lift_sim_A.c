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

    int *arg1 = (int*)malloc(sizeof(int));
    int *arg2 = (int*)malloc(sizeof(int));
    int *arg3 = (int*)malloc(sizeof(int));

    if( argc == 3 )
    {
        if(*argv[1] < 1)
        {
            printf("Error! Buffer Size has to be greater than or equal to 1!");
        }

        else if(*argv[2] < 0)
        {
            printf("Error! Time has to be greater than or equal to 0!");
        }

        else
        {
            bufferSize = &argv[1];
            sleepTime = &argv[2];

            initialise();
            openFiles();

            if(pthread_create(&lift_R, NULL, request, NULL) == -1)
            {
                printf("Can't create Lift R\n");
            }

            *arg1 = 0;
            if(pthread_create(&lift_1, NULL, lift, arg1) == -1)
            {
                printf("Can't create Lift 1\n");
            }

            *arg2 = 1;
            if(pthread_create(&lift_2, NULL, lift, arg2) == -1)
            {
                printf("Can't create Lift 2\n");
            }

            *arg3 = 2;
            if(pthread_create(&lift_3, NULL, lift, arg3) == -1)
            {
                printf("Can't create Lift 3\n");
            }

            pthread_join(lift_R,NULL);
            pthread_join(lift_1,NULL);
            pthread_join(lift_2,NULL);
            pthread_join(lift_3,NULL);

            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&full);
            pthread_cond_destroy(&empty);
            writeResult((liftArray[0].totalMovement+liftArray[1].totalMovement+liftArray[2].totalMovement), requestNo);
            closeFiles();
        }
    }

    else
    {
        printf("Error! Incorrect Number of Arguments!");
    }

    free(arg1);
    free(arg2);
    free(arg3);

    return 0;
}

void initialise()
{
    int jj;

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

    strcpy(liftArray[0].name, "Lift-1");
    strcpy(liftArray[1].name, "Lift-2");
    strcpy(liftArray[2].name, "Lift-3");

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

void *request(void *param)
{
    int reading[2];
    int* readPointer;

    while(finished == 0)
    {
        pthread_mutex_lock(&lock);

        while(((in+1)%10) == out)
        {
            printf("buffer is full\n");
            pthread_cond_wait(&full, &lock);
        }

        readPointer = readNextValue(reading);

        if(readPointer[0] == 66)
        {
            finished = 1;
            printf("END OF FILE\n");
        }

        else
        {
            liftRequests[in].source = readPointer[0];
            liftRequests[in].destination = readPointer[1];
            requestNo++;
            writeBuffer(&liftRequests[in], requestNo);
            printf("Written buffer\n");
            in = (in+1)%10;
        }

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void *lift(void *param)
{
    int i = *((int *) param);

    while(finishLift == 0)
    {
        printf("%s about to enter lock\n", liftArray[i].name);
        pthread_mutex_lock(&lock);

        while((in == out) && (finished != 1))
        {
            printf("buffer is empty\n");
            pthread_cond_wait(&empty, &lock);
        }

        if(in != out)
        {
            sleep(1);

            liftArray[i].source = liftRequests[out].source;
            liftArray[i].destination = liftRequests[out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            printf("%s before writing\n", liftArray[i].name);

            writeLift(&liftArray[i]);

            printf("%s after writing\n", liftArray[i].name);

            liftArray[i].prevRequest = liftArray[i].destination;

            out = (out+1)%10;

            pthread_cond_signal(&full);
        }

        if((in == out) && (finished == 1))
        {
            finishLift = 1;
        }

        pthread_mutex_unlock(&lock);
    }

    printf("EXITING BECAUSE FINISHED %s\n", liftArray[i].name);

    return NULL;
}
