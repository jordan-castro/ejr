#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ejr.h"

// Simple REPL prompt
#define BUFFER_SIZE 1024

int main() {
    // Create a new EasyJS runtime
    EasyJSRHandle* handle = ejr_new();
    if (!handle) {
        fprintf(stderr, "Failed to create EasyJS runtime.\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    printf("EasyJS REPL (type 'exit' to quit)\n");
    while (1) {
        printf("> ");
        if (!fgets(buffer, BUFFER_SIZE, stdin)) {
            break;
        }

        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Evaluate the JS script
        int result_id = ejr_eval_script(handle, buffer, "<repl>");
        if (result_id < 0) {
            fprintf(stderr, "Error evaluating script.\n");
            continue;
        }

        // Convert result to string
        char* result_str = ejr_val_to_string(handle, result_id);
        if (result_str) {
            printf("%s\n", result_str);
            ejr_free_string(result_str);
        } else {
            printf("undefined\n");
        }

        // Free the JSValue
        ejr_free_jsvalue(handle, result_id);
    }

    // Cleanup
    ejr_free(handle);

    return 0;
}
