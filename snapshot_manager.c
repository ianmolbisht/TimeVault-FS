// #include "snapshot_manager.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/stat.h>

// #ifdef _WIN32
// #include <direct.h>
// #define mkdir_p(dir) _mkdir(dir)
// #define PATH_SEP "\\"
// #else
// #include <unistd.h>
// #define mkdir_p(dir) mkdir(dir, 0755)
// #define PATH_SEP "/"
// #endif

// #define DATA_DIR "data"
// #define SNAP_ROOT "snapshots"

// static int file_exists(const char *path)
// {
//     FILE *f = fopen(path, "rb");
//     if (!f)
//         return 0;
//     fclose(f);
//     return 1;
// }

// static int ensure_dir(const char *dir)
// {
//     struct stat st;
//     if (stat(dir, &st) == 0 && (st.st_mode & S_IFDIR))
//         return 0;
//     return mkdir_p(dir);
// }

// static int copy_file_bytes(const char *src, const char *dst)
// {
//     FILE *fs = fopen(src, "rb");
//     if (!fs)
//         return -1;
//     FILE *fd = fopen(dst, "wb");
//     if (!fd)
//     {
//         fclose(fs);
//         return -2;
//     }

//     char buf[4096];
//     size_t r;
//     while ((r = fread(buf, 1, sizeof(buf), fs)) > 0)
//     {
//         if (fwrite(buf, 1, r, fd) != r)
//         {
//             fclose(fs);
//             fclose(fd);
//             return -3;
//         }
//     }
//     fclose(fs);
//     fclose(fd);
//     return 0;
// }

// static void split_base_ext(const char *name, char *base, char *ext)
// {
//     const char *dot = strrchr(name, '.');
//     if (!dot)
//     {
//         strcpy(base, name);
//         ext[0] = '\0';
//         return;
//     }
//     size_t bsz = dot - name;
//     strncpy(base, name, bsz);
//     base[bsz] = '\0';
//     strcpy(ext, dot); // includes '.'
// }

// void save_snapshot(char *filename)
// {
//     if (!filename || filename[0] == '\0')
//     {
//         printf("Snapshot name required.\n");
//         return;
//     }

//     char src_path[512];
//     snprintf(src_path, sizeof(src_path), DATA_DIR PATH_SEP "%s", filename);
//     if (!file_exists(src_path))
//     {
//         printf("File '%s' not found in %s/.\n", filename, DATA_DIR);
//         return;
//     }

//     ensure_dir(SNAP_ROOT);

//     char snap_dir[512];
//     snprintf(snap_dir, sizeof(snap_dir), SNAP_ROOT PATH_SEP "%s", filename);
//     ensure_dir(snap_dir);

//     char base[256], ext[64];
//     split_base_ext(filename, base, ext);

//     int idx = 1;
//     char candidate[512];
//     for (;; ++idx)
//     {
//         snprintf(candidate, sizeof(candidate), "%s"
//                                                "_snap%d%s",
//                  base, idx, ext);
//         char outpath[1024];
//         snprintf(outpath, sizeof(outpath), "%s" PATH_SEP "%s", snap_dir, candidate);
//         if (!file_exists(outpath))
//         {
//             int r = copy_file_bytes(src_path, outpath);
//             if (r != 0)
//             {
//                 printf("Failed to save snapshot (code %d).\n", r);
//                 return;
//             }
//             printf("Snapshot saved: %s\n", candidate);
//             return;
//         }
//     }
// }

// void restore_snapshot_single_or_latest(char *name)
// {
//     if (!name || name[0] == '\0')
//     {
//         printf("Snapshot or filename required.\n");
//         return;
//     }

//     // If name contains "_snap" treat it as a snapshot filename and locate it
//     char *pos = strstr(name, "_snap");
//     if (pos)
//     {
//         // find which snapshot file and deduce parent folder (original filename)
//         // original filename = portion before "_snap" + extension portion after last '.' in name
//         // We need to find parent folder (original full filename). We will search snapshots/<orig>/ for a matching file.
//         // Approach: try to find a folder in snapshots whose file matches 'name'.
//         char folder_path[512], candidate_path[512];
//         // iterate: attempt candidate folder equals every possible original filename by trying to find the matching file.
//         // Simpler approach: try to infer original name by removing "_snapN" and appending extension from provided name.
//         // Find last dot in name to get ext
//         char base_sn[256], ext[64];
//         const char *dot = strrchr(name, '.');
//         if (dot)
//         {
//             size_t bsz = dot - name;
//             strncpy(base_sn, name, bsz);
//             base_sn[bsz] = '\0';
//             strcpy(ext, dot);
//         }
//         else
//         {
//             strcpy(base_sn, name);
//             ext[0] = '\0';
//         }
//         // remove trailing _snapN: find last occurrence of "_snap"
//         char *snappos = strstr(base_sn, "_snap");
//         if (!snappos)
//         {
//             printf("Invalid snapshot name.\n");
//             return;
//         }
//         *snappos = '\0'; // base file name without ext
//         // reconstructed original filename
//         char orig[320];
//         snprintf(orig, sizeof(orig), "%s%s", base_sn, ext);

