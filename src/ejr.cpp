#include <include/ejr.hpp>
#include <lib/quickjs_cpp_utils.hpp>

using namespace ejr;
using namespace std;
JSValue ejr::__to_js(JSContext *ctx, const _JSArg &arg)
{
    return std::visit([&](auto &&value) -> JSValue
                      {
            using T = std::decay_t<decltype(value)>;
            // } else {
            if constexpr (std::is_same_v<T, int>) {
                return JS_NewInt32(ctx, value);
            } else if constexpr (std::is_same_v<T, long>) {
                return JS_NewInt64(ctx, static_cast<int64_t>(value));
            } else if constexpr (std::is_same_v<T, float>) {
                return JS_NewFloat64(ctx, static_cast<double>(value));
            } else if constexpr (std::is_same_v<T, double>) {
                return JS_NewFloat64(ctx, value);
            } else if constexpr (std::is_same_v<T, bool>) {
                return JS_NewBool(ctx, value);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return JS_NewString(ctx, value.c_str());
            } else {
                return JS_UNDEFINED;
            } }, arg);
}

JSValue ejr::to_js(JSContext *ctx, const JSArg &args)
{
    return std::visit([&](auto &&value) -> JSValue
                      {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, vector<_JSArg>>) {
            JSValue arr = JS_NewArray(ctx);
            for (size_t i = 0; i < value.size(); ++i) {
                JSValue elem = __to_js(ctx, value[i]);
                JS_SetPropertyUint32(ctx, arr, i, elem);
            }
            return arr;
        } else if constexpr (std::is_same_v<T, _JSArg>) {
            return __to_js(ctx, value);
        } }, args);
}

JSArg ejr::from_js(JSContext* ctx, JSValue value) {
    if (JS_IsString(value)) {
        const char* val = JS_ToCString(ctx, value);
        std::string val_string(val);
        JS_FreeCString(ctx, val);
        JS_FreeValue(ctx, value);
        return val_string;
    } 
    else if (JS_IsBool(value)) {
        int b = JS_ToBool(ctx, value);
        JS_FreeValue(ctx, value);
        return static_cast<bool>(b);
    } 
    else if (JS_IsNumber(value)) {
        int32_t i32;
        int64_t i64;
        double d;
        if (JS_ToInt32(ctx, &i32, value) == 0) {
            JS_FreeValue(ctx, value);
            return static_cast<int>(i32);
        } else if (JS_ToInt64(ctx, &i64, value) == 0) {
            JS_FreeValue(ctx, value);
            return static_cast<long>(i64);
        } else if (JS_ToFloat64(ctx, &d, value) == 0) {
            JS_FreeValue(ctx, value);
            return d;
        }
        JS_FreeValue(ctx, value);
        return 0; // fallback
    } 
    else if (JS_IsArray(ctx, value)) {
        uint32_t len;
        JSValue len_val = JS_GetPropertyStr(ctx, value, "length");
        JS_ToUint32(ctx, &len, len_val);
        JS_FreeValue(ctx, len_val);

        std::vector<_JSArg> vec;
        vec.reserve(len);

        for (uint32_t i = 0; i < len; i++) {
            JSValue elem = JS_GetPropertyUint32(ctx, value, i);
            JSArg arg = from_js(ctx, elem);
            if (std::holds_alternative<_JSArg>(arg)) {
                vec.push_back(std::get<_JSArg>(arg));
            }
        }

        JS_FreeValue(ctx, value);
        return vec;
    } 
    else if (JS_IsNull(value) || JS_IsUndefined(value)) {
        JS_FreeValue(ctx, value);
        return std::string("null"); // or maybe return false/0 depending on your design
    }

    // Fallback: return undefined as string
    JS_FreeValue(ctx, value);
    return std::string("[unsupported]");
}

EasyJSR::EasyJSR()
{
    this->runtime = JS_NewRuntime();
    if (!this->runtime)
    {
        // We could not get a runtime
        return;
    }

    this->ctx = JS_NewContext(this->runtime);
}

EasyJSR::~EasyJSR()
{
    // Free context first.
    if (this->ctx)
    {
        JS_FreeContext(this->ctx);
        this->ctx = nullptr;
    }
    // Then free runtime.
    if (this->runtime)
    {
        JS_FreeRuntime(this->runtime);
        this->runtime = nullptr;
    }
}

