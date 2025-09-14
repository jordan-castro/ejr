// globalThis.console in the easyjs runtime.

#include "ejr.hpp"

ejr::JSArg ___print(const std::vector<ejr::JSArg>& args) {
    try {} catch (...) {
        return JSArg();
    }
}

void init_console(ejr::EasyJSR* rt) {
    rt->register_callback("___print")
}