//         snprintf(folder_path, sizeof(folder_path), SNAP_ROOT PATH_SEP "%s", orig);
//         snprintf(candidate_path, sizeof(candidate_path), "%s" PATH_SEP "%s", folder_path, name);
//         if (!file_exists(candidate_path))
//         {
//             printf("Snapshot file not found: %s\n", candidate_path);
//             return;
//         }

//         char dest[512];
//         snprintf(dest, sizeof(dest), DATA_DIR PATH_SEP "%s", orig);
//         if (copy_file_bytes(candidate_path, dest) != 0)
//         {
//             printf("Restore failed.\n");
//             return;
//         }
//         printf("Restored %s from %s\n", orig, name);
//         return;
//     }

//     // Otherwise, treat 'name' as an original filename and restore its latest snapshot.
//     char base[256], ext[64];
//     split_base_ext(name, base, ext);
//     char folder_path[512];
//     snprintf(folder_path, sizeof(folder_path), SNAP_ROOT PATH_SEP "%s", name);

//     // Find latest index by probing increasing numbers until missing; keep last existing
//     int idx = 1;
//     int last_found = 0;
//     char candidate[512], candidate_path[1024], last_path[1024];
//     while (idx < 100000)
//     {
//         snprintf(candidate, sizeof(candidate), "%s"
//                                                "_snap%d%s",
//                  base, idx, ext);
//         snprintf(candidate_path, sizeof(candidate_path), "%s" PATH_SEP "%s", folder_path, candidate);
//         if (file_exists(candidate_path))
//         {
//             last_found = idx;
//             strncpy(last_path, candidate_path, sizeof(last_path));
//             idx++;
//             continue;
//         }
//         else
//             break;
//     }

//     if (!last_found)
//     {
//         printf("No snapshots found for %s\n", name);
//         return;
//     }

//     char dest[512];
//     snprintf(dest, sizeof(dest), DATA_DIR PATH_SEP "%s", name);
//     if (copy_file_bytes(last_path, dest) != 0)
//     {
//         printf("Restore failed.\n");
//         return;
//     }
//     // produce snapshot filename for user info
//     char last_snapname[512];
//     snprintf(last_snapname, sizeof(last_snapname), "%s"
//                                                    "_snap%d%s",
//              base, last_found, ext);
//     printf("Restored %s from %s\n", name, last_snapname);
// }

// #include <dirent.h>

// void list_snapshots(const char *filename)
// {
//     if (!filename || filename[0] == '\0')
//     {
//         printf("Filename required.\n");
//         return;
//     }

//     char folder_path[512];
//     snprintf(folder_path, sizeof(folder_path), SNAP_ROOT PATH_SEP "%s", filename);

//     DIR *dir = opendir(folder_path);
//     if (!dir)
//     {
//         printf("No snapshots found for %s\n", filename);
//         return;
//     }

//     struct dirent *entry;
//     int found = 0;
//     printf("Snapshots for %s:\n", filename);

//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (entry->d_name[0] == '.') continue;
//         printf(" - %s\n", entry->d_name);
//         found = 1;
//     }

//     closedir(dir);

//     if (!found)
//         printf("No snapshots available.\n");
// }



// #include <dirent.h>

// void delete_snapshots_for_file(const char *filename)
// {
//     if (!filename || filename[0] == '\0')
//         return;

//     char folder_path[512];
//     snprintf(folder_path, sizeof(folder_path), SNAP_ROOT PATH_SEP "%s", filename);

//     DIR *dir = opendir(folder_path);
//     if (!dir)
//         return;

//     struct dirent *entry;
//     char file_path[1024];
//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (entry->d_name[0] == '.') continue;
//         snprintf(file_path, sizeof(file_path), "%s" PATH_SEP "%s", folder_path, entry->d_name);
//         remove(file_path);
//     }
//     closedir(dir);
//     rmdir(folder_path);
// }


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
#define mkdir_p(x) mkdir(x,0755)
#define PATH_SEP "/"
#endif

#define DATA_DIR "data"
#define SNAP_ROOT "snapshots"

