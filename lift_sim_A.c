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
int in;
int out;
int isFull;
int isEmpty;

int requestNo;

int main(void)
{
    pthread_t lift_R;
    pthread_t lift_1;
    pthread_t lift_2;
    pthread_t lift_3;

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

    *arg = 1;
    if(pthread_create(&lift_2, NULL, lift, arg) == -1)
    {
        printf("Can't create Lift R\n");
    }

    *arg = 2;
    if(pthread_create(&lift_3, NULL, lift, arg) == -1)
    {
        printf("Can't create Lift R\n");
    }

    pthread_join(lift_R,NULL);
    pthread_join(lift_1,NULL);
    pthread_join(lift_2,NULL);
    pthread_join(lift_3,NULL);

    free(arg);

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

    in = 0;
    out = 0;
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

    while((finished == 0) || (in != out))
    {
        printf("%s about to enter lock\n", liftArray[i].name);
        pthread_mutex_lock(&lock);

        while(in == out)
        {
            printf("buffer is empty\n");
            pthread_cond_wait(&empty, &lock);
        }

        if(in != out)
        {
            /*sleep(2);*/

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

        pthread_mutex_unlock(&lock);
    }

    printf("EXITING BECAUSE FINISHED %s\n", liftArray[i].name);

    return NULL;
}
