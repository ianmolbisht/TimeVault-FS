#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_manager.h"
#include "rollback_controller.h"
#include "snapshot_manager.h"
#include "metadata_manager.h"
#include "search_manager.h"
#include "compression_manager.h"
#include "lock_manager.h"

int main()
{
    char command[50], filename[50], data[200];
    char current_file[50] = ""; // Tracks the last file used

    while (1)
    {
        printf("> ");
        if (scanf("%49s", command) != 1)
            break;

        if (strcmp(command, "create") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                printf("Filename required.\n");
                continue;
            }
            create_file(filename);
            strcpy(current_file, filename);
        }
        else if (strcmp(command, "write") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }

            getchar(); // consume leftover newline
            if (fgets(data, sizeof(data), stdin) == NULL)
            {
                printf("Data required.\n");
                continue;
            }
            data[strcspn(data, "\n")] = 0;
            write_file(filename, data);
        }
        else if (strcmp(command, "read") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            read_file(filename);
        }
        else if (strcmp(command, "delete") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            delete_file(filename);
            delete_snapshots_for_file(filename);
            current_file[0] = '\0';
            printf("Deleted all snapshots of %s.\n", filename);
        }
        else if (strcmp(command, "rollback") == 0)
        {
            rollback_last();
        }
        else if (strcmp(command, "snapshot") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify snapshot name.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            save_snapshot(filename);
        }
        else if (strcmp(command, "restore") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                printf("Snapshot or file name required.\n");
                continue;
            }
            restore_snapshot_single_or_latest(filename);
        }
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else if (strcmp(command, "listsnapshots") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            list_snapshots(filename);
        }
        else if (strcmp(command, "cleanup-snapshots") == 0)
        {
            int keep_count = 5; // default
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            if (scanf("%d", &keep_count) == 1)
            {
                // keep_count already set
            }
            cleanup_snapshots(filename, keep_count);
        }
        else if (strcmp(command, "info") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            display_metadata(filename);
        }
        else if (strcmp(command, "search") == 0)
        {
            char pattern[100];
            if (scanf("%99s", pattern) != 1)
            {
                printf("Pattern required.\n");
                continue;
            }
            search_files_by_name(pattern);
        }
        else if (strcmp(command, "grep") == 0)
        {
            char pattern[100];
            if (scanf("%99s", pattern) != 1)
            {
                printf("Pattern required.\n");
                continue;
            }
            search_files_by_content(pattern);
        }
        else if (strcmp(command, "list-files") == 0)
        {
            list_all_files();
        }
        else if (strcmp(command, "compress") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            compress_file(filename);
        }
        else if (strcmp(command, "decompress") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            decompress_file(filename);
        }
        else if (strcmp(command, "lock") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            lock_file(filename);
        }
        else if (strcmp(command, "unlock") == 0)
        {
            if (scanf("%49s", filename) != 1)
            {
                if (strlen(current_file) == 0)
                {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            }
            else
            {
                strcpy(current_file, filename);
            }
            unlock_file(filename);
        }
        else if (strcmp(command, "list-locks") == 0)
        {
            list_locked_files();
        }
        else if (strcmp(command, "help") == 0)
        {
            printf("Available commands:\n");
            printf("  create <filename> - Create a new file\n");
            printf("  write <filename> <data> - Write data to file\n");
            printf("  read <filename> - Read file contents\n");
            printf("  delete <filename> - Delete a file\n");
            printf("  snapshot <filename> - Create snapshot\n");
            printf("  restore <filename|snapshot> - Restore from snapshot\n");
            printf("  listsnapshots <filename> - List snapshots\n");
            printf("  cleanup-snapshots <filename> [N] - Keep only N latest snapshots\n");
            printf("  rollback - Rollback last operation\n");
            printf("  info <filename> - Show file metadata\n");
            printf("  search <pattern> - Search files by name\n");
            printf("  grep <pattern> - Search files by content\n");
            printf("  list-files - List all files\n");
            printf("  compress <filename> - Compress file\n");
            printf("  decompress <filename> - Decompress file\n");
            printf("  lock <filename> - Lock file\n");
            printf("  unlock <filename> - Unlock file\n");
            printf("  list-locks - List locked files\n");
            printf("  exit - Exit program\n");
        }
        else
        {
            printf("Unknown command. Type 'help' for available commands.\n");
        }
    }

    return 0;
}
