#include <stdio.h>
#include "ejr.h"

int main() {
    EasyJSRHandle* ejr = ejr_new();
    int value = ejr_eval_script(ejr, "1 + 1", "<test>");

    if (value == -1) {
        return 1;
    }

    char* str_rep = ejr_val_to_string(ejr, value);

    ejr_free_string(str_rep);
    ejr_free(ejr);
    return 0;
}