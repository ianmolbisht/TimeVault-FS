#ifndef COMPRESSION_MANAGER_H
#define COMPRESSION_MANAGER_H

int compress_file(char *filename);
int decompress_file(char *filename);
int is_compressed(char *filename);

#endif