JSValue EasyJSR::run_script(const string &js_script, const string &file_name)
{
    JSValue val = JS_Eval(this->ctx, js_script.c_str(), js_script.size(), file_name.c_str(), JS_EVAL_TYPE_GLOBAL);

    return val;
}

void EasyJSR::free_jsval(JSValue value)
{
    JS_FreeValue(this->ctx, value);
}

void EasyJSR::free_jsvals(const vector<JSValue>& values) {
    for (auto& val : values) {
        this->free_jsval(val);
    }
}

JSValue EasyJSR::eval(const string &js_script, const string &file_name)
{
    return JS_Eval(this->ctx, js_script.c_str(), js_script.size(), file_name.c_str(), JS_EVAL_FLAG_COMPILE_ONLY);
}

JSValue EasyJSR::eval_function(const string &fnName, const vector<JSArg> &args)
{
    // Get the function
    JSValue global = JS_GetGlobalObject(this->ctx);
    JSValue result = this->eval_class_function(global, fnName, args);
    this->free_jsval(global);

    return result;
}

string EasyJSR::val_to_string(JSValue value, bool free)
{
    const char *c_string;
    if (JS_IsString(value))
    {
        c_string = JS_ToCString(this->ctx, value);
    }
    else
    {
        JSValue js_string = JS_ToString(this->ctx, value);
        c_string = JS_ToCString(this->ctx, js_string);
        this->free_jsval(js_string);
    }

    if (free)
    {
        this->free_jsval(value);
    }
    string value_string = string(c_string);
    JS_FreeCString(this->ctx, c_string);

    return value_string;
}

JSValue EasyJSR::get_from_global(const string& name) {
    JSValue global = JS_GetGlobalObject(this->ctx);
    JSValue val = JS_GetPropertyStr(this->ctx, global, name.c_str());

    this->free_jsval(global);

    return val;
}

JSValue EasyJSR::eval_class_function(JSValue class_obj, const string& fn_name, const vector<JSArg>& args) {
    JSValue function = JS_GetPropertyStr(this->ctx, class_obj, fn_name.c_str());
    vector<JSValue> js_args;

    for (auto arg : args)
    {
        js_args.push_back(to_js(this->ctx, arg));
    }

    JSValue result = JS_Call(this->ctx, function, class_obj, js_args.size(), js_args.data());

    this->free_jsval(function);
    this->free_jsvals(js_args);

    return result;
}

void EasyJSR::register_callback(const string &fn_name, DynCallback callback) {
    this->callbacks[fn_name] = callback;

    // Create JS function bound to callback
    auto trampoline = [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) -> JSValue {
        // Recover EasyJSR* from int64
        int64_t ptr_val;
        JS_ToInt64(ctx, &ptr_val, func_data[0]);
        EasyJSR* self = reinterpret_cast<EasyJSR*>(ptr_val);

        // Convert JS args -> std::vector<JSArg>
        std::vector<JSArg> cpp_args;
        for (int i = 0; i < argc; i++) {
            cpp_args.push_back(from_js(ctx, argv[i])); // you already have `jsvalue_to_jsarg`
        }

        // Lookup callback by magic
        const char* cbname_cstr = JS_ToCString(ctx, func_data[1]);
        std::string cb_name(cbname_cstr);
        JS_FreeCString(ctx, cbname_cstr);
        
        auto it = self->callbacks.find(cb_name);
        if (it == self->callbacks.end()) {
            return js_undefined();
        }

        // Call C++ callback
        JSArg result = it->second(cpp_args);

        // Convert result back to JS
        return to_js(ctx, result);
    };

    // Store EasyJSR* and callback name inside function object
    JSValue global = JS_GetGlobalObject(this->ctx);

    JSValue func_data[2];
    func_data[0] = JS_NewInt64(this->ctx, reinterpret_cast<int64_t>(this));
    func_data[1] = JS_NewString(this->ctx, fn_name.c_str());

    JSValue fn = JS_NewCFunctionData(this->ctx, trampoline, 0, 0, 2, func_data);

    // Free func_data
    this->free_jsvals(vector<JSValue>{func_data[0], func_data[1]});

    JS_SetPropertyStr(this->ctx, global, fn_name.c_str(), fn);
    this->free_jsval(global);
}