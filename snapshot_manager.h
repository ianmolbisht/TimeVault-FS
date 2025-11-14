#ifndef SNAPSHOT_MANAGER_H
#define SNAPSHOT_MANAGER_H

void save_snapshot(char *filename);          
void restore_snapshot_single_or_latest(char *name); 
void list_snapshots(const char *filename);
void delete_snapshots_for_file(const char *filename);
void cleanup_snapshots(char *filename, int keep_count);
#endif
