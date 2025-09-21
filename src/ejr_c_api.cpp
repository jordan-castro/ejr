#include "ejr_c_api.h"
#include <include/ejr.hpp>

struct EasyJSRHandle {
    /// @brief the EasyJSR instance.
    ejr::EasyJSR* instance;

    EasyJSRHandle(ejr::EasyJSR* instance) : instance(instance) {}
};

struct JSValueAD {
    /// @brief id -> value map
    std::unordered_map<int, JSValue> id_to_value;

    /// @brief Add a new value.
    /// @param value the value.
    /// @return the values id in the map.
    int add_value(JSValue value) {
        int id = this->next_id;
        this->id_to_value[id] = value;
        this->next_id++;
        return id;
    }

    /// @brief Free a JSValue
    /// @param id The value id in the map
    void free_value(ejr::EasyJSR* ejsr, int id) {
        if (!ejsr) {
            return;
        }

        auto it = this->id_to_value.find(id);
        if (it != this->id_to_value.end()) {
            ejsr->free_jsval(it->second);
            this->id_to_value.erase(id);
        }
    }

    void free(ejr::EasyJSR* ejsr) {
        if (!ejsr) {
            return;
        }
        
        for (auto& pair : this->id_to_value) {
            ejsr->free_jsval(pair.second);
        }

        this->id_to_value.clear();
        this->next_id = 0;
    }

    JSValue get(int id) {
        auto it = this->id_to_value.find(id);
        if (it != this->id_to_value.end()) {
            return it->second;
        }

        return js_undefined();
    }

    private:
        /// @brief Next id
        int next_id = 0;
};

/// @brief Make sure all pointers are valid.
/// @param ptrs pointers
/// @return true if valid, false if not.
bool valid_ptrs(const std::vector<void*>& ptrs) {
    for (const auto& ptr : ptrs) {
        if (ptr == nullptr) {
            return false;
        }
    }

    return true;
}



/// @brief Convert JSArg to ejr::JSArg
/// @param arg the JSArg
/// @return the ejr::JSArg
ejr::JSArg jsarg_to_ejr(JSArg arg) {
    // Check type
    switch (arg.type) {
        case JSARG_TYPE_INT: {
            return arg.value.int_val;
        }
        case JSARG_TYPE_DOUBLE: {
            return arg.value.double_val;
        }
        case JSARG_TYPE_FLOAT: {
            return arg.value.float_val;
        }
        case JSARG_TYPE_STRING: {
            return std::string(arg.value.str_val);
        }
        case JSARG_TYPE_BOOL: {
            return arg.value.bool_val;
        }
        case JSARG_TYPE_INT64_T: {
            return arg.value.int64_t_val;
        }
        case JSARG_TYPE_UINT32_T: {
            return arg.value.uint32_t_val;
        }
        case JSARG_TYPE_C_ARRAY: {
            std::vector<ejr::_JSArg> args;

            if (!arg.value.c_array_val.items) {
                return args;
            }

            for (size_t i = 0; i < arg.value.c_array_val.count; ++i) {
                ejr::JSArg new_arg = jsarg_to_ejr(arg.value.c_array_val.items[i]);
                args.push_back(std::get<ejr::_JSArg>(new_arg));
            }

            return args;
        }
        case JSARG_TYPE_NULL: {
            return ejr::JSArgNull{};
        }
        default: {
            // Default is a Null. I don't like undefined in JS...
            return ejr::JSArgNull{};
        }
    }
}

JSArg ejr_to_jsarg(ejr::JSArg ejr_arg) {
    JSArg arg;

    std::visit([&](auto &&value) -> JSValue {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            arg.type = JSARG_TYPE_INT;
            arg.value.int_val = value;
        } else if constexpr (std::is_same_v<T, float>) {
            arg.type = JSARG_TYPE_FLOAT;
            arg.value.float_val = value;
        } else if constexpr (std::is_same_v<T, double>) {
            arg.type = JSARG_TYPE_DOUBLE;
            arg.value.double_val = value;
        } else if constexpr (std::is_same_v<T, bool>) {
            arg.type = JSARG_TYPE_BOOL;
            arg.value.bool_val = value;
        } else if constexpr (std::is_same_v<T, std::string>) {
            arg.type = JSARG_TYPE_STRING;
            char* c_str = new char[value.size() + 1];
            std::memcpy(c_str, value.c_str(), value.sizE() + 1);
            arg.value.str_val = c_str;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            arg.type = JSARG_TYPE_INT64_T;
            arg.value.int64_t_val = value;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            arg.type = JSARG_TYPE_UINT32_T;
            arg.value.uint32_t_val = value;
        }
        else {
            arg.type = JSARG_TYPE_NULL; 
        } 
    }, ejr_arg);

    return arg;
}

