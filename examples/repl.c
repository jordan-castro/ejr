#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ejr.h"

// Simple REPL prompt
#define BUFFER_SIZE 1024

JSArg* js_print(JSArg** args, size_t argc, void* opaque) {
    if (argc > 1) {
        return jsarg_null();
    }

    if (args[0]->type != JSARG_TYPE_STRING) {
        if (args[0]->type == JSARG_TYPE_UINT8_ARRAY) {
            printf("%zu\n", args[0]->value.u8_array_val.count);
        } else {
            return jsarg_null();
        }
    } else {
        printf("%s\n", args[0]->value.str_val);
    }


    return jsarg_null();
}

JSArg* create_array(JSArg** args, size_t argc, void* opaque) {
    // Allocate on the heap with malloc
    JSArg* c_array = jsarg_carray(2);
    jsarg_add_value_to_c_array(c_array, jsarg_int(0));
    jsarg_add_value_to_c_array(c_array, jsarg_int(1));

    return c_array;
}

JSArg* create_u8_array(JSArg** args, size_t argc, void* opaque) {
    uint8_t data[5] = {10, 20, 30, 40, 50};

    JSArg* arg = jsarg_u8_array(data, 5);

    return arg;
}

JSArg* create_i32_array(JSArg** args, size_t argc, void* opaque) {
    int32_t data[5] = {10, 20, 30, 40, 55};

    return jsarg_i32_array(data, 5);
}

JSArg* throw_exception(JSArg** args, size_t argc, void* opaque) {
    return jsarg_exception("This is just a test.", "TestError");
}

int main() {
    // Create a new EasyJS runtime
    EasyJSRHandle* handle = ejr_new();
    if (!handle) {
        fprintf(stderr, "Failed to create EasyJS runtime.\n");
        return 1;
    }

    ejr_register_callback(handle, "print", js_print, NULL);
    ejr_register_callback(handle, "create_array", create_array, NULL);
    ejr_register_callback(handle, "create_u8_array", create_u8_array, NULL);
    ejr_register_callback(handle, "create_i32_array", create_i32_array, NULL);
    ejr_register_callback(handle, "throw_exception", throw_exception, NULL);

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
