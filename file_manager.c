#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_manager.h"
#include "journal_manager.h"
#include "metadata_manager.h"
#include "lock_manager.h"

#define DATA_FOLDER "data/"

void create_file(char *filename)
{
    if (is_locked(filename))
    {
        printf("Error: File %s is locked. Cannot create.\n", filename);
        return;
    }

    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    FILE *fp = fopen(path, "w");
    if (fp == NULL)
    {
        printf("Error creating file.\n");
        return;
    }
    fclose(fp);
    update_metadata(filename);
    log_operation("CREATE", filename, NULL);
    printf("File %s created.\n", filename);
}

void write_file(char *filename, char *data)
{
    if (is_locked(filename))
    {
        printf("Error: File %s is locked. Cannot write.\n", filename);
        return;
    }

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
    update_metadata(filename);
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
    if (is_locked(filename))
    {
        printf("Error: File %s is locked. Cannot delete.\n", filename);
        return;
    }

    char path[100];
    sprintf(path, "%s%s", DATA_FOLDER, filename);
    if (remove(path) == 0)
    {
        unlock_file(filename); // Remove lock if exists
        log_operation("DELETE", filename, NULL);
        printf("File %s deleted.\n", filename);
    }
    else
    {
        printf("Failed to delete %s.\n", filename);
    }
}


//COMPRESSEF TECHNIQUE


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "file_manager.h"
// #include "journal_manager.h"

// #define DATA_FOLDER "data/"
// #define CHUNK_STORE ".chunks"
// #define CHUNK_EXT ".chunk"
// #define MANIFEST_MAGIC "TVFM1\n"
// #define CHUNK_COMP_RLE "R1"
// #define CHUNK_COMP_NONE "N1"
// #define CHUNK_SIZE 65536

// static int ensure_chunks_dir(void)
// {
//     char path[256];
//     sprintf(path, "%s%s", DATA_FOLDER, CHUNK_STORE);
// #ifdef _WIN32
//     _mkdir(path);
// #else
//     mkdir(path, 0755);
// #endif
//     return 0;
// }

// static int file_exists(const char *p)
// {
//     FILE *f = fopen(p, "rb");
//     if (!f) return 0;
//     fclose(f);
//     return 1;
// }

// static unsigned long long fnv1a64(const unsigned char *data, size_t len)
// {
//     const unsigned long long FNV_OFFSET = 1469598103934665603ULL;
//     const unsigned long long FNV_PRIME = 1099511628211ULL;
//     unsigned long long h = FNV_OFFSET;
//     for (size_t i = 0; i < len; i++)
//     {
//         h ^= (unsigned long long)data[i];
//         h *= FNV_PRIME;
//     }
//     return h;
// }

// static void hash_to_hex(unsigned long long h, char *out16)
// {
//     const char *hex = "0123456789abcdef";
//     for (int i = 0; i < 16; i++)
//     {
//         int shift = (15 - i) * 4;
//         out16[i] = hex[(h >> shift) & 0xF];
//     }
//     out16[16] = '\0';
// }

// static size_t rle_compress(const unsigned char *in, size_t in_len, unsigned char *out, size_t out_cap)
// {
//     size_t oi = 0;
//     for (size_t i = 0; i < in_len;)
//     {
//         unsigned char b = in[i];
//         size_t run = 1;
//         while (i + run < in_len && in[i + run] == b && run < 255)
//             run++;
//         if (oi + 2 > out_cap)
//             return 0;
//         out[oi++] = (unsigned char)run;
//         out[oi++] = b;
//         i += run;
//     }
//     return oi;
// }

// static size_t rle_decompress(FILE *in, FILE *out, size_t expected_out)
// {
//     size_t written = 0;
//     while (written < expected_out)
//     {
//         int c1 = fgetc(in);
//         int c2 = fgetc(in);
//         if (c1 == EOF || c2 == EOF)
//             break;
//         unsigned char count = (unsigned char)c1;
//         unsigned char byte = (unsigned char)c2;
//         for (unsigned int k = 0; k < count; k++)
//         {
//             fputc(byte, out);
//             written++;
//             if (written >= expected_out)
//                 break;
//         }
//     }
//     return written;
// }