extern "C" {
    // Constructors
    EasyJSRHandle* ejr_new() {
        ejr::EasyJSR* instance = new ejr::EasyJSR();
        EasyJSRHandle* handle = new EasyJSRHandle(instance);
        return handle;
    }

    JSValueAD* jsvad_new() {
        JSValueAD* jsvad = new JSValueAD();
        return jsvad;
    }

    JSArg jsarg_int(int value) {
        JSArg arg;
        arg.type = JSARG_TYPE_INT;
        arg.value.int_val = value;

        return arg;
    }

    JSArg jsarg_str(const char* value) {
        JSArg arg;
        arg.type = JSARG_TYPE_STRING;
        arg.value.str_val = value;
        
        return arg;
    }
    
    JSArg jsarg_double(double value) {
        JSArg arg;
        arg.type = JSARG_TYPE_DOUBLE;
        arg.value.double_val = value;

        return arg;
    }
    JSArg jsarg_float(float value) {
        JSArg arg;
        arg.type = JSARG_TYPE_FLOAT;
        arg.value.float_val = value;

        return arg;
    }

    JSArg jsarg_int64t(int64_t value) {
        JSArg arg;
        arg.type = JSARG_TYPE_INT64_T;
        arg.value.int64_t_val = value;
        
        return arg;
    }

    JSArg jsarg_uint32t(uint32_t value) {
        JSArg arg;
        arg.type = JSARG_TYPE_UINT32_T;
        arg.value.uint32_t_val;

        return arg;
    }

    JSArg jsarg_carray(size_t count) {
        JSArg arg;
        arg.type = JSARG_TYPE_C_ARRAY;
        arg.value.c_array_val.count = count;

        return arg;
    }

    JSArg jsarg_null() {
        JSArg arg;
        arg.type = JSARG_TYPE_NULL;

        return arg;
    }

    void jsarg_add_value_to_c_array(JSArg* arg, JSArg value) {
        if (!arg) {
            return;
        }

        if (arg->type != JSARG_TYPE_C_ARRAY) {
            return;
        }

        JSArg* items = arg->value.c_array_val.items;
        size_t count = arg->value.c_array_val.count;

        if (!items) {
            return;
        }

        // Find first empty
        for (size_t i = 0; i < count; ++i) {
            if (items[i].type == 0) {
                // Empty
                items[i] = value;
                return;
            } 
        }

        // Not enough space...
        // TODO: some kind of error log
    }

    // Deleters
    void ejr_free(EasyJSRHandle* handle) {
        if (!handle) {
            return;
        }

        if (handle->instance) {
            delete handle->instance;
        }

        delete handle;
    }

    void jsvad_free(JSValueAD* jsvad, EasyJSRHandle* handle) {
        if (!jsvad || !handle || !handle->instance) {
            return;
        }

        jsvad->free(handle->instance);

        delete jsvad;
    }

    // EasyJSR specific

    int ejr_eval_script(JSValueAD* jsvad, EasyJSRHandle* handle, const char* js, const char* file_name) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return -1;
        }

        // Convert to string
        std::string js_str = std::string(js);
        std::string file_name_string = std::string(file_name);
        
        // Call
        JSValue value = handle->instance->eval_script(js_str, file_name_string);
        // Create new result
        int result = jsvad->add_value(value);
        
        return result;
    }

    int ejr_eval_module(JSValueAD* jsvad, EasyJSRHandle* handle, const char* js, const char* file_name) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return -1;
        }

        // Convert to string
        std::string js_str = std::string(js);
        std::string file_name_str = std::string(file_name);

        // Call
        JSValue value = handle->instance->eval_module(js_str, file_name_str);

        return jsvad->add_value(value);
    }

    int ejr_eval_function(JSValueAD* jsvad, EasyJSRHandle* handle, const char* fn_name, JSArg* args, size_t arg_count) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance, args})) {
            return -1;
        }

        // Conver to string
        std::string fn_name_str = std::string(fn_name);

        // Convert args
        ejr::JSArgs ejr_jsargs;
        ejr_jsargs.reserve(arg_count);

        for (size_t i = 0; i < arg_count; ++i) {
            JSArg arg = args[i];
            ejr_jsargs.push_back(jsarg_to_ejr(arg));
        }

        // Call the function
        JSValue value = handle->instance->eval_function(fn_name_str, ejr_jsargs);
        
        return jsvad->add_value(value);
    }

    char* ejr_val_to_string(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return nullptr;
        }

        // Get our value
        JSValue value = jsvad->get(value_id);
        if (JS_IsUndefined(value)) {
            return nullptr;
        }

        // Get our string
        std::string str = handle->instance->val_to_string(value, false).c_str();

        // convert it to c_str
        char* c_str = new char[str.size() + 1];
        std::memcpy(c_str, str.c_str(), str.size() + 1);

        return c_str;
    }

    int ejr_eval_class_function(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id, const char* fn_name, JSArg* args, size_t arg_count) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance, args})) {
            return -1;
        }

        // Convert fn_name to string
        std::string fn_name_str = std::string(fn_name);
        // Get JSValue from jsvad
        JSValue object = jsvad->get(value_id);
        if (JS_IsUndefined(object)) {
            return -1;
        }

        // Convert JSArgs to ejr::JSArg
        ejr::JSArgs jsargs;
        jsargs.reserve(arg_count);
        for (size_t i = 0; i < arg_count; ++i) {
            JSArg arg = args[i];
            jsargs.push_back(jsarg_to_ejr(arg));
        }
        // Call
        JSValue value = handle->instance->eval_class_function(object, fn_name_str, jsargs);
        // Return id
        return jsvad->add_value(value);
    }

    int ejr_get_property_from(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id, const char* property) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return -1;
        }

        // Convert property to string
        std::string property_str = std::string(property);
        // Get JSValue
        JSValue object = jsvad->get(value_id);
        if (JS_IsUndefined(object)) {
            return -1;
        }
        // Call
        JSValue value = handle->instance->get_property_from(object, property_str);
        // Return id
        return jsvad->add_value(value);
    }

    int ejr_get_from_global(JSValueAD* jsvad, EasyJSRHandle* handle, const char* property) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return -1;
        }

        // Convert property to string
        std::string property_str = std::string(property);

        // Call
        JSValue value = handle->instance->get_from_global(property_str);

        return jsvad->add_value(value);
    }

    void ejr_register_callback(EasyJSRHandle* handle, const char* fn_name, C_Callback cb) {
        if (!valid_ptrs(std::vector<void*>{handle, handle->instance})) {
            return;
        }

        // fn_name string
        std::string fn_name_str = std::string(fn_name);

        auto wrapper = [cb](const ejr::JSArgs& args) -> ejr::JSArg {
            // Convert C++ -> C
            std::vector<JSArg> c_args;
            c_args.reserve(args.size());
            for (auto& a: args) {
                c_args.push_back(ejr_to_jsarg(a));
            }

            // Call raw C callback
            JSArg result = cb(c_args.data(), c_args.size());

            // Convert C -> C++
            return jsarg_to_ejr(result);
        };

        // Register in easyjsr
        handle->instance->register_callback(fn_name_str, wrapper);
    }

    void ejr_register_module(EasyJSRHandle* handle, const char* module_name, JSMethod* methods, size_t method_count) {
        if (!valid_ptrs(std::vector<void*>{handle, handle->instance, methods})) {
            return;
        }

        // module_name string
        std::string module_name_str = std::string(module_name);

        // Convert C JSMethod -> C++ ejr::JSMethod
        std::vector<ejr::JSMethod> ejr_methods;
        ejr_methods.reserve(method_count);

        // Create wrappers for the C JSMethods
        for (size_t i = 0; i < method_count; ++i) {
            JSMethod method = methods[i];
            C_Callback cb = method.cb;
            auto wrapper = [cb](const ejr::JSArgs& args) -> ejr::JSArg {
                // Convert C++ -> C
                std::vector<JSArg> c_args;
                c_args.reserve(args.size());
                for (auto& a: args) {
                    c_args.push_back(ejr_to_jsarg(a));
                }

                // Call raw C callback
                JSArg result = cb(c_args.data(), c_args.size());

                // Convert C -> C++
                return jsarg_to_ejr(result);
            };

            ejr_methods.push_back(ejr::JSMethod{
                std::string(method.name),
                wrapper
            });
        }

        // Register module
        handle->instance->register_module(module_name_str, ejr_methods);
    }

    void ejr_free_string(char* c_string) {
        if (!c_string) {
            return;
        }

        delete[] c_string;
    }

    void jsvad_free_jsvalue(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id) {
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return;
        }

        jsvad->free_value(handle->instance, value_id);
    }
}