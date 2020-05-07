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

int finished;
int counter;
int isFull;
int isEmpty;

int requestNo;

int main(void)
{
    pthread_t lift_R;
    pthread_t lift_1;
    /*pthread_t lift_2;
    pthread_t lift_3;*/

    int *arg = malloc(sizeof(*arg));

    initialise();
    openFiles();

    if(pthread_create(&lift_R, NULL, request, NULL) == -1)
    {
        printf("Can't create Lift R\n");
    }

    *arg = 0;
    if(pthread_create(&lift_1, NULL, lift, arg) == -1)
    {
        printf("Can't create Lift R\n");
    }

    /**arg = 1;
    if(pthread_create(&lift_2, NULL, lift, arg) == -1)
    {
        printf("Can't create Lift R\n");
    }

    *arg = 2;
    if(pthread_create(&lift_3, NULL, lift, arg) == -1)
    {
        printf("Can't create Lift R\n");
    }*/

    pthread_join(lift_R,NULL);
    pthread_join(lift_1,NULL);
    /*pthread_join(lift_2,NULL);
    pthread_join(lift_3,NULL);*/

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&full);
    pthread_cond_destroy(&empty);
    closeFiles();

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

    counter = 0;
    finished = 0;
    requestNo = 0;
    isFull = 0;
    isEmpty = 0;
}

void *request(void *param)
{
    int reading[2];
    int* readPointer;

    while(finished == 0)
    {
        pthread_mutex_lock(&lock);

        while(isFull == 1)
        {
            printf("buffer is full");
            pthread_cond_wait(&full, &lock);
        }

        readPointer = readNextValue(reading);

        if(readPointer[0] == 66)
        {
            finished = 1;
        }

        else
        {
            liftRequests[counter].source = readPointer[0];
            liftRequests[counter].destination = readPointer[1];
            requestNo++;
            writeBuffer(&liftRequests[counter], requestNo);
            printf("Written buffer");
            counter++;
            isEmpty = 0;

            if(counter == 10)
            {
                isFull = 1;
            }
        }

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void *lift(void *param)
{
    int i = *((int *) param);

    while(finished == 0)
    {
        pthread_mutex_lock(&lock);

        while(isEmpty == 1)
        {
            printf("buffer is empty");
            pthread_cond_wait(&empty, &lock);
        }

        if(finished == 0)
        {
            counter--;
            isFull = 0;

            if(counter == 0)
            {
                isEmpty = 1;
            }

            /*sleep(2);*/

            liftArray[i].source = liftRequests[counter].source;
            liftArray[i].destination = liftRequests[counter].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            printf("%s before writing\n", liftArray[i].name);

            writeLift(&liftArray[i]);

            printf("%s after writing\n", liftArray[i].name);

            liftArray[i].prevRequest = liftArray[i].destination;

            pthread_cond_signal(&full);
        }

        pthread_mutex_unlock(&lock);
    }

    free(param);

    return NULL;
}
