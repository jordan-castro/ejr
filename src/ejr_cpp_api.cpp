#include <include/ejr.hpp>
#include <lib/quickjs_cpp_utils.hpp>
#include <utility>
#include "utils.hpp"

using namespace ejr;
using namespace std;

// Module loader
static JSModuleDef *js_module_loader(JSContext *ctx, const char *module_name, void *opaque)
{
    // Get easyjsr
    EasyJSR *ejsr = static_cast<EasyJSR *>(opaque);
    // Check for native module
    auto it = ejsr->modules.find(string(module_name));
    if (it != ejsr->modules.end())
    {
        return it->second;
    }
    // TODO: .json support

    // Load the JS file
    string contents = ejsr->load_file(string(module_name));
    // a size of 0 means that there was a error
    if (contents.size() == 0) {
        return nullptr;
    }

    // Compile the module
    JSValue func_val;
    func_val = JS_Eval(ctx, contents.c_str(), contents.size(), module_name, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    // Check exception
    if (JS_IsException(func_val))
    {
        cout << "Error compiling module" << endl;
        return nullptr;
    }

    JSModuleDef *m = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(func_val));
    JS_FreeValue(ctx, func_val);

    return m;
}

JSValue ejr::to_js(JSContext *ctx, const JSArg &arg)
{
    return std::visit([&](auto &&value) -> JSValue
                      { 
            using T = std::decay_t<decltype(value)>;
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
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return JS_NewBigInt64(ctx, value);
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                return JS_NewUint32(ctx, value);
            } else if constexpr (std::is_same_v<T, JSArgNull>) {
                return js_null();
            } else if constexpr (std::is_same_v<T, JSArgUndefined>) {
                return js_undefined();
            } else if constexpr (std::is_same_v<T, std::shared_ptr<JSArgArray>>) {
                JSValue arr = JS_NewArray(ctx);
                for (size_t i = 0; i < value->size(); i++) {
                    JSValue elem = to_js(ctx, (*value)[i]);
                    JS_SetPropertyUint32(ctx, arr, i, elem);
                }
                return arr;
            } 
            else { 
                return js_undefined();
            } }, arg.value);
}

JSArg ejr::from_js(JSContext *ctx, JSValue value, bool force_free)
{
    if (JS_IsString(value))
    {
        const char *val = JS_ToCString(ctx, value);
        std::string val_string(val);
        JS_FreeCString(ctx, val);
        if (force_free)
        {
            JS_FreeValue(ctx, value);
        }
        return val_string;
    }
    else if (JS_IsBool(value))
    {
        int b = JS_ToBool(ctx, value);
        if (force_free)
        {
            JS_FreeValue(ctx, value);
        }
        return static_cast<bool>(b);
    }
    else if (JS_IsNumber(value))
    {
        int32_t i32;
        int64_t i64;
        uint32_t u32;
        double d;
        if (JS_ToInt32(ctx, &i32, value) == 0)
        {
            if (force_free)
            {
                JS_FreeValue(ctx, value);
            }
            return static_cast<int>(i32);
        }
        else if (JS_ToInt64(ctx, &i64, value) == 0)
        {
            if (force_free)
            {
                JS_FreeValue(ctx, value);
            }
            return i64;
        }
        else if (JS_ToFloat64(ctx, &d, value) == 0)
        {
            if (force_free)
            {
                JS_FreeValue(ctx, value);
            }
            return d;
        } else if (JS_ToUint32(ctx, &u32, value) == 0) {
            if (force_free) {
                JS_FreeValue(ctx, value);
            }
            return u32;
        }
        if (force_free)
        {
            JS_FreeValue(ctx, value);
        }
        return 0; // fallback
    }
    else if (JS_IsArray(ctx, value))
    {
        uint32_t len;
        JSValue len_val = JS_GetPropertyStr(ctx, value, "length");
        JS_ToUint32(ctx, &len, len_val);
        JS_FreeValue(ctx, len_val);

        std::vector<JSArg> vec;
        vec.reserve(len);

        for (uint32_t i = 0; i < len; i++)
        {
            JSValue elem = JS_GetPropertyUint32(ctx, value, i);
            JSArg arg = from_js(ctx, elem);
            vec.push_back(arg);
        }

        if (force_free)
        {
            JS_FreeValue(ctx, value);
        }
        return JSArg(std::move(vec));
    }
    else if (JS_IsNull(value))
    {
        if (force_free)
        {
            JS_FreeValue(ctx, value);
        }
        return std::string("null"); // or maybe return false/0 depending on your design
    } else if (JS_IsUndefined(value)) {
        if (force_free) {
            JS_FreeValue(ctx, value);
        }
        return std::string("undefined");
    }

    // Fallback: return [unsupported] as string
    if (force_free)
    {
        JS_FreeValue(ctx, value);
    }
    return std::string("[unsupported]");
}

