#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>

using namespace std;
using namespace ejr;

JSArg ___print(const vector<JSArg> args) {
    string msg = as<std::string>(args[0]);
    cout << msg << endl;
    return msg;
}

int main() {
    unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();
    JSValue val = easyjsr->run_script(R"(
        function add(n1, n2) {return n1 + n2;} 

        function return_string(n1,n2) {
            return `test ${add(n1, n2)}`;
        }

        class Person {
            constructor(name) {
                this.name = name;
            }

            print_name() {
                return `my name is ${this.name}`;
            }
        }

        p = new Person("Test");
    )", "test.js");
    
    // cout << "Result: " << easyjsr->val_to_string(val) << endl;
    JSValue testResult = easyjsr->eval_function("return_string", vector<JSArg>{10,2});
    cout << "Result: " << easyjsr->val_to_string(testResult) << endl;

    JSValue p = easyjsr->get_from_global("p");
    // Call .print_name()
    JSValue print_name = easyjsr->eval_class_function(p, "print_name", vector<JSArg>{});
    cout << "Result: " << easyjsr->val_to_string(print_name) << endl;
    easyjsr->free_jsval(p);

    // Register callback
    easyjsr->register_callback("___print", ___print);
    easyjsr->free_jsval(val);
    val = easyjsr->run_script(R"(
        globalThis.console = {
            /**
             * Print to the console.
             * @param  {...any} v the arguments to print.
             */
            log(...v) {
                globalThis.___print(`${v.join(" ")}`);
            },
        };

        console.log("Hello World, next is", add(1,1))
    )", "test2.js");
    easyjsr->free_jsval(val);

    return 0;
}
