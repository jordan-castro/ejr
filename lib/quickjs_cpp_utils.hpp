#pragma once

#include "quickjs.h"

// Build a JS_UNDEFINED for c++
JSValue js_undefined() {
    JSValue v;
    v.tag = JS_TAG_UNDEFINED;
    v.u.int32 = 0;
    return v;
}
