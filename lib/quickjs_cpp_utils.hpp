#pragma once

#include "quickjs.h"
#include <string>
#include <cstring> // for memcpy

// Build a JS_UNDEFINED for c++
inline JSValue js_undefined() {
    JSValue v;
    v.tag = JS_TAG_UNDEFINED;
    v.u.int32 = 0;
    return v;
}

inline JSValue js_null() {
    JSValue v;
    v.tag = JS_TAG_NULL;
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

// Build a JSCFunctionListEntry for a generic C function
inline JSCFunctionListEntry js_cfunc_def(
    const char* name,
    int length,
    JSCFunction* func
) {
    JSCFunctionListEntry e{};
    e.name = name;
    e.prop_flags = JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE;
    e.def_type = JS_DEF_CFUNC;
    e.magic = 0;
    e.u.func.length = length;
    e.u.func.cproto = JS_CFUNC_generic;

    // Set the function pointer safely via memcpy or reinterpret_cast
    std::memcpy(&e.u.func.cfunc, &func, sizeof(func)); 

    return e;
}
