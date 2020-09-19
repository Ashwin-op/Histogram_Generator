#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>

// Attempts to open file
FILE *openFile(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");

    if (!file)
    {
        printf("Error opening file!\n");
        return NULL;
    }

    return file;
}

// Formatted output of statistics
void outputResults(int *charCount)
{
    long numLetters = 0;
    long totalChars = 0;

    for (int i = 32; i < 128; i++)
    {
        totalChars += charCount[i];
        if (i >= 97 && i <= 122)
            numLetters += charCount[i];
    }

    printf("\n  Letter Frequencies\n\n");
    printf(" Char   |  Count\t  [%%] \t\tVisual\n");
    printf("------- | -----------------------------------------------------------\n");

    for (int i = 97; i < 123; i++)
    {
        printf("   %c    |  %d ", i, charCount[i]);
        printf(" \t\t[%.2f%%] \t", ((double)charCount[i] / numLetters) * 100);
        for (int j = 0; j < charCount[i]; j++)
            printf("∎");
        printf("\n");
    }

    printf("----------------------------------------------------------------------\n");
    printf("\n\tFile Statistics\n\n");
    printf(" Char Type |  Count\t  [%%]\n");
    printf("---------- | ---------------------\n");

    printf("  Letters  |  %li", numLetters);
    printf(" \t[%.2f%%]\n", ((double)numLetters / totalChars) * 100);
    printf("  Other    |  %li", totalChars - numLetters);
    printf(" \t[%.2f%%]\n", ((double)(totalChars - numLetters) / totalChars) * 100);
    printf("  Total    |  %li\n\n", totalChars);
}

/*
 * Creates a process for each letter of the alphabet
 * Opens the file and counts the number of occurrences of a specific letter
 * Uses ASCII codes to keep track of the count
 */
int *countLetters(char *filename)
{
    int *charCount;
    FILE *file;

    charCount = mmap(NULL, 128 * sizeof(*charCount), PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < 27; i++)
    {
        int c;

        if ((file = openFile(filename)) == NULL)
        {
            printf("Error opening file in child process %d!\n", getpid());
            exit(1);
        }

        pid_t pid = fork();

        if (pid == -1)
        {
            printf("Error forking process!\n");
            exit(1);
        }
        else if (pid == 0)
        {
            while ((c = tolower(fgetc(file))) != EOF)
            {
                if (i == 26 && (c < 97 || c > 122))
                    charCount[c]++; // Count other char
                else if (c == i + 97)
                    charCount[i + 97] += 1; // Count letters
            }

            fclose(file);
            exit(0);
        }
        else
            rewind(file);
    }

    for (int i = 0; i < 27; i++)
        wait(NULL);

    return charCount;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Syntax: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    FILE *file;

    if ((file = openFile(filename)) == NULL)
        return 1;

    outputResults(countLetters(filename));

    if (fclose(file) != 0)
    {
        printf("Error closing file!\n");
        return 1;
    }

    return 0;
}