#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "lift_sim_B.h"
#include "structs.h"
#include "fileIO.h"

sem_t *mutex;
sem_t *empty;
sem_t *full;

buffer* liftRequests;
lifts* liftArray;

int finished;
int finishLift;
int in;
int out;
int isFull;
int isEmpty;

int requestNo;

int main(void)
{
    int fd;
    int fd2;
    pid_t lift_R;
    pid_t lifts[3];
    int jj;

    fd = shm_open("/liftbuffer", O_CREAT | O_EXCL | O_RDWR, 0600);
    fd2 = shm_open("/liftarray", O_CREAT | O_EXCL | O_RDWR, 0600);

    ftruncate(fd, 10*sizeof(buffer));
    ftruncate(fd, 3*sizeof(lifts));

    *liftRequests = (buffer *)mmap(0, 10*sizeof(buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *liftArray = (lifts *)mmap(0, 3*sizeof(lifts), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

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

    munmap(liftRequests, 3*sizeof(buffer));
    close(fd);

    munmap(liftArray, 10*sizeof(lifts));
    close(fd2);

    shm_unlink("/liftbuffer");
    shm_unlink("/liftarray");

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

    strcpy(liftArray[0]->name, "Lift-1");
    strcpy(liftArray[1]->name, "Lift-2");
    strcpy(liftArray[2]->name, "Lift-3");

    for(jj = 0; jj < 3; jj++)
    {
        liftArray[jj]->prevRequest = 1;
        liftArray[jj]->totalMovement = 0;
        liftArray[jj]->totalRequests = 0;
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

            liftRequests[in]->source = readPointer[0];
            liftRequests[in]->destination = readPointer[1];
            requestNo++;
            writeBuffer(liftRequests[in], requestNo);
            in = (in+1)%bufferSize;

            sem_post(mutex);
            sem_post(full);
        }
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);

    return NULL;
}

void lift(int i)
{
    while(finished == 0)
    {
        sem_wait(full);
        sem_wait(mutex);

        if(in != out)
        {
            sleep(1);

            liftArray[i]->source = liftRequests[out]->source;
            liftArray[i]->destination = liftRequests[out]->destination;
            liftArray[i]->movement = abs(liftArray[i]->prevRequest - liftArray[i]->source) + abs(liftArray[i]->destination - liftArray[i]->source);
            liftArray[i]->totalMovement += liftArray[i]->movement;
            liftArray[i]->totalRequests++;

            writeLift(liftArray[i]);

            liftArray[i]->prevRequest = liftArray[i]->destination;

            out = (out+1)%bufferSize;
        }

        sem_post(mutex);
        sem_post(empty);
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);

    return NULL;
}
