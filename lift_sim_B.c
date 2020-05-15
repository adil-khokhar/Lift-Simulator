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

int ftruncate(int f, off_t length);

sem_t *mutex;
sem_t *empty;
sem_t *full;

sem_t *fileOut;

buffer* liftBuffer;
lifts* liftArray;
int* in;
int* out;

int requestNo;

int main(void)
{
    int bufferFd;
    int arrayFd;
    int inFd;
    int outFd;
    pid_t lift_R;
    pid_t lift_processes[3];
    int jj;

    bufferFd = shm_open("/liftbuffer", O_CREAT | O_RDWR, 0666);
    ftruncate(bufferFd, 10*sizeof(buffer));
    liftBuffer = (buffer*)mmap(0, 10*sizeof(buffer), PROT_READ | PROT_WRITE, MAP_SHARED, bufferFd, 0);

    arrayFd = shm_open("/liftarray", O_CREAT | O_RDWR, 0666);
    ftruncate(arrayFd, 3*sizeof(lifts));
    liftArray = (lifts*)mmap(0, 3*sizeof(lifts), PROT_READ | PROT_WRITE, MAP_SHARED, arrayFd, 0);

    inFd = shm_open("/in", O_CREAT | O_RDWR, 0666);
    ftruncate(inFd, sizeof(int));
    in = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, inFd, 0);

    outFd = shm_open("/out", O_CREAT | O_RDWR, 0666);
    ftruncate(outFd, sizeof(int));
    out = (int*)mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, outFd, 0);

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
        lift_processes[jj] = fork();

        if(lift_processes[jj] == 0)
        {
            lift(jj);
            exit(0);
        }
    }

    waitpid(lift_R, NULL, 0);

    for(jj = 0; jj<3; jj++)
    {
        waitpid(lift_processes[jj], NULL, 0);
    }


    writeResult((liftArray[0].totalMovement+liftArray[1].totalMovement+liftArray[2].totalMovement), (liftArray[0].totalRequests+liftArray[1].totalRequests+liftArray[2].totalRequests));
    closeFiles();

    munmap(liftBuffer, 3*sizeof(buffer));
    close(bufferFd);

    munmap(liftArray, 10*sizeof(lifts));
    close(arrayFd);

    munmap(in, sizeof(int));
    close(inFd);

    munmap(out, sizeof(int));
    close(outFd);

    shm_unlink("/liftbuffer");
    shm_unlink("/liftarray");
    shm_unlink("/int");
    shm_unlink("/out");

    return 0;
}

void initialise()
{
    int jj;

    mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0, 1);
    full = sem_open("/full", O_CREAT | O_EXCL, 0, 0);
    empty = sem_open("/empty", O_CREAT | O_EXCL, 0, 10);
    fileOut = sem_open("/fileOut", O_CREAT | O_EXCL, 0, 1);

    sem_unlink("/mutex");
    sem_unlink("/full");
    sem_unlink("/empty");
    sem_unlink("/fileOut");

    strcpy(liftArray[0].name, "Lift-1");
    strcpy(liftArray[1].name, "Lift-2");
    strcpy(liftArray[2].name, "Lift-3");

    for(jj = 0; jj < 3; jj++)
    {
        liftArray[jj].prevRequest = 1;
        liftArray[jj].totalMovement = 0;
        liftArray[jj].totalRequests = 0;
        liftArray[jj].finished = 0;
    }
}

void request()
{
    int reading[2];
    int* readPointer;
    int finished;
    int requestNo;

    finished = 0;
    requestNo = 0;

    while(finished == 0)
    {
        readPointer = readNextValue(reading);

        if(readPointer[0] == 66)
        {
            finished = 1;
            printf("END OF FILE\n");

            liftArray[0].finished = 1;
            liftArray[1].finished = 1;
            liftArray[2].finished = 1;
        }

        else
        {
            sem_wait(empty);
            sem_wait(mutex);
            printf("REQUEST MUTEX LOCKED\n");

            liftBuffer[*in].source = readPointer[0];
            liftBuffer[*in].destination = readPointer[1];
            requestNo++;
            *in = (*in+1)%10;

            printf("REQUEST MUTEX UNLOCKED\n");
            sem_post(mutex);
            sem_post(full);

            sem_wait(fileOut);
            printf("FILE MUTEX LOCKED\n");
            writeBuffer(&liftBuffer[*in], requestNo);
            printf("FILE MUTEX UNLOCKED\n");
            sem_post(fileOut);
        }
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);
}

void lift(int i)
{
    while(liftArray[i].finished == 0)
    {
        sem_wait(full);
        sem_wait(mutex);
        printf("LIFT MUTEX LOCKED\n");

        if(liftArray[i].finished == 0)
        {
            sleep(1);

            liftArray[i].source = liftBuffer[*out].source;
            liftArray[i].destination = liftBuffer[*out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            sem_wait(fileOut);
            printf("FILE MUTEX LOCKED\n");
            writeLift(&liftArray[i]);
            printf("FILE MUTEX UNLOCKED\n");
            sem_post(fileOut);

            liftArray[i].prevRequest = liftArray[i].destination;

            *out = (*out+1)%10;
        }

        printf("LIFT MUTEX UNLOCKED\n");
        sem_post(mutex);
        sem_post(empty);
    }

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);

    printf("%s ending\n", liftArray[i].name);
}
