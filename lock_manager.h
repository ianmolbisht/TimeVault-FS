#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

int lock_file(char *filename);
int unlock_file(char *filename);
int is_locked(char *filename);
void list_locked_files(void);

#endif

