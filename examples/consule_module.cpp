// Include the console stdlib.

#include <iostream>
#include <vector>
#include <string>
#include "ejr.hpp"

using namespace std;
using namespace ejr;

vector<string> get_all_strs(const JSArgs& args) {
    vector<string> strs;

    for (const auto& arg : args) {
        strs.push_back(jsarg_to_str(arg));
    }

    return strs;
}

JSArg ___print(const JSArgs& args) {
    // Get all messages
    auto msgs = get_all_strs(args);
    for (const auto& msg : msgs) {
        cout << msg << endl;
    }

    return args[0];
}

JSArg ___error(const JSArgs& args) {
    cout << "ERROR: ";
    return ___print(args);
}

JSArg ___warn(const JSArgs& args) {
    cout << "WARNING: ";
    return ___print(args);
}

void include_console_std(EasyJSR& rt) {
    rt.register_callback("___print", ___print);
    rt.register_callback("___error", ___error);
    rt.register_callback("___warn", ___warn);

    JSValue val = rt.eval_script(console_contents, "<script>");
    rt.free_jsval(val);
}