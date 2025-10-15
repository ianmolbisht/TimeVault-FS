#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_manager.h"
#include "journal_manager.h"

#define DATA_FOLDER "data/"

void create_file(char *filename)
{
    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        printf("Error creating file.\n");
        return;
    }
    fclose(fp);
    log_operation("CREATE", filename, NULL);
    printf("File %s created.\n", filename);
}

void write_file(char *filename, char *data)
{
    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    FILE *fp = fopen(path, "a");
    if (fp == NULL)
    {
        printf("Error opening file.\n");
        return;
    }
    fprintf(fp, "%s\n", data);
    fclose(fp);
    log_operation("WRITE", filename, data);
    printf("Data written to %s.\n", filename);
}

void read_file(char *filename)
{
    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("File does not exist.\n");
        return;
    }
    char line[256];
    printf("Contents of %s:\n", filename);
    while (fgets(line, sizeof(line), fp))
    {
        printf("%s", line);
    }
    fclose(fp);
}

void delete_file(char *filename)
{
    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    if (remove(path) == 0)
    {
        log_operation("DELETE", filename, NULL);
        printf("File %s deleted.\n", filename);
    }
    else
    {
        printf("Failed to delete %s.\n", filename);
    }
}
