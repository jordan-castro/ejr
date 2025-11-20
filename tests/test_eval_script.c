#include <stdio.h>
#include "ejr.h"

int main() {
    EasyJSRHandle* ejr = ejr_new();
    char* script = "1 + 1";
    int value = ejr_eval_script(ejr, script, "<test>");

    if (value == -1) {
        return 1;
    }

    JSArg* val = jsarg_from_jsvalue(ejr, value);
    if (val->value.int_val != 2) {
        return 2;
    }

    ejr_free(ejr);
    return 0;
}