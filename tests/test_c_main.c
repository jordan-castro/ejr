#include <stdio.h>
#include "ejr.h"

JSArg* js_print(JSArg** args, size_t argc, void* opaque) {
    if (argc > 1) {
        return jsarg_null();
    }

    if (args[0]->type != JSARG_TYPE_STRING) {
        return jsarg_null();
    }

    printf("%s\n", args[0]->value.str_val);

    return jsarg_null();
}

JSArg* js_test_add(JSArg** args, size_t argc, void* opaque) {
    int a = args[0]->value.int_val;
    int b = args[1]->value.int_val;

    return jsarg_int(a + b);
}

char* file_loader(const char* file_path, void* opaque) {
    // Return "export function test() {return 1;}"
    char* result = "export function test() {return 1;}";
    return result;
}

int main() {
    // Init runtime
    EasyJSRHandle* ejr = ejr_new();

    ejr_set_file_loader(ejr, file_loader, NULL);

    // Script
    char* script = "1 + 1";
    char* file_name = "<test>";

    // Result
    int value_id = ejr_eval_script(ejr, script, file_name);

    if (value_id < 0) {
        fprintf(stderr, "Failed to eval script\n");
        ejr_free(ejr);
        return 1;
    }

    char* value_string = ejr_val_to_string(ejr, value_id);

    printf("Result of JS Code: %s\n", value_string);

    // Free char*
    ejr_free_string(value_string);
    ejr_free_jsvalue(ejr, value_id);

    // Register callback to print
    ejr_register_callback(ejr, "print", js_print, NULL);
    // Register module with test
    JSMethod methods[1];
    methods[0].name = "add";
    methods[0].cb = js_test_add;
    methods[0].opaque = NULL;
    char* module_name = "ejr:test";
    ejr_register_module(ejr, module_name, methods, 1);

    // Ok run a module...
    char* module_script = "import {add} from 'ejr:test';\nprint(add(1,2).toString());";
    char* module_file = "<mod_test>";
    int module_value = ejr_eval_module(ejr, module_script, module_file);

    ejr_free_jsvalue(ejr, module_value);

    // Check file_loader works
    char* module_script_2 = "import {test} from 'test-dayo.js'; print(test().toString())";
    char* module_file_2 = "<file_loader_test>";
    int module_value_2 = ejr_eval_module(ejr, module_script_2, module_file_2);

    ejr_free_jsvalue(ejr, module_value_2);

    // Free runtime
    ejr_free(ejr);

    return 0;
}
