/* File: gen_sim_input.c
 * Author: Adil Khokhar (19182405)
 * Purpose: Generate random input file for lift simulator
 * Date Modified: 16/05/2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    FILE *fp;
    int randomNum1, randomNum2;
    int numRequests;
    int ii;

    /* Open input file to write */
    fp = fopen("sim_input", "w+");

    if (fp == NULL)
    {
        printf("Could not open file\n");
    }

    else
    {
        /* Set starting point for random integers */
        srand(time(0));

        /* Generates random number from 50 to 100 (Number of Requests) */
        numRequests = (rand() % (100 - 50 + 1)) + 50;

        printf("Num requests = %d\n", numRequests);

        /* Picks two random numbers between 1 and 20 and prints on new lines each time */
        for(ii = 0; ii < numRequests; ii++)
        {
            randomNum1 = (rand() % (20 - 1 + 1)) + 1;
            randomNum2 = (rand() % (20 - 1 + 1)) + 1;

            while(randomNum1 == randomNum2)
            {
              randomNum1 = (rand() % (20 - 1 + 1)) + 1;
              randomNum2 = (rand() % (20 - 1 + 1)) + 1;
            }

            fprintf(fp,"%d %d\n",randomNum1, randomNum2);
        }

        fclose(fp);
    }

    return 0;
}
