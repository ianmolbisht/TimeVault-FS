#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "lock_manager.h"
#include <direct.h>
#include <io.h>

#define DATA_FOLDER "data/"
#define LOCK_FOLDER "data/.locks/"


static void ensure_lock_dir(void)
{
    _mkdir(LOCK_FOLDER);
}

static char *get_lock_path(char *filename)
{
    static char lock_path[256];
    sprintf(lock_path, "%s%s.lock", LOCK_FOLDER, filename);
    return lock_path;
}

int lock_file(char *filename)
{
    ensure_lock_dir();
    char *lock_path = get_lock_path(filename);

    if (is_locked(filename))
    {
        printf("File %s is already locked.\n", filename);
        return -1;
    }

    FILE *lock_fp = fopen(lock_path, "w");
    if (lock_fp == NULL)
    {
        printf("Error: Cannot create lock file.\n");
        return -1;
    }

    fprintf(lock_fp, "locked\n");
    fclose(lock_fp);
    printf("File %s locked successfully.\n", filename);
    return 0;
}

int unlock_file(char *filename)
{
    char *lock_path = get_lock_path(filename);

    if (!is_locked(filename))
    {
        printf("File %s is not locked.\n", filename);
        return -1;
    }

    if (remove(lock_path) == 0)
    {
        printf("File %s unlocked successfully.\n", filename);
        return 0;
    }
    else
    {
        printf("Error: Cannot remove lock file.\n");
        return -1;
    }
}

int is_locked(char *filename)
{
    char *lock_path = get_lock_path(filename);
    FILE *fp = fopen(lock_path, "r");
    if (fp == NULL)
        return 0;
    fclose(fp);
    return 1;
}

void list_locked_files(void)
{
    ensure_lock_dir();
    DIR *dir = opendir(LOCK_FOLDER);
    if (dir == NULL)
    {
        printf("No locked files.\n");
        return;
    }

    struct dirent *entry;
    int count = 0;
    printf("Locked files:\n");
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        // Remove .lock extension
        char filename[256];
        strncpy(filename, entry->d_name, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        char *dot = strrchr(filename, '.');
        if (dot && strcmp(dot, ".lock") == 0)
            *dot = '\0';

        printf("  %s\n", filename);
        count++;
    }
    closedir(dir);
    if (count == 0)
        printf("  (no locked files)\n");
}

