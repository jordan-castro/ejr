#include "ejr.h"
#include <stdio.h>
#include <string.h>

int main() {
    JSArg* msg = jsarg_str("Test");
    char* response = jsarg_to_string(msg);
    if (response) {
        printf("%s\n", response);
    } else {
        printf("Null");
    }
    ejr_free_string(response);

    return 0;
}