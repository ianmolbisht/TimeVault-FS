#ifndef METADATA_MANAGER_H
#define METADATA_MANAGER_H

#include <time.h>

void update_metadata(char *filename);
void display_metadata(char *filename);
long get_file_size(char *filename);
time_t get_file_creation_time(char *filename);
time_t get_file_modification_time(char *filename);

#endif

