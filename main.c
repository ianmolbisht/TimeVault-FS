#include <stdio.h>
#include <string.h>
#include "file_manager.h"
#include "rollback_controller.h"
#include "snapshot_manager.h"

int main() {
    char command[50], filename[50], data[200];

    while (1) {
        printf("> ");
        scanf("%s", command);

        if (strcmp(command, "create") == 0) {
            scanf("%s", filename);
            create_file(filename);
        } else if (strcmp(command, "write") == 0) {
            scanf("%s %[^\n]", filename, data);
            write_file(filename, data);
        } else if (strcmp(command, "read") == 0) {
            scanf("%s", filename);
            read_file(filename);
        } else if (strcmp(command, "delete") == 0) {
            scanf("%s", filename);
            delete_file(filename);
        } else if (strcmp(command, "rollback") == 0) {
            rollback_last();
        } else if (strcmp(command, "snapshot") == 0) {
            scanf("%s", filename); // snapshot name
            save_snapshot(filename);
        } else if (strcmp(command, "restore") == 0) {
            scanf("%s", filename); // snapshot name
            restore_snapshot(filename);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Unknown command.\n");
        }
    }
    return 0;
}
