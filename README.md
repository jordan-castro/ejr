# ejr (Easyjs Runtime)
The easyjs runtime written in c++, a wrapper on top of QuickJS.

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
```cpp
std::string js_script = R("
    function say_hello_to(name) {
        console.log('Hello', name);
    }
");

JSValue result = easyjsr->call("say_hello_to", vector<JSArg>{"Jordan"});
// Hello Jordan
easyjsr->free_jsval(result);
```

## Registering callables
```cpp
JSArg ___print(const vector<JSArg> args) {
    string msg = jsarg_as<std::string>(args[0]);
    cout << msg << endl;
    return msg; 
}

// Register callback
easyjsr->register_callback("___print", ___print);
easyjsr->register_callback("lambda_call", [](const vector<JSArg>& args) -> JSArg {
    cout << "Testing dayo!" << endl;
    return 1;
});
```

## Registering classes
In progress...

## Registering modules
```cpp

easyjsr->register_module("ejr:john_cena", vector<JSMethod>{
    JSMethod("test", [](const JSArgs& args) -> JSArg {
        return "AND HIS NAME IS JOHN CENA!";
    })
});

// Now in JS you can do:
// import {test} from "ejr:john_cena";
// console.log(test());
```

Think of the easyjsr runtime like a very very easy way to embed JS into your application. I have used a lot of different options, and while yes they work they 
kinda fall into three different categories.

- Too low level
    - Quickjs by itself
    - V8
- Too heavy (mb wise)
    - V8
    - Deno
- Too high level
    - Any FFI wrapper of quickjs
    - Does not support advanced callbacks
    - Requires too many types (IN JS???)

## Use case
The use case of another JS runtime is specifically to be a high level wrapper for easy FFI use in:
- Kazoku
- easyjs
- Going Up

All projects that use easyjs as scripting.
