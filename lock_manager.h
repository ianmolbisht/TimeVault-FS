#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

int lock_file(const char *filename);
int unlock_file(const char *filename);
int is_locked(const char *filename);
void list_locked_files(void);

#endif

