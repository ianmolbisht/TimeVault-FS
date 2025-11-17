#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "metadata_manager.h"
#include "lock_manager.h"

#define DATA_FOLDER "data/"
#define METADATA_FOLDER "data/.metadata/"
#include <direct.h>
#include <io.h>
#define stat _stat

static void ensure_metadata_dir(void)
{
    _mkdir(METADATA_FOLDER);
}

void update_metadata(char *filename)
{
    ensure_metadata_dir();
    char filepath[256];
    char metapath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    sprintf(metapath, "%s%s.meta", METADATA_FOLDER, filename);

    struct stat file_stat;
    if (stat(filepath, &file_stat) != 0)
    {
        return; // File doesn't exist
    }

    FILE *meta_fp = fopen(metapath, "w");
    if (meta_fp == NULL)
        return;

    char created_str[64];
    char modified_str[64];

    struct tm *timeinfo = localtime(&file_stat.st_ctime);
    if (timeinfo)
        strftime(created_str, sizeof(created_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    else
        strcpy(created_str, "unknown");

    timeinfo = localtime(&file_stat.st_mtime);
    if (timeinfo)
        strftime(modified_str, sizeof(modified_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    else
        strcpy(modified_str, "unknown");

    fprintf(meta_fp, "size:%ld\n", (long)file_stat.st_size);
    fprintf(meta_fp, "created:%s\n", created_str);
    fprintf(meta_fp, "modified:%s\n", modified_str);
    fclose(meta_fp);
}

void display_metadata(char *filename)
{
    if (is_locked(filename))
    {
        printf("File %s is locked. Cannot display metadata until it is unlocked.\n", filename);
        return;
    }

    char filepath[256];
    char metapath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    sprintf(metapath, "%s%s.meta", METADATA_FOLDER, filename);

    struct stat file_stat;
    if (stat(filepath, &file_stat) != 0)
    {
        printf("File %s does not exist.\n", filename);
        return;
    }

    printf("Metadata for %s:\n", filename);
    printf("  Size: %ld bytes\n", (long)file_stat.st_size);

    char time_str[64];
    struct tm *timeinfo;

    timeinfo = localtime(&file_stat.st_ctime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("  Created: %s\n", time_str);

    timeinfo = localtime(&file_stat.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("  Modified: %s\n", time_str);
}

long get_file_size(char *filename)
{
    char filepath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    struct stat file_stat;
    if (stat(filepath, &file_stat) == 0)
        return (long)file_stat.st_size;
    return -1;
}

time_t get_file_creation_time(char *filename)
{
    char filepath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    struct stat file_stat;
    if (stat(filepath, &file_stat) == 0)
        return file_stat.st_ctime;
    return 0;
}

time_t get_file_modification_time(char *filename)
{
    char filepath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    struct stat file_stat;
    if (stat(filepath, &file_stat) == 0)
        return file_stat.st_mtime;
    return 0;
}