int file_exists(char *p) {
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int make_dir(char *d) {
    struct stat s;
    if (stat(d,&s)==0 && (s.st_mode&S_IFDIR)) return 0;
    return mkdir_p(d);
}

int copy_file(char *a, char *b) {
    FILE *f1=fopen(a,"rb");
    if(!f1) return -1;
    FILE *f2=fopen(b,"wb");
    if(!f2){fclose(f1);return -1;}
    char buf[4096];
    int n;
    while((n=fread(buf,1,4096,f1))>0) fwrite(buf,1,n,f2);
    fclose(f1);
    fclose(f2);
    return 0;
}

void split_name(char *n,char *b,char *e){
    char *d=strrchr(n,'.');
    if(!d){strcpy(b,n);e[0]='\0';return;}
    int l=d-n;
    strncpy(b,n,l);
    b[l]='\0';
    strcpy(e,d);
}

void save_snapshot(char *f){
    if(!f||f[0]=='\0'){printf("need name\n");return;}
    char s1[512];
    sprintf(s1,DATA_DIR PATH_SEP "%s",f);
    if(!file_exists(s1)){printf("file not found\n");return;}
    make_dir(SNAP_ROOT);
    char s2[512];
    sprintf(s2,SNAP_ROOT PATH_SEP "%s",f);
    make_dir(s2);
    char b[256],e[64];
    split_name(f,b,e);
    int i=1;
    while(1){
        char c[512],o[1024];
        sprintf(c,"%s_snap%d%s",b,i,e);
        sprintf(o,"%s" PATH_SEP "%s",s2,c);
        if(!file_exists(o)){
            if(copy_file(s1,o)==0) printf("snapshot %s made\n",c);
            else printf("error\n");
            return;
        }
        i++;
    }
}

void restore_snapshot_single_or_latest(char *n){
    if(!n||n[0]=='\0'){printf("need name\n");return;}
    if(strstr(n,"_snap")){
        char base[256],ext[64];
        char *dot=strrchr(n,'.');
        if(dot){
            int l=dot-n;
            strncpy(base,n,l);
            base[l]='\0';
            strcpy(ext,dot);
        } else {strcpy(base,n);ext[0]='\0';}
        char *pos=strstr(base,"_snap");
        if(!pos){printf("bad name\n");return;}
        *pos='\0';
        char orig[300];
        sprintf(orig,"%s%s",base,ext);
        char fp[512],cp[512];
        sprintf(fp,SNAP_ROOT PATH_SEP "%s",orig);
        sprintf(cp,"%s" PATH_SEP "%s",fp,n);
        if(!file_exists(cp)){printf("no snapshot\n");return;}
        char dest[512];
        sprintf(dest,DATA_DIR PATH_SEP "%s",orig);
        if(copy_file(cp,dest)==0) printf("restored %s from %s\n",orig,n);
        else printf("fail\n");
        return;
    }
    char b[256],e[64];
    split_name(n,b,e);
    char fp[512];
    sprintf(fp,SNAP_ROOT PATH_SEP "%s",n);
    int i=1,last=0;
    char cand[512],candp[1024],lastp[1024];
    while(i<100000){
        sprintf(cand,"%s_snap%d%s",b,i,e);
        sprintf(candp,"%s" PATH_SEP "%s",fp,cand);
        if(file_exists(candp)){last=i;strcpy(lastp,candp);i++;}
        else break;
    }
    if(!last){printf("no snaps\n");return;}
    char dest[512];
    sprintf(dest,DATA_DIR PATH_SEP "%s",n);
    if(copy_file(lastp,dest)==0)
        printf("restored %s from %s_snap%d%s\n",n,b,last,e);
    else printf("fail\n");
}

void list_snapshots(const char *f){
    if(!f||f[0]=='\0'){printf("need file\n");return;}
    char fp[512];
    sprintf(fp,SNAP_ROOT PATH_SEP "%s",f);
    DIR *d=opendir(fp);
    if(!d){printf("none\n");return;}
    struct dirent *en;
    int found=0;
    printf("snapshots of %s:\n",f);
    while((en=readdir(d))!=NULL){
        if(en->d_name[0]=='.')continue;
        printf("%s\n",en->d_name);
        found=1;
    }
    closedir(d);
    if(!found)printf("no snaps\n");
}

void delete_snapshots_for_file(const char *f){
    if(!f||f[0]=='\0')return;
    char fp[512];
    sprintf(fp,SNAP_ROOT PATH_SEP "%s",f);
    DIR *d=opendir(fp);
    if(!d)return;
    struct dirent *en;
    char path[1024];
    while((en=readdir(d))!=NULL){
        if(en->d_name[0]=='.')continue;
        sprintf(path,"%s" PATH_SEP "%s",fp,en->d_name);
        remove(path);
    }
    closedir(d);
    rmdir(fp);
}