// static int store_chunk_if_needed(const unsigned char *buf, size_t len, char *out_hash_hex16)
// {
//     unsigned long long h = fnv1a64(buf, len);
//     hash_to_hex(h, out_hash_hex16);
//     char chunk_path[512];
//     sprintf(chunk_path, "%s%s/%s%s", DATA_FOLDER, CHUNK_STORE, out_hash_hex16, CHUNK_EXT);
//     if (file_exists(chunk_path))
//         return 0;

//     unsigned char *comp = (unsigned char *)malloc(len * 2);
//     if (!comp) return -1;
//     size_t comp_len = rle_compress(buf, len, comp, len * 2);

//     FILE *cf = fopen(chunk_path, "wb");
//     if (!cf)
//     {
//         free(comp);
//         return -1;
//     }
//     if (comp_len > 0 && comp_len + 2 < len)
//     {
//         fwrite(CHUNK_COMP_RLE, 1, 2, cf);
//         fwrite(comp, 1, comp_len, cf);
//     }
//     else
//     {
//         fwrite(CHUNK_COMP_NONE, 1, 2, cf);
//         fwrite(buf, 1, len, cf);
//     }
//     fclose(cf);
//     free(comp);
//     return 0;
// }

// static int stream_chunk_from_store(const char *hash_hex16, size_t orig_size, FILE *out)
// {
//     char chunk_path[512];
//     sprintf(chunk_path, "%s%s/%s%s", DATA_FOLDER, CHUNK_STORE, hash_hex16, CHUNK_EXT);
//     FILE *cf = fopen(chunk_path, "rb");
//     if (!cf) return -1;
//     char hdr[3] = {0};
//     if (fread(hdr, 1, 2, cf) != 2)
//     {
//         fclose(cf);
//         return -1;
//     }
//     if (hdr[0] == 'N' && hdr[1] == '1')
//     {
//         char buf[4096];
//         size_t remaining = orig_size;
//         while (remaining > 0)
//         {
//             size_t to_read = remaining > sizeof(buf) ? sizeof(buf) : remaining;
//             size_t rn = fread(buf, 1, to_read, cf);
//             if (rn == 0) break;
//             fwrite(buf, 1, rn, out);
//             remaining -= rn;
//         }
//     }
//     else if (hdr[0] == 'R' && hdr[1] == '1')
//     {
//         rle_decompress(cf, out, orig_size);
//     }
//     else
//     {
//         fclose(cf);
//         return -1;
//     }
//     fclose(cf);
//     return 0;
// }

// static int ensure_manifest_header(FILE *fp)
// {
//     if (fwrite(MANIFEST_MAGIC, 1, strlen(MANIFEST_MAGIC), fp) != strlen(MANIFEST_MAGIC))
//         return -1;
//     return 0;
// }

// static int migrate_raw_file_to_manifest(const char *path)
// {
//     char tmp[256];
//     sprintf(tmp, "%s.tmp", path);
//     FILE *in = fopen(path, "rb");
//     if (!in) return -1;
//     FILE *out = fopen(tmp, "wb");
//     if (!out) { fclose(in); return -1; }
//     if (ensure_manifest_header(out) != 0) { fclose(in); fclose(out); remove(tmp); return -1; }

//     unsigned char *buf = (unsigned char *)malloc(CHUNK_SIZE);
//     if (!buf) { fclose(in); fclose(out); remove(tmp); return -1; }
//     size_t rn;
//     while ((rn = fread(buf, 1, CHUNK_SIZE, in)) > 0)
//     {
//         char hash_hex[17];
//         if (store_chunk_if_needed(buf, rn, hash_hex) != 0) { free(buf); fclose(in); fclose(out); remove(tmp); return -1; }
//         fprintf(out, "H %s %zu\n", hash_hex, rn);
//     }
//     free(buf);
//     fclose(in);
//     fclose(out);
//     remove(path);
//     rename(tmp, path);
//     return 0;
// }

