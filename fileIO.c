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
    fprintf(fileOutput, "--------------------------------");
    fprintf(fileOutput, "New Lift Request From Floor %d to %d", buffer->source, buffer->destination);
    fprintf(fileOutput, "Request No: %d", requestNo);
    fprintf(fileOutput, "--------------------------------");
    fprintf(fileOutput, "\n");
}

void writeLift(lifts* lift)
{
    fprintf(fileOutput, "%s Operation", lift->name);
    fprintf(fileOutput, "Previous Position: Floor %d", lift->prevRequest);
    fprintf(fileOutput, "Request: Floor %d to %d", lift->source, lift->destination);
    fprintf(fileOutput, "Detail Operations");
    fprintf(fileOutput, "   Go from Floor %d to Floor %d", lift->prevRequest, lift->source);
    fprintf(fileOutput, "   Go from Floor %d to Floor %d", lift->source, lift->destination);
    fprintf(fileOutput, "   #movement for this request: %d", lift->movement);
    fprintf(fileOutput, "   #request: %d", lift->totalRequests);
    fprintf(fileOutput, "   Total #movement: %d", lift->totalMovement);
    fprintf(fileOutput, "Current position: Floor %d", lift->destination);
    fprintf(fileOutput, "\n");
}

void closeFiles()
{
    fclose(fileInput);
    fclose(fileOutput);
}
