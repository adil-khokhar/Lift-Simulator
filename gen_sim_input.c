#include <stdio.h>

int main(void)
{
    FILE *fp;
    int randomNum;
    int numRequests;

    fp = fopen("sim_input", "w+");

    if (fp == NULL) 
    {
        printf("Could not open file\n");
    }

    else
    {
        srand(time(0));

        numRequests = (rand() % (100 - 50 + 1)) + 50;

        printf("Num requests = %d\n", numRequests);

        for(int ii = 0; ii < numRequests; ii++)
        {
            randomNum = (rand() % (20 - 1 + 1)) + 1;
            fprintf(fp,"%d ",randomNum);
            randomNum = (rand() % (20 - 1 + 1)) + 1;
            fprintf(fp,"%d\n",randomNum);
        }

        fclose(fp);
    }

    return 0;
}
