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
    int add_value(JSValue value);

    /// @brief Free a JSValue
    /// @param id The value id in the map
    void free_value(ejr::EasyJSR* ejsr, int id) {
        if (!ejsr) {
            return;
        }

        auto it = this->id_to_value.find(id);
        if (it != this->id_to_value.end()) {
            ejsr->free_jsval(it->second);
        }
    }

    void free(ejr::EasyJSR* ejsr) {
        if (!ejsr) {
            return;
        }
        
        for (auto& pair : this->id_to_value) {
            ejsr->free_jsval(pair.second);
        }
    }
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
        if (!valid_ptrs(std::vector<void*>{jsvad, handle, handle->instance})) {
            return -1;
        }

        // Conver to string
        std::string fn_name_str = std::string(fn_name);
        
        // TODO: Convert args
        
        return 0;
    }
}