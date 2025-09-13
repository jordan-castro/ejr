#pragma once

#include "quickjs.h"

// Build a JS_UNDEFINED for c++
inline JSValue js_undefined() {
    JSValue v;
    v.tag = JS_TAG_UNDEFINED;
    v.u.int32 = 0;
    return v;
}
inline JSValue js_exception() {
    JSValue v;
    v.tag = JS_TAG_EXCEPTION;
    v.u.int32 = 0;
    return v;
}

inline JSValue js_mkptr(int tag, void* ptr) { 
    JSValue v;
    v.u.ptr = ptr;   // assign pointer
    v.tag = tag;     // assign tag
    return v;
}
