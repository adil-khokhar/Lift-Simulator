#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "fileIO.h"

FILE *fileInput;
FILE *fileOutput;

void openFiles()
{
    fileInput = fopen("sim_input", "r+");

    if (fileInput == NULL)
    {
        printf("Could not open file\n");
    }

    fileOutput = fopen("sim_output", "w+");

    if (fileInput == NULL)
    {
        printf("Could not open file\n");
    }
}

int* readNextValue(int* reading)
{
    if(fscanf(fileInput, "%d %d", &reading[0], &reading[1]) != EOF)
    {
    }

    else
    {
        reading[0] = 66;
    }

    return reading;
}

void writeBuffer(buffer* buffer, int requestNo)
{
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "New Lift Request From Floor %d to %d\n", buffer->source, buffer->destination);
    fprintf(fileOutput, "Request No: %d\n", requestNo);
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "\n");
}

void writeLift(lifts* lift)
{
    fprintf(fileOutput, "%s Operation\n", lift->name);
    fprintf(fileOutput, "Previous Position: Floor %d\n", lift->prevRequest);
    fprintf(fileOutput, "Request: Floor %d to %d\n", lift->source, lift->destination);
    fprintf(fileOutput, "Detail Operations\n");
    fprintf(fileOutput, "   Go from Floor %d to Floor %d\n", lift->prevRequest, lift->source);
    fprintf(fileOutput, "   Go from Floor %d to Floor %d\n", lift->source, lift->destination);
    fprintf(fileOutput, "   #movement for this request: %d\n", lift->movement);
    fprintf(fileOutput, "   #request: %d\n", lift->totalRequests);
    fprintf(fileOutput, "   Total #movement: %d\n", lift->totalMovement);
    fprintf(fileOutput, "Current position: Floor %d\n", lift->destination);
    fprintf(fileOutput, "\n");
}

void writeResult(int movement, int requests)
{
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "Total Number of Requests: %d\n", requests);
    fprintf(fileOutput, "Total Number of Movements: %d\n", movement);
    fprintf(fileOutput, "--------------------------------\n");
}

void closeFiles()
{
    fclose(fileInput);
    fclose(fileOutput);
}
