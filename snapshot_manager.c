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
    char b[256], e[64];
    split_name(f, b, e);
    int i = 1;
    while (1)
    {
        char c[512], o[1024];
        sprintf(c, "%s_snap%d%s", b, i, e);
        sprintf(o, "%s" PATH_SEP "%s", s2, c);
        if (!file_exists(o))
        {
            if (copy_file(s1, o) == 0)
                printf("snapshot %s made\n", c);
            else
                printf("error\n");
            return;
        }
        i++;
    }
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
        char dest[512];
        sprintf(dest, DATA_DIR PATH_SEP "%s", orig);
        if (copy_file(cp, dest) == 0)
            printf("restored %s from %s\n", orig, n);
        else
            printf("fail\n");
        return;
    }
    char b[256], e[64];
    split_name(n, b, e);
    char fp[512];
    sprintf(fp, SNAP_ROOT PATH_SEP "%s", n);
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
