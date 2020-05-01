#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "buffer.h"

pthread_mutex_t lock;
sem_t full, empty;
FILE *fp;

buffer liftRequests[10];

void *request(void *param)
{
    pthread_mutex_lock(&lock);

    fp = fopen("sim_input", "r+");

    fscanf(fp, "%d %d", &liftRequests[0].source, &liftRequests[0].destination);
    fscanf(fp, "%d %d", &liftRequests[1].source, &liftRequests[1].destination);

    fclose(fp);

    printf("%d %d\n",liftRequests[0].source, liftRequests[0].destination);
    printf("%d %d\n",liftRequests[1].source, liftRequests[1].destination);

    pthread_mutex_unlock(&lock);

    return NULL;
}

void *lift(void *param)
{
    pthread_mutex_lock(&lock);

    printf("%s\n",(char*)param);

    pthread_mutex_unlock(&lock);

    return NULL;
}

int main(void)
{
    pthread_t lift_R, lift_1, lift_2, lift_3;

    char strr[] = "The request";
    char str1[] = "Lift 1";
    char str2[] = "Lift 2";
    char str3[] = "Lift 3";

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
           printf("\n mutex init has failed\n");
    }

    if(pthread_create(&lift_R, NULL, request, (void*)strr) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_1, NULL, lift, (void*)str1) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_2, NULL, lift, (void*)str2) == -1)
    {
        printf("Can't create Lift R\n");
    }

    if(pthread_create(&lift_3, NULL, lift, (void*)str3) == -1)
    {
        printf("Can't create Lift R\n");
    }

    pthread_join(lift_R,NULL);
    pthread_join(lift_1,NULL);
    pthread_join(lift_2,NULL);
    pthread_join(lift_3,NULL);
    pthread_mutex_destroy(&lock);

    return 0;
}
