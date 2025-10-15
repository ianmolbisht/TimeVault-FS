#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rollback_controller.h"

#define JOURNAL_FILE "data/journal.log"

void rollback_last()
{
    FILE *fp = fopen(JOURNAL_FILE, "r");
    if (fp == NULL)
    {
        printf("No journal found.\n");
        return;
    }

    char lines[100][256];
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), fp))
        count++;
    fclose(fp);

    if (count == 0)
    {
        printf("Journal is empty.\n");
        return;
    }
    printf("Last operation: %s", lines[count - 1]);
    printf("Rollback functionality can be implemented based on this.\n");
}
