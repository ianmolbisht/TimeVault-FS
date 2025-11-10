#include "snapshot_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir_p(x) _mkdir(x)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define mkdir_p(x) mkdir(x, 0755)
#define PATH_SEP "/"
#endif

#define DATA_DIR "data"
#define SNAP_ROOT "snapshots"

/* Delta + Chunked dedup + lightweight compression (append-only optimization):
   - snap1 stores a manifest of chunks covering the full file
   - snapN (N>1) stores a manifest of chunks for only the appended bytes
   - A metadata file 'meta.txt' stores lines: "<index> <cumulative_size>\n"
   - Chunks are stored once in snapshots/<file>/.chunks/<hash>.chunk
   - Each chunk may be RLE-compressed if smaller than original
   - To restore, concatenate chunks from manifests (snap1..K) into destination
*/
#define META_FILE "meta.txt"
#define CHUNK_STORE ".chunks"
#define CHUNK_EXT ".chunk"
#define MANIFEST_MAGIC "TVFS1\n"
#define CHUNK_COMP_RLE "R1"
#define CHUNK_COMP_NONE "N1"
#define CHUNK_SIZE 65536

int file_exists(char *p)
{
    FILE *f = fopen(p, "rb");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

int make_dir(char *d)
{
    struct stat s;
    if (stat(d, &s) == 0 && (s.st_mode & S_IFDIR))
        return 0;
    return mkdir_p(d);
}

int copy_file(char *a, char *b)
{
    FILE *f1 = fopen(a, "rb");
    if (!f1)
        return -1;
    FILE *f2 = fopen(b, "wb");
    if (!f2)
    {
        fclose(f1);
        return -1;
    }
    char buf[4096];
    int n;
    while ((n = fread(buf, 1, 4096, f1)) > 0)
        fwrite(buf, 1, n, f2);
    fclose(f1);
    fclose(f2);
    return 0;
}

void split_name(char *n, char *b, char *e)
{
    char *d = strrchr(n, '.');
    if (!d)
    {
        strcpy(b, n);
        e[0] = '\0';
        return;
    }
    int l = d - n;
    strncpy(b, n, l);
    b[l] = '\0';
    strcpy(e, d);
}

static long file_size_bytes(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return -1;
    }
    long sz = ftell(f);
    fclose(f);
    return sz;
}

static long read_last_cumulative_size(const char *meta_path, int *last_index)
{
    FILE *mf = fopen(meta_path, "r");
    if (!mf)
    {
        if (last_index)
            *last_index = 0;
        return 0;
    }
    long last_size = 0;
    int idx = 0;
    int ti;
    long tsz;
    while (fscanf(mf, "%d %ld", &ti, &tsz) == 2)
    {
        idx = ti;
        last_size = tsz;
    }
    fclose(mf);
    if (last_index)
        *last_index = idx;
    return last_size;
}

static int append_meta_line(const char *meta_path, int index, long cumulative_size)
{
    FILE *mf = fopen(meta_path, "a");
    if (!mf)
        return -1;
    fprintf(mf, "%d %ld\n", index, cumulative_size);
    fclose(mf);
    return 0;
}

