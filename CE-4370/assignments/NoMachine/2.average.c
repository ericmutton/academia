
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    int character, count = 0;
    float average = 0;
    int sum = 0, number1 = 0, number2 = 0;
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
                while (fscanf(fp, "%d %d", &number1, &number2) == 2)
                {
                    sum += number1 + number2;
                    count += 2;
                }
                average = (float)((float) sum / (float) count);
                printf("%.2f\n", average);
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

