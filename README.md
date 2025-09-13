# ejr (Easyjs Runtime)
The easyjs runtime written in c++, a wrapper on top of QuickJS.

This exposes easy high level methods:
- Eval of scripts
- Evaluating Scripts
- Compiling JS programs into executables
- Calling specific functions

## Eval of scripts
```cpp
unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();

JSValue evaled_script = easyjsr->run_script("1 + 1");
```

## Compiling JS programs into executables
```cpp
std::string js_script = "console.log('hello world');";
bool success = ejr::compile_script(js_script, "test", "windows", "x64");
            // EJR::compile_module(module, name, os, arch)
if (!success) {
    ejr::log(ejr::last_error());
}
```

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

## Creating callables
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