#include <stdio.h>
#include <stdlib.h>

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
    if(fscanf(fp, "%d %d", &reading[0], &reading[1]) != EOF)
    {

    }

    return reading;
}

void closeFiles()
{
    fclose(fileInput);
    fclose(fileOutput);
}
