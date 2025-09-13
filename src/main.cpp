#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>

using namespace std;
using namespace ejr;

class Person {
    public:
        string name;

        Person(vector<JSArg> args) {
            name = jsarg_as<std::string>(args[0]);
        }

        JSArg print_name(const vector<JSArg>& args) {
            cout << name << endl;
            return name;
        }
};

JSArg ___print(const vector<JSArg> args) {
    string msg = jsarg_as<std::string>(args[0]);
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

        class JSPerson {
            constructor(name) {
                this.name = name;
            }

            print_name() {
                return `my name is ${this.name}`;
            }
        }

        globalThis.jsp = new JSPerson("Test");
    )", "test.js");

    // TODO: Optionally Register C++ classes (This is for when you are using the library in a C++ project.)
    // easyjsr->register_class<Person>("Person", {
    //     {
    //         "print_name", 
    //         [](const vector<JSArg> args) -> JSArg {
    //             cout << "Calling from print_name" << endl;
    //             // Get ptr from first arg
    //             auto p = get_obj_from_ptr<Person>(args[0]);
    //             return p->print_name(args);
    //         }
    //     }
    // });
    // For all other projects use full callbacks. JS should not touch your C++ classes.

    // // cout << "Result: " << easyjsr->val_to_string(val) << endl;
    // JSValue testResult = easyjsr->eval_function("return_string", vector<JSArg>{10,2});
    // cout << "Result: " << easyjsr->val_to_string(testResult) << endl;

    // JSValue p = easyjsr->get_from_global("jsp");
    // // Call .print_name()
    // JSValue print_name = easyjsr->eval_class_function(p, "print_name", vector<JSArg>{});
    // cout << "Result: " << easyjsr->val_to_string(print_name) << endl;
    // easyjsr->free_jsval(p);

    // Register callback
    easyjsr->register_callback("___print", ___print);
    easyjsr->register_callback("easyjsr_test", [](const vector<JSArg>& args) -> JSArg {
        cout << "Testing dayo!" << endl;
        return 1;
    });
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

        console.log("Hello World, next is", add(1,1));
        
        easyjsr_test();
    )", "test2.js");
    easyjsr->free_jsval(val);

    return 0;
}