// TODO: Finish jsarg to string!
string ejr::jsarg_to_str(const JSArg &arg)
{
    return jsarg_as<std::string>(arg);
}

EJRValue::EJRValue(JSContext *ctx, JSValue val)
{
    this->ctx = ctx;
    this->val = val;
}

EJRValue::~EJRValue()
{
    // if (JS_IsLiveObject(this->runtime, this->val)) {
    JS_FreeValue(this->ctx, this->val);
    // }
}

JSValue &EJRValue::get_ref()
{
    return this->val;
}

JSMethod::JSMethod(const string &name, DynCallback callback)
{
    this->name = name;
    this->callback = std::move(callback);
}

EasyJSR::EasyJSR()
{
    this->runtime = JS_NewRuntime();
    if (!this->runtime)
    {
        // We could not get a runtime
        return;
    }

    // Default file loader
    auto flfn = [](const string& val) -> string {
        return "";
    };
    this->file_loader_fn = flfn;

    // Setup module loader
    JS_SetModuleLoaderFunc(this->runtime, nullptr, js_module_loader, static_cast<void *>(this));

    this->ctx = JS_NewContext(this->runtime);
}

EasyJSR::~EasyJSR()
{
    // Free context first.
    if (this->ctx)
    {
        this->callbacks.clear();

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

int EasyJSR::module_init(JSContext *ctx, JSModuleDef *m)
{
    // Get the module name
    JSAtom module_name_atom = JS_GetModuleName(ctx, m);
    const char *module_name = JS_AtomToCString(ctx, module_name_atom);
    string mod = string(module_name);

    JS_FreeAtom(ctx, module_name_atom);
    JS_FreeCString(ctx, module_name);

    // Get pointer and real module name
    vector<string> module_data = str_split(mod, "::");
    string ptr = module_data[0];
    string real_mod_name = module_data[1];

    // Dereference easyjsr
    uintptr_t pval = stoull(ptr, nullptr, 16);
    EasyJSR *ejsr = reinterpret_cast<EasyJSR *>(pval);

    // Now lets add our module methods...
    vector<tuple<string, JSValue>> module_methods = ejsr->methods_by_module[real_mod_name];

    for (auto &method : module_methods)
    {
        // Get method name
        string method_name = std::get<0>(method);
        // Get method value
        JSValue method_value = std::get<1>(method);

        // Set module export
        JS_SetModuleExport(ctx, m, method_name.c_str(), method_value);
    }

    return 0;
}

JSValue EasyJSR::eval_script(const string &js_script, const string &file_name)
{
    JSValue val = this->eval(js_script, file_name, JS_EVAL_TYPE_GLOBAL);

    return val;
}

JSValue EasyJSR::eval_module(const string &js_module, const string &file_name)
{
    JSValue promise = this->eval(js_module, file_name, JS_EVAL_TYPE_MODULE);
    JSValue promise_result = JS_PromiseResult(this->ctx, promise);

    this->free_jsval(promise);
    return promise_result;
}

void EasyJSR::free_jsval(JSValue value)
{
    // Check if value is a JS_Exception
    if (JS_IsException(value)) {
        // No need to free it, let QuickJS handle it.
        return;
    }
    JS_FreeValue(this->ctx, value);
}

void EasyJSR::free_jsvals(const vector<JSValue> &values)
{
    for (auto &val : values)
    {
        this->free_jsval(val);
    }
}

JSValue EasyJSR::eval(const string &js_script, const string &file_name, int eval_flags)
{
    return JS_Eval(this->ctx, js_script.c_str(), js_script.size(), file_name.c_str(), eval_flags);
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
    // Clean val first
    tuple<JSValue, bool> cleaned = this->clean_js_value(value);
    JSValue cleaned_value = std::get<0>(cleaned);

    const char *c_string;
    if (JS_IsString(cleaned_value))
    {
        c_string = JS_ToCString(this->ctx, cleaned_value);
    }
    else
    {
        JSValue js_string = JS_ToString(this->ctx, cleaned_value);
        c_string = JS_ToCString(this->ctx, js_string);

        this->free_jsval(js_string);
    }

    // Always free Exception...
    if (std::get<1>(cleaned))
    {
        this->free_jsval(cleaned_value);
    }

    if (free)
    {
        this->free_jsval(value);
    }
    string value_string = string(c_string);
    JS_FreeCString(this->ctx, c_string);

    return value_string;
}

JSValue EasyJSR::get_from_global(const string &name)
{
    JSValue global = JS_GetGlobalObject(this->ctx);
    JSValue val = JS_GetPropertyStr(this->ctx, global, name.c_str());

    this->free_jsval(global);

    return val;
}

JSValue EasyJSR::eval_class_function(JSValue class_obj, const string &fn_name, const vector<JSArg> &args)
{
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

void EasyJSR::register_callback(const string &fn_name, DynCallback callback)
{
    JSValue global = JS_GetGlobalObject(this->ctx);

    auto fn = this->create_trampoline(fn_name, callback);

    JS_SetPropertyStr(this->ctx, global, fn_name.c_str(), fn);
    this->free_jsval(global);
}

void EasyJSR::register_module(const string &module_name, const vector<JSMethod> &methods)
{
    // Check if module_name already exists
    if (this->modules.find(module_name) != this->modules.end())
    {
        // TODO: logs
        cout << "Module: " << module_name << " already registered." << endl;
    }

    // Mangled with ptr module name
    ostringstream oss;
    oss << this;
    oss << "::" << module_name;
    string mangled_with_ptr_name = oss.str();

    JSModuleDef *m = JS_NewCModule(this->ctx, mangled_with_ptr_name.c_str(), EasyJSR::module_init);

    vector<tuple<string, JSValue>> module_methods;

    // Create the trampolines
    for (auto &method : methods)
    {
        // Mangle the callback name in EasyJSR.
        string cb_name = "__" + module_name + "_" + method.name;
        string method_name = method.name;
        JS_AddModuleExport(this->ctx, m, method.name.c_str());
        // Create actual method in EJR, and just keep the JSValue alive for now...
        auto fn = this->create_trampoline(cb_name, method.callback);
        module_methods.push_back(make_tuple(method_name, fn));
    }

    this->methods_by_module[module_name] = module_methods;
    this->modules[module_name] = m;
}

tuple<JSValue, bool> EasyJSR::clean_js_value(JSValue val)
{
    if (JS_IsException(val))
    {
        JSValue js_exception = JS_GetException(this->ctx);

        // User has to free this result manunally
        return make_tuple(js_exception, true);
    }
    else
    {
        return make_tuple(val, false);
    }
}

EJRValue EasyJSR::wrap_js_val(JSValue val)
{
    return EJRValue(this->ctx, val);
}

JSValue EasyJSR::get_property_from(JSValue this_obj, string property)
{
    return JS_GetPropertyStr(this->ctx, this_obj, property.c_str());
}

JSValue EasyJSR::create_trampoline(const string &cb_name, DynCallback cb)
{
    this->callbacks[cb_name] = std::move(cb);

    // Create JS function bound to callback
    auto trampoline = [](JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, int magic, JSValue *func_data) -> JSValue
    {
        int64_t address_int64;
        JS_ToBigInt64(ctx, &address_int64, func_data[0]);
        uintptr_t address = static_cast<uintptr_t>(address_int64);
        EasyJSR *self = reinterpret_cast<EasyJSR *>(address);

        // Convert JS args -> std::vector<JSArg>
        std::vector<JSArg> cpp_args;
        for (int i = 0; i < argc; i++)
        {
            cpp_args.push_back(from_js(ctx, argv[i], false));
        }

        // Lookup callback by magic
        const char *cbname_cstr = JS_ToCString(ctx, func_data[1]);
        std::string cb_name(cbname_cstr);
        JS_FreeCString(ctx, cbname_cstr);

        auto it = self->callbacks.find(cb_name);
        if (it == self->callbacks.end())
        {
            return js_undefined();
        }

        // Call C++ callback
        JSArg result = it->second(cpp_args);

        // Convert result back to JS
        return to_js(ctx, result);
    };
    // Store EasyJSR* and callback name inside function object
    JSValue func_data[2];
    uintptr_t address = reinterpret_cast<uintptr_t>(this);
    int64_t address_int64 = static_cast<int64_t>(address);
    func_data[0] = JS_NewBigInt64(this->ctx, address_int64);
    func_data[1] = JS_NewString(this->ctx, cb_name.c_str());

    JSValue fn = JS_NewCFunctionData(this->ctx, trampoline, 0, 0, 2, func_data);

    return fn;
}

void EasyJSR::set_file_loader(FileLoaderFn loader_fn) {
    // Just set and viola
    this->file_loader_fn = std::move(loader_fn);
}

string EasyJSR::load_file(const string& file_path) {
    // Call file_loader
    return this->file_loader_fn(file_path);
}

JSArg EasyJSR::jsvalue_to_jsarg(JSValue value, bool force_free) {
    return from_js(this->ctx, value, force_free);
}