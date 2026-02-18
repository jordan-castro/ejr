#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>
#include <filesystem>
#include "utils.hpp"

using namespace std;
using namespace ejr;

int main() {
    unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();

    easyjsr->register_callback("get_cwd", [](const JSArgs& args) -> JSArg {
        return filesystem::current_path().string();
    });

    easyjsr->register_callback("print", [](const JSArgs& args) -> JSArg {
        cout << jsarg_to_str(args[0]) << endl;
        return JSArg(nullptr);
    });

    easyjsr->register_module("ejr:john_cena", vector<JSMethod>{
        JSMethod("test", [](const JSArgs& args) -> JSArg {
            return JSArg("AND HIS NAME IS JOHN CENA!");
        })
    });

    auto result = load_js_file("run.js");
    if (result.has_error) {
        cout << "Could not load file: " << result.msg << endl;
    }
    try {
        auto val = easyjsr->eval_script(result.result, "run.js");
        cout << easyjsr->val_to_string(val) << endl;
    } catch (const std::exception& e) {
        cout << e.what() << endl;
    }

    return 0;
}