// void create_file(char *filename)
// {
//     char path[100];
//     sprintf(path, "%s%s", DATA_FOLDER, filename);
//     FILE *fp = fopen(path, "w");
//     if (fp == NULL)
//     {
//         printf("Error creating file.\n");
//         return;
//     }
//     fclose(fp);
//     log_operation("CREATE", filename, NULL);
//     printf("File %s created.\n", filename);
// }

// void write_file(char *filename, char *data)
// {
//     char path[100];
//     sprintf(path, "%s%s", DATA_FOLDER, filename);

//     ensure_chunks_dir();

//     int needs_init = !file_exists(path);
//     if (needs_init)
//     {
//         FILE *nf = fopen(path, "wb");
//         if (!nf) { printf("Error opening file.\n"); return; }
//         if (ensure_manifest_header(nf) != 0) { fclose(nf); printf("Error writing header.\n"); return; }
//         fclose(nf);
//     }
//     else
//     {
//         FILE *chk = fopen(path, "rb");
//         if (!chk) { printf("Error opening file.\n"); return; }
//         char magic[6] = {0};
//         size_t mr = fread(magic, 1, 6, chk);
//         fclose(chk);
//         if (!(mr == 6 && memcmp(magic, MANIFEST_MAGIC, 6) == 0))
//         {
//             if (migrate_raw_file_to_manifest(path) != 0) { printf("Error migrating file.\n"); return; }
//         }
//     }

//     size_t in_len = strlen(data);
//     size_t payload_len = in_len + 1;
//     unsigned char *payload = (unsigned char *)malloc(payload_len);
//     if (!payload) { printf("Error.\n"); return; }
//     memcpy(payload, data, in_len);
//     payload[in_len] = '\n';

//     char hash_hex[17];
//     if (store_chunk_if_needed(payload, payload_len, hash_hex) != 0)
//     {
//         free(payload);
//         printf("Error.\n");
//         return;
//     }
//     FILE *mf = fopen(path, "ab");
//     if (!mf) { free(payload); printf("Error opening file.\n"); return; }
//     fprintf(mf, "H %s %zu\n", hash_hex, payload_len);
//     fclose(mf);
//     free(payload);

//     log_operation("WRITE", filename, data);
//     printf("Data written to %s.\n", filename);
// }

// void read_file(char *filename)
// {
//     char path[100];
//     sprintf(path, "%s%s", DATA_FOLDER, filename);
//     FILE *fp = fopen(path, "rb");
//     if (fp == NULL)
//     {
//         printf("File does not exist.\n");
//         return;
//     }
//     char magic[6] = {0};
//     size_t mr = fread(magic, 1, 6, fp);
//     printf("Contents of %s:\n", filename);
//     if (mr == 6 && memcmp(magic, MANIFEST_MAGIC, 6) == 0)
//     {
//         char line[256];
//         while (fgets(line, sizeof(line), fp))
//         {
//             if (line[0] != 'H') continue;
//             char hash_hex[33];
//             size_t orig_sz = 0;
//             if (sscanf(line, "H %32s %zu", hash_hex, &orig_sz) == 2)
//             {
//                 stream_chunk_from_store(hash_hex, orig_sz, stdout);
//             }
//         }
//         fclose(fp);
//         return;
//     }
//     else
//     {
//         if (mr > 0) fwrite(magic, 1, mr, stdout);
//         char buf[4096];
//         size_t rn;
//         while ((rn = fread(buf, 1, sizeof(buf), fp)) > 0)
//             fwrite(buf, 1, rn, stdout);
//         fclose(fp);
//     }
// }

// void delete_file(char *filename)
// {
//     char path[100];
//     sprintf(path, "%s%s", DATA_FOLDER, filename);
//     if (remove(path) == 0)
//     {
//         log_operation("DELETE", filename, NULL);
//         printf("File %s deleted.\n", filename);
//     }
//     else
//     {
//         printf("Failed to delete %s.\n", filename);
//     }
// }
