#include <stdio.h>
#include <string.h>
#include "ejr.h"

JSArg* js_add(JSArg** args, size_t argc, void* opaque) {
    if (argc > 2) {
        return jsarg_null();
    }

    return jsarg_int(args[0]->value.int_val + args[1]->value.int_val);
}

JSArg* js_write_file(JSArg** args, size_t argc, void* opaque) {
    FILE *fptr;

    // Open a file in writing mode
    fptr = fopen(args[0]->value.str_val, "w");

    // Write some text to the file
    fprintf(fptr, args[1]->value.str_val);

    // Close the file
    fclose(fptr);

    return jsarg_undefined();
}

int main() {
    EasyJSRHandle* ejr = ejr_new();
    JSMethod methods[1];

    ejr_register_callback(ejr, "write_file", js_write_file, NULL);

    char* module_name = "ejr:math";
    char* method_name = "add";

    methods[0].cb = js_add;
    methods[0].name = method_name;
    methods[0].opaque = NULL;

    ejr_register_module(ejr, module_name, methods, 1);

    char* script = "import {add} from 'ejr:math'; let x = add(1, 2); write_file('jstest.txt', x.toString());";
    int value = ejr_eval_module(ejr, script, "<module>");

    if (value == -1) {
        return 1;
    }
    
    FILE *fptr;

    // Open a file in read mode
    fptr = fopen("jstest.txt", "r");

    // Store the content of the file
    char myString[2];
    fgets(myString, 2, fptr);

    fclose(fptr);

    if (strcmp(myString, "3") != 0) {
        printf(myString);
        printf("\n");
        return 2;
    }

    ejr_free_jsvalue(ejr, value);
    ejr_free(ejr);
    return 0;
}