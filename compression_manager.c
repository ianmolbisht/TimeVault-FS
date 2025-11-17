#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compression_manager.h"
#include "lock_manager.h"

#define DATA_FOLDER "data/"
#define COMPRESSED_EXT ".compressed"

static size_t rle_compress(const unsigned char *in, size_t in_len, unsigned char *out, size_t out_cap)
{
    size_t oi = 0;
    for (size_t i = 0; i < in_len;)
    {
        unsigned char b = in[i];
        size_t run = 1;
        while (i + run < in_len && in[i + run] == b && run < 255)
            run++;
        if (oi + 2 > out_cap)
            return 0;
        out[oi++] = (unsigned char)run;
        out[oi++] = b;
        i += run;
    }
    return oi;
}

static size_t rle_decompress(const unsigned char *in, size_t in_len, unsigned char *out, size_t out_cap)
{
    size_t oi = 0;
    for (size_t i = 0; i < in_len && oi < out_cap; i += 2)
    {
        if (i + 1 >= in_len)
            break;
        unsigned char count = in[i];
        unsigned char byte = in[i + 1];
        for (unsigned char k = 0; k < count && oi < out_cap; k++)
        {
            out[oi++] = byte;
        }
    }
    return oi;
}

int compress_file(char *filename)
{
    if (is_locked(filename))
    {
        printf("File %s is locked. Cannot compress until it is unlocked.\n", filename);
        return -1;
    }

    char filepath[256];
    char compressed_path[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    sprintf(compressed_path, "%s%s%s", DATA_FOLDER, filename, COMPRESSED_EXT);

    FILE *in = fopen(filepath, "rb");
    if (in == NULL)
    {
        printf("Error: File %s not found.\n", filename);
        return -1;
    }

    // Read entire file
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (file_size <= 0)
    {
        fclose(in);
        printf("Error: File is empty.\n");
        return -1;
    }

    unsigned char *input = (unsigned char *)malloc(file_size);
    if (input == NULL)
    {
        fclose(in);
        printf("Error: Memory allocation failed.\n");
        return -1;
    }

    fread(input, 1, file_size, in);
    fclose(in);

    // Compress
    unsigned char *compressed = (unsigned char *)malloc(file_size * 2);
    if (compressed == NULL)
    {
        free(input);
        printf("Error: Memory allocation failed.\n");
        return -1;
    }

    size_t compressed_size = rle_compress(input, file_size, compressed, file_size * 2);

    if (compressed_size == 0 || compressed_size >= file_size)
    {
        free(input);
        free(compressed);
        printf("Compression not beneficial for %s (size: %ld bytes)\n", filename, file_size);
        return 0;
    }

    // Write compressed file
    FILE *out = fopen(compressed_path, "wb");
    if (out == NULL)
    {
        free(input);
        free(compressed);
        printf("Error: Cannot create compressed file.\n");
        return -1;
    }

    // Write header: original size (4 bytes) + compressed size (4 bytes)
    fwrite(&file_size, sizeof(long), 1, out);
    fwrite(&compressed_size, sizeof(size_t), 1, out);
    fwrite(compressed, 1, compressed_size, out);
    fclose(out);

    // Replace original with compressed
    remove(filepath);
    rename(compressed_path, filepath);

    printf("Compressed %s: %ld -> %zu bytes (%.1f%% reduction)\n", 
           filename, file_size, compressed_size, 
           (1.0 - (double)compressed_size / file_size) * 100.0);

    free(input);
    free(compressed);
    return 0;
}

int decompress_file(char *filename)
{
    if (is_locked(filename))
    {
        printf("File %s is locked. Cannot decompress until it is unlocked.\n", filename);
        return -1;
    }

    if (!is_compressed(filename))
    {
        printf("Error: File %s is not compressed.\n", filename);
        return -1;
    }

    char filepath[256];
    char temp_path[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    sprintf(temp_path, "%s%s.tmp", DATA_FOLDER, filename);

    FILE *in = fopen(filepath, "rb");
    if (in == NULL)
    {
        printf("Error: File %s not found.\n", filename);
        return -1;
    }

    // Read header
    long original_size;
    size_t compressed_size;
    if (fread(&original_size, sizeof(long), 1, in) != 1 ||
        fread(&compressed_size, sizeof(size_t), 1, in) != 1)
    {
        fclose(in);
        printf("Error: File is not in compressed format.\n");
        return -1;
    }

    // Read compressed data
    unsigned char *compressed = (unsigned char *)malloc(compressed_size);
    if (compressed == NULL)
    {
        fclose(in);
        printf("Error: Memory allocation failed.\n");
        return -1;
    }

    fread(compressed, 1, compressed_size, in);
    fclose(in);

    // Decompress
    unsigned char *decompressed = (unsigned char *)malloc(original_size);
    if (decompressed == NULL)
    {
        free(compressed);
        printf("Error: Memory allocation failed.\n");
        return -1;
    }

    size_t decompressed_size = rle_decompress(compressed, compressed_size, decompressed, original_size);

    if (decompressed_size != original_size)
    {
        free(compressed);
        free(decompressed);
        printf("Error: Decompression size mismatch.\n");
        return -1;
    }

    // Write decompressed file
    FILE *out = fopen(temp_path, "wb");
    if (out == NULL)
    {
        free(compressed);
        free(decompressed);
        printf("Error: Cannot create decompressed file.\n");
        return -1;
    }

    fwrite(decompressed, 1, original_size, out);
    fclose(out);

    // Replace compressed with decompressed
    remove(filepath);
    rename(temp_path, filepath);

    printf("Decompressed %s: %zu -> %ld bytes\n", filename, compressed_size, original_size);

    free(compressed);
    free(decompressed);
    return 0;
}

int is_compressed(char *filename)
{
    char filepath[256];
    sprintf(filepath, "%s%s", DATA_FOLDER, filename);
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL)
        return 0;

    long original_size;
    size_t compressed_size;
    int result = (fread(&original_size, sizeof(long), 1, fp) == 1 &&
                  fread(&compressed_size, sizeof(size_t), 1, fp) == 1);
    fclose(fp);
    return result;
}

