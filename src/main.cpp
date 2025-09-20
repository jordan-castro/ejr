#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>
#include <stdlib/ejr_console.hpp>
#include <filesystem>
#include "utils.hpp"

using namespace std;
using namespace ejr;

int main() {
    unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();

    // REPL test
    include_console_std(*easyjsr);

    easyjsr->register_callback("get_cwd", [](const JSArgs& args) -> JSArg {
        return filesystem::current_path().string();
    });

    easyjsr->register_module("ejr:john_cena", vector<JSMethod>{
        JSMethod("test", [](const JSArgs& args) -> JSArg {
            return "AND HIS NAME IS JOHN CENA!";
        })
    });

    auto result = load_js_file("run.js");
    if (result.has_error) {
        cout << "Could not load file: " << result.msg << endl;
    }
    try {
    auto val = easyjsr->eval_module(result.result, "run.js");
    cout << easyjsr->val_to_string(val) << endl;
    } catch (const std::exception& e) {
        cout << e.what() << endl;
    }
    // while (1) {
    //     string input;
    //     cout << ">> ";
    //     getline(cin, input);
    //     input = str_trim(input);

    //     if (input == "q") {
    //         break;
    //     }

    //     // Run JS and get as string
    //     try {
    //         auto val = easyjsr->eval_script(input, "repl.js");
    //         cout << easyjsr->val_to_string(val) << endl;
    //     } catch (const std::exception& e) {
    //         cout << e.what() << endl;
    //     }
    // }

    // cout << "So long, and thanks for all the fish!" << endl;

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
    return 0;
}
