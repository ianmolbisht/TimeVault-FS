#include <stdio.h>
#include <stdlib.h>
#include "journal_manager.h"

#define JOURNAL_FILE "data/journal.log"

void log_operation(char *op, char *filename, char *data) {
    FILE *fp = fopen(JOURNAL_FILE, "a");
    if (fp == NULL) return;
    if (data)
        fprintf(fp, "%s %s %s\n", op, filename, data);
    else
        fprintf(fp, "%s %s\n", op, filename);
    fclose(fp);
}
