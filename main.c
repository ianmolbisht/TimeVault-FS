// #include <stdio.h>
// #include <string.h>
// #include "file_manager.h"
// #include "rollback_controller.h"
// #include "snapshot_manager.h"

// int main() {
//     char command[50], filename[50], data[200];

//     while (1) {
//         printf("> ");
//         scanf("%s", command);

//         if (strcmp(command, "create") == 0) {
//             scanf("%s", filename);
//             create_file(filename);
//         } else if (strcmp(command, "write") == 0) {
//             scanf("%s %[^\n]", filename, data);
//             write_file(filename, data);
//         } else if (strcmp(command, "read") == 0) {
//             scanf("%s", filename);
//             read_file(filename);
//         } else if (strcmp(command, "delete") == 0) {
//             scanf("%s", filename);
//             delete_file(filename);
//         } else if (strcmp(command, "rollback") == 0) {
//             rollback_last();
//         } else if (strcmp(command, "snapshot") == 0) {
//             scanf("%s", filename); // snapshot name
//             save_snapshot(filename);
//         } else if (strcmp(command, "restore") == 0) {
//             scanf("%s", filename); // snapshot name
//             restore_snapshot(filename);
//         } else if (strcmp(command, "exit") == 0) {
//             break;
//         } else {
//             printf("Unknown command.\n");
//         }
//     }
//     return 0;
// }

#include <stdio.h>
#include <string.h>
#include "file_manager.h"
#include "rollback_controller.h"
#include "snapshot_manager.h"

int main() {
    char command[50], filename[50], data[200];
    char current_file[50] = "";  // Tracks the last file used

    while (1) {
        printf("> ");
        if (scanf("%49s", command) != 1) break;

        if (strcmp(command, "create") == 0) {
            if (scanf("%49s", filename) != 1) {
                printf("Filename required.\n");
                continue;
            }
            create_file(filename);
            strcpy(current_file, filename);  // Set current file
        } 
        else if (strcmp(command, "write") == 0) {
            if (scanf("%49s", filename) != 1) {
                if (strlen(current_file) == 0) {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            } else {
                strcpy(current_file, filename);  // Update current file
            }

            getchar(); // consume leftover newline
            if (fgets(data, sizeof(data), stdin) == NULL) {
                printf("Data required.\n");
                continue;
            }
            data[strcspn(data, "\n")] = 0;
            write_file(filename, data);
        } 
        else if (strcmp(command, "read") == 0) {
            if (scanf("%49s", filename) != 1) {
                if (strlen(current_file) == 0) {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            } else {
                strcpy(current_file, filename);
            }
            read_file(filename);
        } 
        else if (strcmp(command, "delete") == 0) {
            if (scanf("%49s", filename) != 1) {
                if (strlen(current_file) == 0) {
                    printf("No current file. Specify filename.\n");
                    continue;
                }
                strcpy(filename, current_file);
            } else {
                strcpy(current_file, filename);
            }
            delete_file(filename);
            current_file[0] = '\0'; // Clear current file if deleted
        } 
        else if (strcmp(command, "rollback") == 0) {
            rollback_last();
        } 
        else if (strcmp(command, "snapshot") == 0) {
            if (scanf("%49s", filename) != 1) {
                if (strlen(current_file) == 0) {
                    printf("No current file. Specify snapshot name.\n");
                    continue;
                }
                strcpy(filename, current_file);  // default snapshot name = current file
            }
            save_snapshot(filename);
        } 
        else if (strcmp(command, "restore") == 0) {
            if (scanf("%49s", filename) != 1) {
                printf("Snapshot name required.\n");
                continue;
            }
            restore_snapshot(filename);
        } 
        else if (strcmp(command, "exit") == 0) {
            break;
        } 
        else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}
