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

            initialise();
            openFiles();

            lift_R = fork();

            if(lift_R == 0)
            {
                printf("Lift-R Process Created and Running ...\n");
                request(bufferSize);
                exit(0);
            }

            for(jj = 0; jj < 3; jj++)
            {
                lift_processes[jj] = fork();

                if(lift_processes[jj] == 0)
                {
                    printf("%s Process Created and Running ...\n", *liftArray[jj].name);
                    lift(jj, bufferSize, sleepTime);
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

    *in = 0;
    *out = 0;
}

void request(int bufferSize)
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

            liftArray[0].finished = 1;
            liftArray[1].finished = 1;
            liftArray[2].finished = 1;

            sem_post(full);
            sem_post(full);
            sem_post(full);
        }

        else
        {
            sem_wait(empty);
            sem_wait(mutex);

            liftBuffer[*in].source = readPointer[0];
            liftBuffer[*in].destination = readPointer[1];
            requestNo++;
            *in = (*in+1)%bufferSize;

            sem_post(mutex);
            sem_post(full);

            sem_wait(fileOut);
            writeBuffer(readPointer[0], readPointer[1], requestNo);
            sem_post(fileOut);
        }
    }

    printf("Lift-R Process Finished ...\n");

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);
}

void lift(int i, int bufferSize, int sleepTime)
{
    int finishLift;

    finishLift = 0;

    while(finishLift == 0)
    {
        sem_wait(full);
        sem_wait(mutex);

        if((liftArray[i].finished == 0) || (*in != *out))
        {

            liftArray[i].source = liftBuffer[*out].source;
            liftArray[i].destination = liftBuffer[*out].destination;
            liftArray[i].movement = abs(liftArray[i].prevRequest - liftArray[i].source) + abs(liftArray[i].destination - liftArray[i].source);
            liftArray[i].totalMovement += liftArray[i].movement;
            liftArray[i].totalRequests++;

            sem_wait(fileOut);
            writeLift(&liftArray[i]);
            sem_post(fileOut);

            liftArray[i].prevRequest = liftArray[i].destination;

            *out = (*out+1)%bufferSize;
        }

        else
        {
            finishLift = 1;
        }

        sem_post(mutex);
        sem_post(empty);

        sleep(sleepTime);
    }

    printf("%s Process Finished ...\n", *liftArray[jj].name);

    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_close(fileOut);
}
