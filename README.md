# ejr (Easyjs Runtime)
The easyjs runtime written in c++, a wrapper on top of QuickJS.

This exposes easy high level methods:
- Eval of scripts
- Evaluating Scripts
- Compiling JS programs into executables
- Calling specific functions

## Eval of scripts
```cpp
auto evaled_script = ejr::eval_script("1 + 1");
```

## Evaluating scripts
```cpp
bool success = ejr::evaluate_script("1 + 1");
if (!success) {
    ejr::log(ejr::last_error());
}
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

auto evaled_script = ejr::eval_script(js_script);
evaled_script->call("say_hello_to", ["Jordan"]);
// Hello Jordan
```

## Getting a function to use later (in the same runtime instance)
```cpp
std::string js_script = R("
    function add(n1,n2) {
        return n1 + n2
    }
");

auto evaled_script = ejr::eval_script(js_script);
auto add_func = evaled_script->get_function("add");
auto result = add_func->call([1,2]);

if (result[0] != ejr::FAILED) {
    std::cout << "Result is " << result << std::endl;
}
```

## Creating callables
```cpp
void print_line(std::string msg) {
    std::cout << msg << std::endl;
}

auto runtime = ejr::new_runtime();
runtime->add_callback(print_line);

bool success = runtime->evaluate_script("print_line('Test')");
if (!success) {
    ejr::log(ejr::last_error());
}
```

## Registering classes
```cpp
class Person {
    public:
        std::string name;
        Person(const std::string& name) {
            this->name = name;
        }

        void print_name() {
            std::cout << "My name is " << this->name << std::endl;
        }
};

easyjsr->register_class<Person>(
    "Person", 
    vector<PublicMethod>{
        PublicMethod("print_name", T) // Don't know what type will look like rn
    },
    vector<PrivateMethod>{}, // None for now
    // Potentially Static Methods?
);
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