#include <stdio.h>
#include "ejr.h"

JSArg js_print(const JSArg* args, size_t argc) {
    if (args[0].type == JSARG_TYPE_STRING && args[0].value.str_val) {
        printf("Print from C: %s\n", args[0].value.str_val);
        // Free your JSArg if you aren't going to return it.
        jsarg_free(&args[0]);
    } else {
        printf("Print from C: [non-string or null]\n");
    }
    printf("Arg type: %d\n", args[0].type);

    return jsarg_null();
}

JSArg js_test_add(const JSArg* args, size_t argc) {
    int a = args[0].value.int_val;
    int b = args[1].value.int_val;

    return jsarg_int(a + b);
}

int main() {
    // Init runtime
    EasyJSRHandle* ejr = ejr_new();

    // Init JSVAD
    JSValueAD* jsvad = jsvad_new();

    // Script
    char* script = "1 + 1";
    char* file_name = "<test>";

    // Result
    int value_id = ejr_eval_script(jsvad, ejr, script, file_name);

    if (value_id < 0) {
        fprintf(stderr, "Failed to eval script\n");
        jsvad_free(jsvad, ejr);
        ejr_free(ejr);
        return 1;
    }

    char* value_string = ejr_val_to_string(jsvad, ejr, value_id);

    printf("Result of JS Code: %s\n", value_string);

    // Free char*
    ejr_free_string(value_string);
    jsvad_free_jsvalue(jsvad, ejr, value_id);

    // Register callback to print
    ejr_register_callback(ejr, "print", js_print);
    // Register module with test
    JSMethod methods[1];
    methods[0].name = "add";
    methods[0].cb = js_test_add;
    char* module_name = "ejr:test";
    ejr_register_module(ejr, module_name, methods, 1);

    // Ok run a module...
    char* module_script = "import {add} from 'ejr:test';\nprint(add(1,2));";
    char* module_file = "<mod_test>";
    int module_value = ejr_eval_module(jsvad, ejr, module_script, module_file);

    jsvad_free_jsvalue(jsvad, ejr, module_value);
    // Free runtime
    jsvad_free(jsvad, ejr);
    ejr_free(ejr);

    return 0;
}
