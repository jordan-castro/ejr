# ejr (Easyjs Runtime)
The easyjs runtime written in c++, a wrapper on top of QuickJS.

> [!IMPORTANT]
> EJR is in the process of a major rewrite. Please keep in mind when using the api.

This exposes easy high level methods:
- Eval of scripts
- Evaluating Scripts
- Compiling JS programs into executables
- Calling specific functions
- registering callbacks at runtime
- registering modules at runtime

## Eval of scripts
```c
EasyJSRHandle* ejr = ejr_new();
int value = ejr_eval_script(ejr, "1 + 1", "<test>");
// See ## Read Values
```

## Eval of modules
```c
EasyJSRHandle* ejr = ejr_new();
int value = ejr_eval_module(ejr, "1 + 1", "<test>");
```

<!-- ## Compiling JS programs into executables
```cpp
std::string js_script = "console.log('hello world');";
bool success = ejr::compile_script(js_script, "test", "windows", "x64");
            // EJR::compile_module(module, name, os, arch)
if (!success) {
    ejr::log(ejr::last_error());
}
``` -->

## Calling specific functions
```c
const char* js_script = R("
    function say_hello_to(name) {
        console.log('Hello', name);
    }
")
int result = ejr_eval_script(ejr, js_script, "<test>");
ejr_free_jsvalue(ejr, result);

JSArg** args = jsarg_make_list(1);
const char* name = "Jordan";
args[0] = jsarg_str(name);

int value_id = ejr_eval_function(ejr, "say_hello_to", args, 1)

if (value_id > -1) {
    char* val_string = ejr_val_to_string(ejr, value_id);
    printf("%s", val_string);

    ejr_free_string(val_string);
}
jsarg_free_all(args, 1);
```

## Registering callables
```c
JSArg* js_print(JSArg** args, size_t argc, void* opaque) {
    if (argc > 1) {
        return jsarg_undefined();
    }

    if (args[0]->type != JSARG_TYPE_STRING) {
        return jsarg_undefined();
    }

    printf("%s\n", args[0]->value.str_val);

    return jsarg_undefined();
}

// Register callback to print
ejr_register_callback(ejr, "print", js_print, NULL);
```

## Registering classes
```c
// CustomMath class
JSArg* js_add(JSArg** args, size_t argc, void* opaque) {
    int n1 = args[0]->value.int_val;
    int n2 = args[0]->value.int_val;

    return jsarg_int(n1 + n2);
}
JSArg* js_sub(JSArg** args, size_t argc, void* opaque) {
    int n1 = args[0]->value.int_val;
    int n2 = args[0]->value.int_val;

    return jsarg_int(n1 - n2);
}
JSArg* js_mul(JSArg** args, size_t argc, void* opaque) {
    int n1 = args[0]->value.int_val;
    int n2 = args[0]->value.int_val;

    return jsarg_int(n1 * n2);
}
JSArg* js_div(JSArg** args, size_t argc, void* opaque) {
    int n1 = args[0]->value.int_val;
    int n2 = args[0]->value.int_val;

    return jsarg_int(n1 / n2);
}
JSArg* js_mod(JSArg** args, size_t argc, void* opaque) {
    int n1 = args[0]->value.int_val;
    int n2 = args[0]->value.int_val;

    return jsarg_int(n1 % n2);
}

// Class
const char* custom_math_class_script = R("
class CustomMath {
    add(n1, n2) {
        return _add(n1,n2);
    }
    sub(n1, n2) {
        return _sub(n1,n2);
    }
    mul(n1, n2) {
        return _mul(n1,n2);
    }
    div(n1, n2) {
        return _div(n1,n2);
    }
    mod(n1, n2) {
        return _mod(n1,n2);
    }
}

let cm = new CustomMath();
let a = 1;
let b = 2;
let c = cm.add(a,b); // 3

c
");

// Then register the callbacks
ejr_register_callback(ejr, "_add", js_add, NULL);
ejr_register_callback(ejr, "_sub", js_sub, NULL);
ejr_register_callback(ejr, "_mul", js_mul, NULL);
ejr_register_callback(ejr, "_div", js_div, NULL);
ejr_register_callback(ejr, "_mod", js_mod, NULL);
```

## Registering modules
```c

JSArg* js_test_add(JSArg** args, size_t argc, void* opaque) {
    int a = args[0]->value.int_val;
    int b = args[1]->value.int_val;

    return jsarg_int(a + b);
}

// Register module with test
JSMethod methods[1];
methods[0].name = "add";
methods[0].cb = js_test_add;
methods[0].opaque = NULL;
char* module_name = "ejr:test";
ejr_register_module(ejr, module_name, methods, 1);

// Now in JS you can do: 
// import {add} from "ejr:test";
// add(1,2);
```

The ejr (easyjs runtime) is built for embedding a JS runtime in easyjs related projects.
It currently uses QuickJS as it's engine of sorts, but I'm not married to it.
But for the time being it fits my use case. Important 

It's specifically a really easy way to embed JS into your application. Its only a few lines and for FFI there is currently only the rust (easyjsr) crate 
avaiable as open source. But I've also implmeented the ejr library with the Kazoku application.

## Use case
The use case of another JS runtime is specifically to be a high level wrapper for easy FFI use in:
- Kazoku
- easyjs
- Going Up

All projects that use easyjs as scripting.
