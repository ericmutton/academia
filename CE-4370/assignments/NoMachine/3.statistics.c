
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/*
 * 3) Find the max, min, variance, and standard deviation.
 */

int main(int argc, char *argv[])
{
    FILE *fp;
    uint32_t character, count = 0;

    float variance = 0.0f, standardDeviation = 0.0f;
    uint32_t number = 0;
    char *prog = argv[0]; /* program name for errors */
    if (argc == 1)        /* no args; no file to count lines from */
    {
        printf(0);
    }
    else
    {
        while (--argc > 0)
        {
            fp = fopen(*++argv, "r");
            if (fp == NULL)
            {
                fprintf(stderr, "%s: can't open %s\n",
                        prog, *argv);
                exit(1);
            }
            else
            {
                float minimum = INT32_MAX, maximum = 0; /* data set has no negative integers. */
                float localMin = minimum, localMax = maximum;
                uint32_t sum = 0, count = 0;
                while (fscanf(fp, "%d", &number) == 1)
                {
                    if (number < localMin)
                        localMin = number;
                    if (number > localMax)
                        localMax = number;
                    sum += number;
                    count++;
                }
                minimum = localMin;
                maximum = localMax;
                float mean = (float)((float) sum / (float) count);
                fseek(fp, 0, SEEK_SET);
                while (fscanf(fp, "%d", &number) == 1)
                { 
                    // float x = (number - mean); // to avoid libm dependency of pow();
                    // variance += x * x;
                    variance += powf((number - mean), 2);
                }
                variance = (float) (variance / (float) (count - 1));
                standardDeviation = sqrtf(variance); // forced libm dependency
                printf("MIN = %.2f, MAX = %.2f, VAR σ^2 = %.2f, SD σ = %.6f\n", minimum, maximum, variance, standardDeviation);
            }
        }
    }
    if (ferror(stdout))
    {
        fprintf(stderr, "%s: error writing stdout\n", prog);
        exit(2);
    }
    exit(0);
}