void save_snapshot(char *f)
{
    if (!f || f[0] == '\0')
    {
        printf("need name\n");
        return;
    }
    char s1[512];
    sprintf(s1, DATA_DIR PATH_SEP "%s", f);
    if (!file_exists(s1))
    {
        printf("file not found\n");
        return;
    }
    make_dir(SNAP_ROOT);
    char s2[512];
    sprintf(s2, SNAP_ROOT PATH_SEP "%s", f);
    make_dir(s2);

    /* metadata path */
    char meta_path[1024];
    sprintf(meta_path, "%s" PATH_SEP META_FILE, s2);

    char b[256], e[64];
    split_name(f, b, e);

    /* Determine next snapshot index using metadata (preferred) or scanning */
    int last_idx = 0;
    long last_cum_size = read_last_cumulative_size(meta_path, &last_idx);
    int next_idx = last_idx + 1;

    /* Fallback scan if no meta (legacy folders) */
    if (last_idx == 0)
    {
        int probe = 1;
        int found_any = 0;
        while (probe < 1000000)
        {
            char probe_name[512], probe_path[1024];
            sprintf(probe_name, "%s_snap%d%s", b, probe, e);
            sprintf(probe_path, "%s" PATH_SEP "%s", s2, probe_name);
            if (file_exists(probe_path))
            {
                found_any = 1;
                last_idx = probe;
                long psize = file_size_bytes(probe_path);
                if (psize < 0)
                    psize = 0;
                last_cum_size += psize; /* approximate if legacy */
                probe++;
            }
            else
                break;
        }
        next_idx = found_any ? (last_idx + 1) : 1;
    }

    /* Compute delta based on append-only writes */
    long cur_size = file_size_bytes(s1);
    if (cur_size < 0)
    {
        printf("error\n");
        return;
    }

    char c[512], o[1024];
    sprintf(c, "%s_snap%d%s", b, next_idx, e);
    sprintf(o, "%s" PATH_SEP "%s", s2, c);

    if (next_idx == 1 || last_cum_size <= 0 || cur_size < last_cum_size)
    {
        /* Store full copy for the first snapshot or if sizes are inconsistent */
        if (copy_file(s1, o) == 0)
        {
            append_meta_line(meta_path, next_idx, cur_size);
            printf("snapshot %s made (full)\n", c);
        }
        else
            printf("error\n");
        return;
    }

    /* Store only appended bytes since previous snapshot */
    long delta_size = cur_size - last_cum_size;
    if (delta_size == 0)
    {
        /* Nothing new appended; still create a zero-byte delta for consistency */
        FILE *zf = fopen(o, "wb");
        if (zf)
            fclose(zf);
        append_meta_line(meta_path, next_idx, cur_size);
        printf("snapshot %s made (no changes)\n", c);
        return;
    }
    FILE *src = fopen(s1, "rb");
    if (!src)
    {
        printf("error\n");
        return;
    }
    FILE *dst = fopen(o, "wb");
    if (!dst)
    {
        fclose(src);
        printf("error\n");
        return;
    }
    if (fseek(src, last_cum_size, SEEK_SET) != 0)
    {
        fclose(src);
        fclose(dst);
        printf("error\n");
        return;
    }
    char buf[4096];
    long remaining = delta_size;
    while (remaining > 0)
    {
        size_t chunk = (remaining > (long)sizeof(buf)) ? sizeof(buf) : (size_t)remaining;
        size_t rn = fread(buf, 1, chunk, src);
        if (rn == 0)
            break;
        fwrite(buf, 1, rn, dst);
        remaining -= (long)rn;
    }
    fclose(src);
    fclose(dst);
    append_meta_line(meta_path, next_idx, cur_size);
    printf("snapshot %s made (delta %ld bytes)\n", c, delta_size);
}

