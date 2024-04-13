
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    int character, count = 0;
    char *prog = argv[0]; /* program name for errors */
    if (argc == 1)        /* no args; no file to count lines from */
        printf(0);
    else
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
                while ((character = getc(fp)) != EOF)
                {
                    if (character == '\n')
                    {
                        ++count;
                    }
                }
                printf("%d\n", count);
            }
        }

    if (ferror(stdout))
    {
        fprintf(stderr, "%s: error writing stdout\n", prog);
        exit(2);
    }
    exit(0);
}