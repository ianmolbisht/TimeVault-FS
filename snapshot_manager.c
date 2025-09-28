#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DATA_FOLDER "data/"

void save_snapshot(char *name) {
    char cmd[256];
    sprintf(cmd, "mkdir \"%s%s\"", DATA_FOLDER, name);
    system(cmd);
    sprintf(cmd, "copy \"%s*\" \"%s%s/\" > nul", DATA_FOLDER, DATA_FOLDER, name); // Windows copy
    system(cmd);
    printf("Snapshot %s saved.\n", name);
}

void restore_snapshot(char *name) {
    char cmd[256];
    sprintf(cmd, "copy \"%s%s/*\" \"%s\" /Y > nul", DATA_FOLDER, name, DATA_FOLDER); // Windows copy
    system(cmd);
    printf("Snapshot %s restored.\n", name);
}
