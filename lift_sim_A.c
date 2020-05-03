#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "buffer.h"
#include "fileIO.h"

pthread_mutex_t lock;
pthread_cond_t full;
pthread_cond_t empty;

buffer liftRequests[10];

int main(void)
{
    initalise();

    pthread_t lift_R, lift_1, lift_2, lift_3;

    char strr[] = "The request";
    char str1[] = "Lift 1";
    char str2[] = "Lift 2";
    char str3[] = "Lift 3";

    if(pthread_create(&lift_R, NULL, request, NULL) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_1, NULL, lift, NULL) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_2, NULL, lift, NULL) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_3, NULL, lift, NULL) == -1)
    {
        printf("Can't create Lift R\n");
    }

    pthread_join(lift_R,NULL);
    pthread_join(lift_1,NULL);
    pthread_join(lift_2,NULL);
    pthread_join(lift_3,NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&full);
    pthread_cond_destroy(&empty);

    return 0;
}

void initialise()
{
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
}

void *request(void *param)
{
    int finished = 0;
    int reading[2];

    while(finished == 0)
    {
        pthread_mutex_lock(&lock);

        while(isFull == 1)
        {
            pthread_cond_wait(&full, &lock);
        }

        int* readPointer = readNextValue(reading);


        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

void *lift(void *param)
{
    pthread_mutex_lock(&lock);

    pthread_mutex_unlock(&lock);

    return NULL;
}
