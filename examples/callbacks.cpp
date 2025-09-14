#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>

using namespace std;
using namespace ejr;

JSArg add_callback(const JSArgs& args) {
    return jsarg_as<int>(args[0]) + jsarg_as<int>(args[1]);
}

int main() {
    unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();

    easyjsr->register_callback("add", add_callback);
    easyjsr->register_callback("sub", [](const JSArgs& args) -> JSArg {
        return jsarg_as<int>(args[0]) - jsarg_as<int>(args[1]);
    });

    auto result = easyjsr->run_script(R"(
        sub(add(1, 1), add(1, 0)) // result is going to be 1 (2 - 1)
    )", "<script>");
    cout << "Result is: " << easyjsr->val_to_string(result) << endl;

    return 0;
}
