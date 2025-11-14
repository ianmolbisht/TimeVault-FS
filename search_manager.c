#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "search_manager.h"

#define DATA_FOLDER "data/"

#include <io.h>

void list_all_files(void)
{
    DIR *dir = opendir(DATA_FOLDER);
    if (dir == NULL)
    {
        printf("Cannot open data directory.\n");
        return;
    }

    struct dirent *entry;
    int count = 0;
    printf("Files in filesystem:\n");
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        printf("  %s\n", entry->d_name);
        count++;
    }
    closedir(dir);
    if (count == 0)
        printf("  (no files found)\n");
    else
        printf("Total: %d files\n", count);
}

void search_files_by_name(char *pattern)
{
    DIR *dir = opendir(DATA_FOLDER);
    if (dir == NULL)
    {
        printf("Cannot open data directory.\n");
        return;
    }

    struct dirent *entry;
    int found = 0;
    printf("Files matching '%s':\n", pattern);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;
        if (strstr(entry->d_name, pattern) != NULL)
        {
            printf("  %s\n", entry->d_name);
            found = 1;
        }
    }
    closedir(dir);
    if (!found)
        printf("  (no matches found)\n");
}

void search_files_by_content(char *pattern)
{
    DIR *dir = opendir(DATA_FOLDER);
    if (dir == NULL)
    {
        printf("Cannot open data directory.\n");
        return;
    }

    struct dirent *entry;
    int found = 0;
    printf("Files containing '%s':\n", pattern);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        char filepath[256];
        sprintf(filepath, "%s%s", DATA_FOLDER, entry->d_name);
        FILE *fp = fopen(filepath, "r");
        if (fp == NULL)
            continue;

        char line[512];
        int file_contains = 0;
        while (fgets(line, sizeof(line), fp))
        {
            if (strstr(line, pattern) != NULL)
            {
                file_contains = 1;
                break;
            }
        }
        fclose(fp);

        if (file_contains)
        {
            printf("  %s\n", entry->d_name);
            found = 1;
        }
    }
    closedir(dir);
    if (!found)
        printf("  (no matches found)\n");
}

