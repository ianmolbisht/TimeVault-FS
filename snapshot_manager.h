#ifndef SNAPSHOT_MANAGER_H
#define SNAPSHOT_MANAGER_H

void save_snapshot(char *filename);           // save next numbered snapshot for data/<filename>
void restore_snapshot_single_or_latest(char *name); // restore: either a snapshot name (file_snapN.ext) or an original filename to restore its latest snapshot
void list_snapshots(const char *filename);
void delete_snapshots_for_file(const char *filename);
#endif
