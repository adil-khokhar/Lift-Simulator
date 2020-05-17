/* File: fileIO.c
 * Author: Adil Khokhar (19182405)
 * Purpose: Responible for all File Input/Output for Lift Simulator
 * Date Modified: 16/05/2020
 */

#include <stdio.h>
#include <stdlib.h>

#include "structs.h"
#include "fileIO.h"

FILE *fileInput;
FILE *fileOutput;

/*
 * Function: openFiles
 * Purpose: Open input and output files for lift simulator
 */
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

    fclose(fileOutput);
}

/*
 * Function: readNextValue
 * Purpose: Read next set of values from input file. If file has been fully
 * read, return the number 66 (not a normally accepted parameter) to
 * indicate file is finished
 */
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

/*
 * Function: writeBuffer
 * Purpose: Print source, destination and the current request number to output
 * file when new item gets added to lift buffer
 */
void writeBuffer(int source, int destination, int requestNo)
{
    fileOutput = fopen("sim_output", "a");
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "New Lift Request From Floor %d to %d\n", source, destination);
    fprintf(fileOutput, "Request No: %d\n", requestNo);
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "\n");
    fclose(fileOutput);
}

/*
 * Function: writeLift
 * Purpose: Write the lift movement summary when lift serves request from buffer
 * (accepts lifts struct pointer and uses structs.h)
 */
void writeLift(lifts* lift)
{
    fileOutput = fopen("sim_output", "a");
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
    fclose(fileOutput);
}

/*
 * Function: writeResult
 * Purpose: Write total movement and total requests at end of output file
 * when lift simulator has concluded
 */
void writeResult(int movement, int requests)
{
    fileOutput = fopen("sim_output", "a");
    fprintf(fileOutput, "--------------------------------\n");
    fprintf(fileOutput, "Total Number of Requests: %d\n", requests);
    fprintf(fileOutput, "Total Number of Movements: %d\n", movement);
    fprintf(fileOutput, "--------------------------------\n");
    fclose(fileOutput);
}

/*
 * Function: closeFiles
 * Purpose: Close input file once simulator has finished
 */
void closeFiles()
{
    fclose(fileInput);
}
