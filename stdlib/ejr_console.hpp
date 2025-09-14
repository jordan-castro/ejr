// globalThis.console in the easyjs runtime.
#pragma once

#include <include/ejr.hpp>
#include <js/include_console.h>
#include <functional>

namespace ejr {
    /// @brief Print using cout 
    JSArg ___print(const JSArgs& args);
    /// @brief error using cout
    JSArg ___error(const JSArgs& args);
    /// @brief warn using cout
    JSArg ___warn(const JSArgs& args);

    /// @brief Include the console std library
    void include_console_std(EasyJSR& rt);
}