#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "lift_sim_B.h"
#include "structs.h"
#include "fileIO.h"

sem_t *mutex;
sem_t *empty;
sem_t *full;

buffer liftRequests[10];
lifts liftArray[3];

int finished;
int finishLift;
int in;
int out;
int isFull;
int isEmpty;

int requestNo;

int main(void)
{
    pid_t lift_R;
    pid_t lifts[3];
    int jj;

    initialise();
    openFiles();

    lift_R = fork();

    if(lift_R == 0)
    {
        request();
        exit(0);
    }

    for(jj = 0; jj < 3; jj++)
    {
        lifts[jj] = fork();

        if(lifts[jj] == 0)
        {
            lift(ii);
            exit(0);
        }
    }

    waitpid(lift_R, NULL, 0);

    for(jj = 0; jj<3; jj++)
    {
        waitpid(lifts[jj], NULL, 0);
    }


    writeResult((liftArray[0].totalMovement+liftArray[1].totalMovement+liftArray[2].totalMovement), requestNo);
    closeFiles();

    sem_unlink(mutex);
    sem_unlink(full);
    sem_unlink(empty);

    return 0;
}

void initialise()
{
    int jj;

    mutex = sem_open("/mutex", O_CREAT|O_EXCL, 0644, 1);
    full = sem_open("/full", O_CREAT|O_EXCL, 0644, 0);
    empty = sem_open("/empty", O_CREAT|O_EXCL, 0644, 10);

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

void request()
{
    int reading[2];
    int* readPointer;

    while(finished == 0)
    {
        readPointer = readNextValue(reading);

        if(readPointer[0] == 66)
        {
            finished = 1;
            printf("END OF FILE\n");
        }

        else
        {
            sem_wait(empty);
            sem_wait(mutex);

            liftRequests[in].source = readPointer[0];
            liftRequests[in].destination = readPointer[1];
            writeBuffer(&liftRequests[in], requestNo);

            sem_post(mutex);
            sem_post(full);
        }
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);

    return NULL;
}

void lift()
{
    while(finished == 0)
    {
        sem_wait(full);
        sem_wait(mutex);

        if(in != out)
        {
            sleep(1);

            liftArray[i].source = liftRequests[out].source;
            liftArray[i].destination = liftRequests[out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            writeLift(&liftArray[i]);

            liftArray[i].prevRequest = liftArray[i].destination;
        }

        sem_post(mutex);
        sem_post(empty);
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);

    return NULL;
}
