#pragma once

#include <lib/quickjs.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <type_traits>
#include <functional>
#include <unordered_map>

namespace ejr
{
    /// @brief A JSArg for dynamic typing.
    using _JSArg = std::variant<int, double, float, long, std::string, bool>;
    /// @brief A JSArgs type
    using JSArg = std::variant<_JSArg, std::vector<_JSArg>>;
    /// @brief A type for Dynamic Callbacks ([JSArgs]) -> JSArg
    using DynCallback = std::function<JSArg(const std::vector<JSArg>&)>;

    template<typename T>
    T as(const ejr::JSArg& arg) {
        if (std::holds_alternative<_JSArg>(arg)) {
            return std::get<T>(std::get<_JSArg>(arg));
        }
        throw std::bad_variant_access(); // not a scalar
    }

    /// @brief Convert a JSArg into a JSValue
    JSValue __to_js(JSContext *ctx, const _JSArg &arg);
    /// @brief Convert a JSArgs type into a JSValue.
    JSValue to_js(JSContext *ctx, const JSArg &args);
    /// @brief Convert a JSValue into a JSArg
    JSArg from_js(JSContext *ctx, JSValue value);

    /**
     * @brief The easyjs runtime.
     */
    class EasyJSR
    {
    private:
        /// @brief The runtime from quickjs.
        /// Each EasyJSR instance has its own sandboxed runtime.
        JSRuntime *runtime = nullptr;
        JSContext *ctx = nullptr;

        /// @brief Callbacks
        std::unordered_map<std::string, DynCallback> callbacks;

    public:
        EasyJSR();
        ~EasyJSR();

        /// @brief a really easy way to run a JS script. No strings attached, very plain way.
        JSValue run_script(const std::string &js, const std::string &file_name);

        /// @brief free a JSValue using this runtimes context.
        void free_jsval(JSValue value);

        /// @brief Free a vector of JSValue
        void free_jsvals(const std::vector<JSValue>& js_args);

        /// @brief evaluate a JS script without running it.
        JSValue eval(const std::string &js, const std::string &file_name);

        /// @brief evalute a function from the current global context.
        JSValue eval_function(const std::string &fnName, const std::vector<JSArg> &args);

        /// @brief convert a JSValue into a std::string with the current context.
        std::string val_to_string(JSValue value, bool free = true);

        /// @brief Get a Value from globalThis
        JSValue get_from_global(const std::string &name);

        /// @brief Evalute a function on a class/object.
        JSValue eval_class_function(JSValue obj, const std::string &fn_name, const std::vector<JSArg> &args);

        /// @brief register a callback
        void register_callback(const std::string &fn_name, DynCallback callback);
        
        // /// @brief register a JS class
    };
};