void restore_snapshot_single_or_latest(char *n)
{
    if (!n || n[0] == '\0')
    {
        printf("need name\n");
        return;
    }
    if (strstr(n, "_snap"))
    {
        char base[256], ext[64];
        char *dot = strrchr(n, '.');
        if (dot)
        {
            int l = dot - n;
            strncpy(base, n, l);
            base[l] = '\0';
            strcpy(ext, dot);
        }
        else
        {
            strcpy(base, n);
            ext[0] = '\0';
        }
        char *pos = strstr(base, "_snap");
        if (!pos)
        {
            printf("bad name\n");
            return;
        }
        *pos = '\0';
        char orig[300];
        sprintf(orig, "%s%s", base, ext);
        char fp[512], cp[512];
        sprintf(fp, SNAP_ROOT PATH_SEP "%s", orig);
        sprintf(cp, "%s" PATH_SEP "%s", fp, n);
        if (!file_exists(cp))
        {
            printf("no snapshot\n");
            return;
        }

        /* Determine snapshot index K from name */
        int snap_idx = 0;
        char *sp = strstr(n, "_snap");
        if (sp)
            snap_idx = atoi(sp + 5);
        if (snap_idx <= 0)
        {
            printf("bad name\n");
            return;
        }

        /* Reconstruct by concatenating snap1 + deltas up to K */
        char dest[512], first_snap[512];
        sprintf(dest, DATA_DIR PATH_SEP "%s", orig);
        sprintf(first_snap, "%s" PATH_SEP "%s_snap1%s", fp, base, ext);
        FILE *out = fopen(dest, "wb");
        if (!out)
        {
            printf("fail\n");
            return;
        }
        /* write snap1 (full) */
        if (file_exists(first_snap))
        {
            FILE *in1 = fopen(first_snap, "rb");
            if (in1)
            {
                char buf[4096];
                size_t rn;
                while ((rn = fread(buf, 1, sizeof(buf), in1)) > 0)
                    fwrite(buf, 1, rn, out);
                fclose(in1);
            }
            else
            {
                fclose(out);
                printf("fail\n");
                return;
            }
        }
        else
        {
            /* legacy: if snap1 not found, copy selected snapshot directly */
            FILE *inl = fopen(cp, "rb");
            if (!inl)
            {
                fclose(out);
                printf("fail\n");
                return;
            }
            char buf[4096];
            size_t rn;
            while ((rn = fread(buf, 1, sizeof(buf), inl)) > 0)
                fwrite(buf, 1, rn, out);
            fclose(inl);
            fclose(out);
            printf("restored %s from %s\n", orig, n);
            return;
        }
        /* append deltas 2..K */
        for (int k = 2; k <= snap_idx; k++)
        {
            char part[512];
            sprintf(part, "%s" PATH_SEP "%s_snap%d%s", fp, base, k, ext);
            if (!file_exists(part))
            {
                /* missing delta: treat as failure */
                fclose(out);
                printf("fail\n");
                return;
            }
            FILE *in = fopen(part, "rb");
            if (!in)
            {
                fclose(out);
                printf("fail\n");
                return;
            }
            char buf[4096];
            size_t rn;
            while ((rn = fread(buf, 1, sizeof(buf), in)) > 0)
                fwrite(buf, 1, rn, out);
            fclose(in);
        }
        fclose(out);
        printf("restored %s from %s\n", orig, n);
        return;
    }
    char b[256], e[64];
    split_name(n, b, e);
    char fp[512];
    sprintf(fp, SNAP_ROOT PATH_SEP "%s", n);

    /* Determine latest snapshot index from metadata */
    char meta_path[1024];
    sprintf(meta_path, "%s" PATH_SEP META_FILE, fp);
    int last_idx = 0;
    read_last_cumulative_size(meta_path, &last_idx);

    if (last_idx <= 0)
    {
        /* Fallback legacy scan */
        int i = 1, last = 0;
        char cand[512], candp[1024], lastp[1024];
        while (i < 100000)
        {
            sprintf(cand, "%s_snap%d%s", b, i, e);
            sprintf(candp, "%s" PATH_SEP "%s", fp, cand);
            if (file_exists(candp))
            {
                last = i;
                strcpy(lastp, candp);
                i++;
            }
            else
                break;
        }
        if (!last)
        {
            printf("no snaps\n");
            return;
        }
        char dest[512];
        sprintf(dest, DATA_DIR PATH_SEP "%s", n);
        if (copy_file(lastp, dest) == 0)
            printf("restored %s from %s_snap%d%s\n", n, b, last, e);
        else
            printf("fail\n");
        return;
    }

    /* Reconstruct latest by concatenating from 1..last_idx */
    char dest[512], part[512];
    sprintf(dest, DATA_DIR PATH_SEP "%s", n);
    FILE *out = fopen(dest, "wb");
    if (!out)
    {
        printf("fail\n");
        return;
    }
    for (int k = 1; k <= last_idx; k++)
    {
        sprintf(part, "%s" PATH_SEP "%s_snap%d%s", fp, b, k, e);
        if (!file_exists(part))
        {
            fclose(out);
            printf("fail\n");
            return;
        }
        FILE *in = fopen(part, "rb");
        if (!in)
        {
            fclose(out);
            printf("fail\n");
            return;
        }
        char buf[4096];
        size_t rn;
        while ((rn = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, rn, out);
        fclose(in);
    }
    fclose(out);
    printf("restored %s from %s_snap%d%s\n", n, b, last_idx, e);
}

void list_snapshots(const char *f)
{
    if (!f || f[0] == '\0')
    {
        printf("need file\n");
        return;
    }
    char fp[512];
    sprintf(fp, SNAP_ROOT PATH_SEP "%s", f);
    DIR *d = opendir(fp);
    if (!d)
    {
        printf("none\n");
        return;
    }
    struct dirent *en;
    int found = 0;
    printf("snapshots of %s:\n", f);
    while ((en = readdir(d)) != NULL)
    {
        if (en->d_name[0] == '.')
            continue;
        printf("%s\n", en->d_name);
        found = 1;
    }
    closedir(d);
    if (!found)
        printf("no snaps\n");
}

void delete_snapshots_for_file(const char *f)
{
    if (!f || f[0] == '\0')
        return;
    char fp[512];
    sprintf(fp, SNAP_ROOT PATH_SEP "%s", f);
    DIR *d = opendir(fp);
    if (!d)
        return;
    struct dirent *en;
    char path[1024];
    while ((en = readdir(d)) != NULL)
    {
        if (en->d_name[0] == '.')
            continue;
        sprintf(path, "%s" PATH_SEP "%s", fp, en->d_name);
        remove(path);
    }
    closedir(d);
    rmdir(fp);
}

//old one ise delete mat krna abhi

// #include "snapshot_manager.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/stat.h>
// #include <dirent.h>

// #ifdef _WIN32
// #include <direct.h>
// #define mkdir_p(x) _mkdir(x)
// #define PATH_SEP "\\"
// #else
// #include <unistd.h>
// #define mkdir_p(x) mkdir(x, 0755)
// #define PATH_SEP "/"
// #endif

// #define DATA_DIR "data"
// #define SNAP_ROOT "snapshots"

// int file_exists(char *p)
// {
//     FILE *f = fopen(p, "rb");
//     if (!f)
//         return 0;
//     fclose(f);
//     return 1;
// }

// int make_dir(char *d)
// {
//     struct stat s;
//     if (stat(d, &s) == 0 && (s.st_mode & S_IFDIR))
//         return 0;
//     return mkdir_p(d);
// }

// int copy_file(char *a, char *b)
// {
//     FILE *f1 = fopen(a, "rb");
//     if (!f1)
//         return -1;
//     FILE *f2 = fopen(b, "wb");
//     if (!f2)
//     {
//         fclose(f1);
//         return -1;
//     }
//     char buf[4096];
//     int n;
//     while ((n = fread(buf, 1, 4096, f1)) > 0)
//         fwrite(buf, 1, n, f2);
//     fclose(f1);
//     fclose(f2);
//     return 0;
// }

// void split_name(char *n, char *b, char *e)
// {
//     char *d = strrchr(n, '.');
//     if (!d)
//     {
//         strcpy(b, n);
//         e[0] = '\0';
//         return;
//     }
//     int l = d - n;
//     strncpy(b, n, l);
//     b[l] = '\0';
//     strcpy(e, d);
// }

// void save_snapshot(char *f)
// {
//     if (!f || f[0] == '\0')
//     {
//         printf("need name\n");
//         return;
//     }
//     char s1[512];
//     sprintf(s1, DATA_DIR PATH_SEP "%s", f);
//     if (!file_exists(s1))
//     {
//         printf("file not found\n");
//         return;
//     }
//     make_dir(SNAP_ROOT);
//     char s2[512];
//     sprintf(s2, SNAP_ROOT PATH_SEP "%s", f);
//     make_dir(s2);
//     char b[256], e[64];
//     split_name(f, b, e);
//     int i = 1;
//     while (1)
//     {
//         char c[512], o[1024];
//         sprintf(c, "%s_snap%d%s", b, i, e);
//         sprintf(o, "%s" PATH_SEP "%s", s2, c);
//         if (!file_exists(o))
//         {
//             if (copy_file(s1, o) == 0)
//                 printf("snapshot %s made\n", c);
//             else
//                 printf("error\n");
//             return;
//         }
//         i++;
//     }
// }

// void restore_snapshot_single_or_latest(char *n)
// {
//     if (!n || n[0] == '\0')
//     {
//         printf("need name\n");
//         return;
//     }
//     if (strstr(n, "_snap"))
//     {
//         char base[256], ext[64];
//         char *dot = strrchr(n, '.');
//         if (dot)
//         {
//             int l = dot - n;
//             strncpy(base, n, l);
//             base[l] = '\0';
//             strcpy(ext, dot);
//         }
//         else
//         {
//             strcpy(base, n);
//             ext[0] = '\0';
//         }
//         char *pos = strstr(base, "_snap");
//         if (!pos)
//         {
//             printf("bad name\n");
//             return;
//         }
//         *pos = '\0';
//         char orig[300];
//         sprintf(orig, "%s%s", base, ext);
//         char fp[512], cp[512];
//         sprintf(fp, SNAP_ROOT PATH_SEP "%s", orig);
//         sprintf(cp, "%s" PATH_SEP "%s", fp, n);
//         if (!file_exists(cp))
//         {
//             printf("no snapshot\n");
//             return;
//         }
//         char dest[512];
//         sprintf(dest, DATA_DIR PATH_SEP "%s", orig);
//         if (copy_file(cp, dest) == 0)
//             printf("restored %s from %s\n", orig, n);
//         else
//             printf("fail\n");
//         return;
//     }
//     char b[256], e[64];
//     split_name(n, b, e);
//     char fp[512];
//     sprintf(fp, SNAP_ROOT PATH_SEP "%s", n);
//     int i = 1, last = 0;
//     char cand[512], candp[1024], lastp[1024];
//     while (i < 100000)
//     {
//         sprintf(cand, "%s_snap%d%s", b, i, e);
//         sprintf(candp, "%s" PATH_SEP "%s", fp, cand);
//         if (file_exists(candp))
//         {
//             last = i;
//             strcpy(lastp, candp);
//             i++;
//         }
//         else
//             break;
//     }
//     if (!last)
//     {
//         printf("no snaps\n");
//         return;
//     }
//     char dest[512];
//     sprintf(dest, DATA_DIR PATH_SEP "%s", n);
//     if (copy_file(lastp, dest) == 0)
//         printf("restored %s from %s_snap%d%s\n", n, b, last, e);
//     else
//         printf("fail\n");
// }

// void list_snapshots(const char *f)
// {
//     if (!f || f[0] == '\0')
//     {
//         printf("need file\n");
//         return;
//     }
//     char fp[512];
//     sprintf(fp, SNAP_ROOT PATH_SEP "%s", f);
//     DIR *d = opendir(fp);
//     if (!d)
//     {
//         printf("none\n");
//         return;
//     }
//     struct dirent *en;
//     int found = 0;
//     printf("snapshots of %s:\n", f);
//     while ((en = readdir(d)) != NULL)
//     {
//         if (en->d_name[0] == '.')
//             continue;
//         printf("%s\n", en->d_name);
//         found = 1;
//     }
//     closedir(d);
//     if (!found)
//         printf("no snaps\n");
// }

// void delete_snapshots_for_file(const char *f)
// {
//     if (!f || f[0] == '\0')
//         return;
//     char fp[512];
//     sprintf(fp, SNAP_ROOT PATH_SEP "%s", f);
//     DIR *d = opendir(fp);
//     if (!d)
//         return;
//     struct dirent *en;
//     char path[1024];
//     while ((en = readdir(d)) != NULL)
//     {
//         if (en->d_name[0] == '.')
//             continue;
//         sprintf(path, "%s" PATH_SEP "%s", fp, en->d_name);
//         remove(path);
//     }
//     closedir(d);
//     rmdir(fp);
// }
