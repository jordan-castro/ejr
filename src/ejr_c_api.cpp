#include <include/ejr.h>
#include <include/ejr.hpp>
#include "src/utils.hpp"
#include <memory>
#include <string>
#include <cstdlib>
#include <cstring>
#include "ejr.h"

struct JSValueAD
{
    /// @brief id -> value map
    std::unordered_map<int, JSValue> id_to_value;

    /// @brief Add a new value.
    /// @param value the value.
    /// @return the values id in the map.
    int add_value(JSValue value)
    {
        int id = this->next_id;
        this->id_to_value[id] = value;
        this->next_id++;
        return id;
    }

    /// @brief Free a JSValue
    /// @param id The value id in the map
    void free_value(ejr::EasyJSR *ejsr, int id)
    {
        if (!ejsr)
        {
            return;
        }

        auto it = this->id_to_value.find(id);
        if (it != this->id_to_value.end())
        {
            ejsr->free_jsval(it->second);
            this->id_to_value.erase(id);
        }
    }

    void free(ejr::EasyJSR *ejsr)
    {
        if (!ejsr)
        {
            return;
        }

        for (auto &pair : this->id_to_value)
        {
            ejsr->free_jsval(pair.second);
        }

        this->id_to_value.clear();
        this->next_id = 0;
    }

    JSValue get(int id)
    {
        auto it = this->id_to_value.find(id);
        if (it != this->id_to_value.end())
        {
            return it->second;
        }

        return js_undefined();
    }

private:
    /// @brief Next id
    int next_id = 0;
};

struct EasyJSRHandle
{
    /// @brief the EasyJSR instance.
    ejr::EasyJSR *instance;

    /// @brief the JSVAD
    JSValueAD *jsvad;

    EasyJSRHandle(ejr::EasyJSR *instance, JSValueAD *jsvad) : instance(instance), jsvad(jsvad) {}
};

/// @brief Make sure all pointers are valid.
/// @param ptrs pointers
/// @return true if valid, false if not.
bool valid_ptrs(const std::vector<void *> &ptrs)
{
    for (const auto &ptr : ptrs)
    {
        if (ptr == nullptr)
        {
            return false;
        }
    }

    return true;
}

/// @brief Convert JSArg to ejr::JSArg
/// @param arg the JSArg
/// @return the ejr::JSArg
ejr::JSArg jsarg_to_ejr(JSArg arg)
{
    // Check type
    switch (arg.type)
    {
    case JSARG_TYPE_INT:
    {
        return arg.value.int_val;
    }
    case JSARG_TYPE_DOUBLE:
    {
        return arg.value.double_val;
    }
    case JSARG_TYPE_FLOAT:
    {
        return arg.value.float_val;
    }
    case JSARG_TYPE_STRING:
    {
        return std::string(arg.value.str_val);
    }
    case JSARG_TYPE_BOOL:
    {
        return arg.value.bool_val;
    }
    case JSARG_TYPE_INT64_T:
    {
        return arg.value.int64_t_val;
    }
    case JSARG_TYPE_UINT32_T:
    {
        return arg.value.uint32_t_val;
    }
    case JSARG_TYPE_C_ARRAY:
    {
        std::vector<ejr::JSArg> args;

        if (!arg.value.c_array_val.items)
        {
            return ejr::JSArg(std::move(args));
        }

        for (size_t i = 0; i < arg.value.c_array_val.count; ++i)
        {
            ejr::JSArg new_arg = jsarg_to_ejr(*arg.value.c_array_val.items[i]);
            args.push_back(new_arg);
        }

        return ejr::JSArg(std::move(args));
    }
    case JSARG_TYPE_NULL:
    {
        return ejr::JSArg(nullptr);
    }
    case JSARG_TYPE_UNDEFINED:
    {
        return ejr::JSArg(std::monostate());
    }
    case JSARG_TYPE_UINT8_ARRAY:
    {
        if (!arg.value.u8_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<uint8_t>(std::move(std::vector<uint8_t>{})));
        }
        std::vector<uint8_t> u8_values(arg.value.u8_array_val.items, arg.value.u8_array_val.items + arg.value.u8_array_val.count);

        return ejr::JSArg(ejr::JSArgTypedArray<uint8_t>(std::move(u8_values)));
    }
    case JSARG_TYPE_INT32_ARRAY:
    {
        if (!arg.value.i32_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<int32_t>(std::move(std::vector<int32_t>{})));
        }
        std::vector<int32_t> i32_values(arg.value.i32_array_val.items, arg.value.i32_array_val.items + arg.value.i32_array_val.count);

        return ejr::JSArg(ejr::JSArgTypedArray<int32_t>(std::move(i32_values)));
    }
    case JSARG_TYPE_UINT32_ARRAY:
    {
        if (!arg.value.u32_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<uint32_t>(std::move(std::vector<uint32_t>{})));
        }
        std::vector<uint32_t> u32_values(arg.value.u32_array_val.items, arg.value.u32_array_val.items + arg.value.u32_array_val.count);

        return ejr::JSArg(ejr::JSArgTypedArray<uint32_t>(std::move(u32_values)));
    }
    case JSARG_TYPE_INT64_ARRAY:
    {
        if (!arg.value.i64_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<int64_t>(std::vector<int64_t>{}));
        }
        std::vector<int64_t> i64_values(arg.value.i64_array_val.items,
                                        arg.value.i64_array_val.items + arg.value.i64_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<int64_t>(std::move(i64_values)));
    }
    case JSARG_TYPE_INT8_ARRAY:
    {
        if (!arg.value.i8_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<int8_t>(std::vector<int8_t>{}));
        }
        std::vector<int8_t> i8_values(arg.value.i8_array_val.items,
                                      arg.value.i8_array_val.items + arg.value.i8_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<int8_t>(std::move(i8_values)));
    }
    case JSARG_TYPE_INT16_ARRAY:
    {
        if (!arg.value.i16_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<int16_t>(std::vector<int16_t>{}));
        }
        std::vector<int16_t> i16_values(arg.value.i16_array_val.items,
                                        arg.value.i16_array_val.items + arg.value.i16_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<int16_t>(std::move(i16_values)));
    }
    case JSARG_TYPE_UINT16_ARRAY:
    {
        if (!arg.value.u16_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<uint16_t>(std::vector<uint16_t>{}));
        }
        std::vector<uint16_t> u16_values(arg.value.u16_array_val.items,
                                         arg.value.u16_array_val.items + arg.value.u16_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<uint16_t>(std::move(u16_values)));
    }
    case JSARG_TYPE_UINT64_ARRAY:
    {
        if (!arg.value.u64_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<uint64_t>(std::vector<uint64_t>{}));
        }
        std::vector<uint64_t> u64_values(arg.value.u64_array_val.items,
                                         arg.value.u64_array_val.items + arg.value.u64_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<uint64_t>(std::move(u64_values)));
    }
    case JSARG_TYPE_FLOAT_ARRAY:
    {
        if (!arg.value.float_array_val.items)
        {
            return ejr::JSArg(ejr::JSArgTypedArray<float>(std::vector<float>{}));
        }
        std::vector<float> f32_values(arg.value.float_array_val.items,
                                      arg.value.float_array_val.items + arg.value.float_array_val.count);
        return ejr::JSArg(ejr::JSArgTypedArray<float>(std::move(f32_values)));
    }
    case JSARG_TYPE_EXCEPTION:
    {
        std::string message = "Exception";
        std::string name = "Exception";

        if (arg.value.exception_val.name) {
            name = arg.value.exception_val.name;
        }
        if (arg.value.exception_val.msg) {
            message = arg.value.exception_val.msg;
        }

        return ejr::JSArg(ejr::JSArgException(message, name));
    }

    default:
    {
        // Default is a undefined. I don't like undefined in JS...
        return ejr::JSArg(std::monostate());
    }
    }
}

JSArg *ejr_to_jsarg(ejr::JSArg ejr_arg)
{
    JSArg *arg;

    std::visit([&](auto &&value)
               {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            arg = jsarg_int(value);
        } else if constexpr (std::is_same_v<T, float>) {
            arg = jsarg_float(value);
        } else if constexpr (std::is_same_v<T, double>) {
            arg = jsarg_double(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            arg = jsarg_bool(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            char* c_str = new char[value.size() + 1];
            std::memcpy(c_str, value.c_str(), value.size() + 1);
            arg = jsarg_str(c_str);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            arg = jsarg_int64t(value);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            arg = jsarg_uint32t(value);
        } else if constexpr (std::is_same_v<T, ejr::JSArgNull>) {
            arg = jsarg_null();
        } else if constexpr (std::is_same_v<T, ejr::JSArgUndefined>) {
            arg = jsarg_undefined();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<std::vector<ejr::JSArg>>>) {
            arg = jsarg_carray(value->size());
            for (size_t i = 0; i < value->size(); i++) {
                JSArg* i_arg = ejr_to_jsarg((*value)[i]);
                jsarg_add_value_to_c_array(arg, i_arg);
            }
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<uint8_t>>) {
            arg = jsarg_u8_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<int32_t>>) {
            arg = jsarg_i32_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<uint32_t>>) {
            arg = jsarg_u32_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<int64_t>>) {
            arg = jsarg_i64_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<int8_t>>) {
            arg = jsarg_i8_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<int16_t>>) {
            arg = jsarg_i16_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<uint16_t>>) {
            arg = jsarg_u16_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<uint64_t>>) {
            arg = jsarg_u64_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgTypedArray<float>>) {
            arg = jsarg_float_array(value.values.data(), value.values.size());
        } else if constexpr (std::is_same_v<T, ejr::JSArgException>) {
            arg = jsarg_exception(value.msg.c_str(), value.name.c_str());
        }
        else {
            arg = jsarg_null();
        } }, ejr_arg.value);

    return arg;
}

extern "C"
{
    // Constructors
    EasyJSRHandle *ejr_new()
    {
        ejr::EasyJSR *instance = new ejr::EasyJSR();
        JSValueAD *jsvad = new JSValueAD();
        EasyJSRHandle *handle = new EasyJSRHandle(instance, jsvad);
        return handle;
    }

    JSArg *jsarg_int(int value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT;
        arg->value.int_val = value;

        return arg;
    }

    JSArg *jsarg_str(const char *value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_STRING;
        
        char* value_copy = new char[strlen(value) + 1];
        std::memcpy(value_copy, value, strlen(value) + 1);
        
        arg->value.str_val = value_copy;

        return arg;
    }

    JSArg *jsarg_double(double value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_DOUBLE;
        arg->value.double_val = value;

        return arg;
    }
    JSArg *jsarg_float(float value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_FLOAT;
        arg->value.float_val = value;

        return arg;
    }

    JSArg *jsarg_int64t(int64_t value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT64_T;
        arg->value.int64_t_val = value;

        return arg;
    }

    JSArg *jsarg_uint32t(uint32_t value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UINT32_T;
        arg->value.uint32_t_val = value;

        return arg;
    }

    JSArg *jsarg_bool(bool value)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_BOOL;
        arg->value.bool_val = value;

        return arg;
    }

    JSArg *jsarg_carray(size_t count)
    {
        JSArg *arg = new JSArg;
        arg->type = JSARG_TYPE_C_ARRAY;
        arg->value.c_array_val.capacity = count;
        arg->value.c_array_val.count = 0;
        arg->value.c_array_val.items = new JSArg *[count]{nullptr};

        return arg;
    }

    JSArg *jsarg_null()
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_NULL;

        return arg;
    }

    JSArg *jsarg_undefined()
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UNDEFINED;

        return arg;
    }

    JSArg *jsarg_u8_array(const uint8_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UINT8_ARRAY;
        arg->value.u8_array_val.count = argc;

        // Allocate internal copy
        arg->value.u8_array_val.items = new uint8_t[argc];
        std::memcpy((void*)arg->value.u8_array_val.items, args, argc * sizeof(uint8_t));

        return arg;
    }

    JSArg *jsarg_i32_array(const int32_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT32_ARRAY;
        arg->value.i32_array_val.count = argc;

        // Allocate internal copy
        arg->value.i32_array_val.items = new int32_t[argc];
        std::memcpy((void*)arg->value.i32_array_val.items, args, argc * sizeof(int32_t));

        return arg;
    }

    JSArg *jsarg_u32_array(const uint32_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UINT32_ARRAY;
        arg->value.u32_array_val.count = argc;

        // Allocate internal copy
        arg->value.u32_array_val.items = new uint32_t[argc];
        std::memcpy((void*)arg->value.u32_array_val.items, args, argc * sizeof(uint32_t));

        return arg;
    }

    JSArg *jsarg_i64_array(const int64_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT64_ARRAY;
        arg->value.i64_array_val.count = argc;

        // Allocate internal copy
        arg->value.i64_array_val.items = new int64_t[argc];
        std::memcpy((void*)arg->value.i64_array_val.items, args, argc * sizeof(int64_t));

        return arg;
    }

    JSArg *jsarg_i8_array(const int8_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT8_ARRAY;
        arg->value.i8_array_val.count = argc;

        // Allocate internal copy
        arg->value.i8_array_val.items = new int8_t[argc];
        std::memcpy((void*)arg->value.i8_array_val.items, args, argc * sizeof(int8_t));

        return arg;
    }

    JSArg *jsarg_i16_array(const int16_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_INT16_ARRAY;
        arg->value.i16_array_val.count = argc;

        // Allocate internal copy
        arg->value.i16_array_val.items = new int16_t[argc];
        std::memcpy((void*)arg->value.i16_array_val.items, args, argc * sizeof(int16_t));

        return arg;
    }

    JSArg *jsarg_u16_array(const uint16_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UINT16_ARRAY;
        arg->value.u16_array_val.count = argc;

        // Allocate internal copy
        arg->value.u16_array_val.items = new uint16_t[argc];
        std::memcpy((void*)arg->value.u16_array_val.items, args, argc * sizeof(uint16_t));

        return arg;
    }

    JSArg *jsarg_u64_array(const uint64_t *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_UINT64_ARRAY;
        arg->value.u64_array_val.count = argc;

        // Allocate internal copy
        arg->value.u64_array_val.items = new uint64_t[argc];
        std::memcpy((void*)arg->value.u64_array_val.items, args, argc * sizeof(uint64_t));

        return arg;
    }


    JSArg *jsarg_float_array(const float *args, size_t argc)
    {
        JSArg *arg = new JSArg();
        arg->type = JSARG_TYPE_FLOAT_ARRAY;
        arg->value.float_array_val.count = argc;

        // Allocate internal copy
        arg->value.float_array_val.items = new float[argc];
        std::memcpy((void*)arg->value.float_array_val.items, args, argc * sizeof(float));

        return arg;
    }

    JSArg* jsarg_exception(const char* message, const char* name) {
        JSArg* arg = new JSArg();
        arg->type = JSARG_TYPE_EXCEPTION;

        char* message_copy = new char[strlen(message) + 1]; // nullptr
        char* name_copy = new char[strlen(name) + 1]; // nullptr

        std::memcpy(message_copy, message, strlen(message) + 1);
        std::memcpy(name_copy, name, strlen(name) + 1);

        arg->value.exception_val.msg = message_copy;
        arg->value.exception_val.name = name_copy;

        return arg;
    }
    
    void jsarg_add_value_to_c_array(JSArg *arg, JSArg *value)
    {
        if (!arg || !value)
        {
            return;
        }

        if (arg->type != JSARG_TYPE_C_ARRAY)
        {
            return;
        }

        size_t capacity = arg->value.c_array_val.capacity;
        JSArg **items = arg->value.c_array_val.items;
        size_t count = arg->value.c_array_val.count;

        if (!items)
        {
            return;
        }

        if (count >= capacity) {
            // Not enough space...
            // TODO: some kind of error log OR grow the memory...
            return;
        }

        // Update mem
        items[count] = value;
        arg->value.c_array_val.count++;
    }

    JSArg *jsarg_from_jsvalue(EasyJSRHandle *handle, int value)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}) || value < 0)
        {
            return jsarg_null();
        }

        JSValue jsvalue = handle->jsvad->get(value);

        // Get the jsarg but also free the JSValue since we ain't finna use it later.
        ejr::JSArg ejr_arg = handle->instance->jsvalue_to_jsarg(jsvalue, true);

        return ejr_to_jsarg(ejr_arg);
    }

    JSArg **jsarg_make_list(size_t argc)
    {
        return new JSArg *[argc]
        { ejr_to_jsarg(ejr::JSArg(std::monostate())) };
    }

    void jsarg_free_all(JSArg **args, size_t argc)
    {
        if (args == nullptr)
        {
            return;
        }

        for (size_t i = 0; i < argc; ++i)
        {
            if (args[i] == nullptr)
            {
                continue;
            }
            jsarg_free(args[i]);
        }

        delete[] args;
    }

    void jsarg_add_to_list(JSArg **jsarg, JSArg *njsarg, size_t i)
    {
        if (valid_ptrs(std::vector<void *>{jsarg, njsarg}))
        {
            return;
        }

        if (jsarg[i] == nullptr)
        {
            return;
        }

        // Add njsarg
        jsarg[i] = njsarg;
    }

    // Deleters
    void ejr_free(EasyJSRHandle *handle)
    {
        if (!handle)
        {
            return;
        }

        // Free jsvad first
        if (handle->jsvad)
        {
            handle->jsvad->free(handle->instance);
            delete handle->jsvad;
        }

        if (handle->instance)
        {
            delete handle->instance;
        }

        delete handle;
    }

    void jsarg_free(JSArg *arg)
    {
        if (!arg)
        {
            return;
        }
        switch (arg->type)
        {
        case JSARG_TYPE_STRING:
            delete[] arg->value.str_val;
            break;

        case JSARG_TYPE_C_ARRAY:
            if (arg->value.c_array_val.items)
            {
                for (size_t i = 0; i < arg->value.c_array_val.count; ++i)
                {
                    jsarg_free(arg->value.c_array_val.items[i]);
                }
                delete[] arg->value.c_array_val.items;
            }
            break;
        case JSARG_TYPE_UINT8_ARRAY:
            if (arg->value.u8_array_val.items)
            {
                delete[] arg->value.u8_array_val.items;
            }
            break;
        case JSARG_TYPE_INT32_ARRAY:
            if (arg->value.i32_array_val.items)
            {
                delete[] arg->value.i32_array_val.items;
            }
            break;
        case JSARG_TYPE_UINT32_ARRAY:
            if (arg->value.u32_array_val.items)
            {
                delete[] arg->value.u32_array_val.items;
            }
            break;
        case JSARG_TYPE_INT64_ARRAY:
            if (arg->value.i64_array_val.items)
            {
                delete[] arg->value.i64_array_val.items;
            }
            break;
        case JSARG_TYPE_INT8_ARRAY:
            if (arg->value.i8_array_val.items)
            {
                delete[] arg->value.i8_array_val.items;
            }
            break;
        case JSARG_TYPE_UINT16_ARRAY:
            if (arg->value.u16_array_val.items)
            {
                delete[] arg->value.u16_array_val.items;
            }
            break;
        case JSARG_TYPE_INT16_ARRAY:
            if (arg->value.i16_array_val.items)
            {
                delete[] arg->value.i16_array_val.items;
            }
            break;
        case JSARG_TYPE_UINT64_ARRAY:
            if (arg->value.u64_array_val.items)
            {
                delete[] arg->value.u64_array_val.items;
            }
            break;
        case JSARG_TYPE_FLOAT_ARRAY:
            if (arg->value.float_array_val.items)
            {
                delete[] arg->value.float_array_val.items;
            }
            break;

        default:
            break;
        }

        delete arg;
    }

    // EasyJSR specific

    void ejr_set_file_loader(EasyJSRHandle *handle, C_FileLoaderFn fn, void *opaque)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->instance}))
        {
            return;
        }

        auto wrapper = [fn, opaque](const std::string &file_path) -> std::string
        {
            // Convert C++ -> C
            const char *file_path_c_str = file_path.c_str();

            // Call raw C
            char *result = fn(file_path_c_str, opaque);

            // Convert C -> C++
            std::string result_str(result);

            return result_str;
        };

        // Register in ejr
        handle->instance->set_file_loader(wrapper);
    }

    int ejr_eval_script(EasyJSRHandle *handle, const char *js, const char *file_name)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return -1;
        }

        // Convert to string
        std::string js_str = std::string(js);
        std::string file_name_string = std::string(file_name);

        // Call
        JSValue value = handle->instance->eval_script(js_str, file_name_string);
        // Create new result
        int result = handle->jsvad->add_value(value);

        return result;
    }

    int ejr_eval_module(EasyJSRHandle *handle, const char *js, const char *file_name)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return -1;
        }

        // Convert to string
        std::string js_str = std::string(js);
        std::string file_name_str = std::string(file_name);

        // Call
        JSValue value = handle->instance->eval_module(js_str, file_name_str);

        // Check if value is not a promise
        if (JS_IsException(value)) {
            return handle->jsvad->add_value(value);
        }

        JSValue promise_result = handle->instance->await_promise(value);

        // Free value (We might not need this actually... becuase quickjs might free the value themselves.) 
        handle->instance->free_jsval(value);

        return handle->jsvad->add_value(promise_result);
    }

    int ejr_eval_function(EasyJSRHandle *handle, const char *fn_name, JSArg **args, size_t arg_count)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance, args}))
        {
            return -1;
        }

        // Conver to string
        std::string fn_name_str = std::string(fn_name);

        // Convert args
        ejr::JSArgs ejr_jsargs;
        ejr_jsargs.reserve(arg_count);

        for (size_t i = 0; i < arg_count; ++i)
        {
            JSArg arg = *args[i];
            ejr_jsargs.push_back(jsarg_to_ejr(arg));
        }

        // Call the function
        JSValue value = handle->instance->eval_function(fn_name_str, ejr_jsargs);

        // Free input args
        jsarg_free_all(args, arg_count);

        return handle->jsvad->add_value(value);
    }

    char *ejr_val_to_string(EasyJSRHandle *handle, int value_id)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return nullptr;
        }

        // Get our value
        JSValue value = handle->jsvad->get(value_id);
        if (JS_IsUndefined(value))
        {
            return nullptr;
        }

        // Get our string
        std::string str = handle->instance->val_to_string(value, false).c_str();

        // convert it to c_str
        char *c_str = new char[str.size() + 1];
        std::memcpy(c_str, str.c_str(), str.size() + 1);

        return c_str;
    }

    int ejr_eval_class_function(EasyJSRHandle *handle, int value_id, const char *fn_name, JSArg **args, size_t arg_count)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance, args}))
        {
            return -1;
        }

        // Convert fn_name to string
        std::string fn_name_str = std::string(fn_name);
        // Get JSValue from jsvad
        JSValue object = handle->jsvad->get(value_id);
        if (JS_IsUndefined(object))
        {
            return -1;
        }

        // Convert JSArgs to ejr::JSArg
        ejr::JSArgs jsargs;
        jsargs.reserve(arg_count);
        for (size_t i = 0; i < arg_count; ++i)
        {
            JSArg arg = *args[i];
            jsargs.push_back(jsarg_to_ejr(arg));
        }
        // Call
        JSValue value = handle->instance->eval_class_function(object, fn_name_str, jsargs);

        // Free input args
        jsarg_free_all(args, arg_count);

        // Return id
        return handle->jsvad->add_value(value);
    }

    int ejr_get_property_from(EasyJSRHandle *handle, int value_id, const char *property)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return -1;
        }

        // Convert property to string
        std::string property_str = std::string(property);
        // Get JSValue
        JSValue object = handle->jsvad->get(value_id);
        if (JS_IsUndefined(object))
        {
            return -1;
        }
        // Call
        JSValue value = handle->instance->get_property_from(object, property_str);
        // Return id
        return handle->jsvad->add_value(value);
    }

    int ejr_get_from_global(EasyJSRHandle *handle, const char *property)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return -1;
        }

        // Convert property to string
        std::string property_str = std::string(property);

        // Call
        JSValue value = handle->instance->get_from_global(property_str);

        return handle->jsvad->add_value(value);
    }

    void ejr_register_callback(EasyJSRHandle *handle, const char *fn_name, C_Callback cb, void *opaque)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->instance}))
        {
            return;
        }

        // fn_name string
        std::string fn_name_str = std::string(fn_name);

        auto wrapper = [cb, opaque](const ejr::JSArgs &args) -> ejr::JSArg
        {
            // Convert C++ -> C
            std::vector<JSArg *> c_args;
            c_args.reserve(args.size());
            for (auto &a : args)
            {
                c_args.push_back(ejr_to_jsarg(a));
            }

            // Call raw C callback
            JSArg *result = cb(c_args.data(), c_args.size(), opaque);

            // Convert C -> C++
            ejr::JSArg res = jsarg_to_ejr(*result);

            // Free JSArg
            jsarg_free(result);

            // Free c_args
            for (auto &arg : c_args)
            {
                jsarg_free(arg);
            }

            // This goes to C++
            return res;
        };

        // Register in easyjsr
        handle->instance->register_callback(fn_name_str, wrapper);
    }

    void ejr_register_module(EasyJSRHandle *handle, const char *module_name, JSMethod *methods, size_t method_count)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->instance, methods}))
        {
            return;
        }

        // module_name string
        std::string module_name_str = std::string(module_name);

        // Convert C JSMethod -> C++ ejr::JSMethod
        std::vector<ejr::JSMethod> ejr_methods;
        ejr_methods.reserve(method_count);

        // Create wrappers for the C JSMethods
        for (size_t i = 0; i < method_count; ++i)
        {
            JSMethod method = methods[i];
            C_Callback cb = method.cb;
            void *opaque = method.opaque;
            auto wrapper = [cb, opaque](const ejr::JSArgs &args) -> ejr::JSArg
            {
                // Convert C++ -> C
                std::vector<JSArg *> c_args;
                c_args.reserve(args.size());
                for (auto &a : args)
                {
                    c_args.push_back(ejr_to_jsarg(a));
                }

                // Call raw C callback
                JSArg *result = cb(c_args.data(), c_args.size(), opaque);

                // Convert C -> C++
                ejr::JSArg res = jsarg_to_ejr(*result);

                jsarg_free(result);
                // Free c_args
                for (auto &arg : c_args)
                {
                    jsarg_free(arg);
                }

                // This goes to c++
                return res;
            };

            ejr_methods.push_back(ejr::JSMethod{
                std::string(method.name),
                wrapper});
        }

        // Register module
        handle->instance->register_module(module_name_str, ejr_methods);
    }

    void ejr_free_string(char *c_string)
    {
        if (!c_string)
        {
            return;
        }

        delete[] c_string;
    }

    void ejr_free_jsvalue(EasyJSRHandle *handle, int value_id)
    {
        if (!valid_ptrs(std::vector<void *>{handle, handle->jsvad, handle->instance}))
        {
            return;
        }

        handle->jsvad->free_value(handle->instance, value_id);
    }

    int ejr_await_promise(EasyJSRHandle* handle, int value_id) {
        if (handle == nullptr) {
            return -1;
        }

        JSValue promise = handle->jsvad->get(value_id);
        JSValue promise_result = handle->instance->await_promise(promise);

        return handle->jsvad->add_value(promise_result);
    }

    char* jsarg_to_string(JSArg* arg) {
        if (arg == nullptr) {
            return nullptr;
        }
        std::string str = "";
        
        switch(arg->type) {
        case JSARG_TYPE_INT:
            str = std::to_string(arg->value.int_val);
            break;
        case JSARG_TYPE_DOUBLE:
            str = std::to_string(arg->value.double_val);
            break;
        case JSARG_TYPE_FLOAT:
            str = std::to_string(arg->value.float_val);
            break;
        case JSARG_TYPE_BOOL:
            str = std::to_string(arg->value.bool_val);
            break;
        case JSARG_TYPE_INT64_T:
            str = std::to_string(arg->value.int64_t_val);
            break;
        case JSARG_TYPE_NULL:
            str = "null";
            break;
        case JSARG_TYPE_UNDEFINED:
            str = "undefined";
            break;
        case JSARG_TYPE_UINT32_T:
            str = std::to_string(arg->value.uint32_t_val);
            break;
        case JSARG_TYPE_EXCEPTION:
            str += std::string(arg->value.exception_val.name);
            str += ",";
            str += std::string(arg->value.exception_val.msg);
            break;
        case JSARG_TYPE_STRING:
            str = std::string(arg->value.str_val);
            break;

        case JSARG_TYPE_C_ARRAY:
            if (arg->value.c_array_val.items)
            {
                str += "[";
                for (size_t i = 0; i < arg->value.c_array_val.count; ++i)
                {
                    if (i > 0 && i < arg->value.c_array_val.count - 1) {
                        str += ", ";
                    }
                    str += jsarg_to_string(arg->value.c_array_val.items[i]);
                }
                str += "]";
            }
            break;
        case JSARG_TYPE_UINT8_ARRAY:
            if (arg->value.u8_array_val.items)
            {
                str = ejr::bytes_to_string<uint8_t>(
                    arg->value.u8_array_val.items,
                    arg->value.u8_array_val.count
                );
            }
            break;
        case JSARG_TYPE_INT32_ARRAY:
            if (arg->value.i32_array_val.items)
            {
                str = ejr::bytes_to_string<int32_t>(
                    arg->value.i32_array_val.items,
                    arg->value.i32_array_val.count
                );
            }
            break;
        case JSARG_TYPE_UINT32_ARRAY:
            if (arg->value.u32_array_val.items)
            {
                str = ejr::bytes_to_string<uint32_t>(
                    arg->value.u32_array_val.items,
                    arg->value.u32_array_val.count
                );
            }
            break;
        case JSARG_TYPE_INT64_ARRAY:
            if (arg->value.i64_array_val.items)
            {
                str = ejr::bytes_to_string<int64_t>(
                    arg->value.i64_array_val.items,
                    arg->value.i64_array_val.count
                );
            }
            break;
        case JSARG_TYPE_INT8_ARRAY:
            if (arg->value.i8_array_val.items)
            {
                str = ejr::bytes_to_string<int8_t>(
                    arg->value.i8_array_val.items,
                    arg->value.i8_array_val.count
                );
            }
            break;
        case JSARG_TYPE_UINT16_ARRAY:
            if (arg->value.u16_array_val.items)
            {
                str = ejr::bytes_to_string<uint16_t>(
                    arg->value.u16_array_val.items,
                    arg->value.u16_array_val.count
                );
            }
            break;
        case JSARG_TYPE_INT16_ARRAY:
            if (arg->value.i16_array_val.items)
            {
                str = ejr::bytes_to_string<int16_t>(
                    arg->value.i16_array_val.items,
                    arg->value.i16_array_val.count
                );
            }
            break;
        case JSARG_TYPE_UINT64_ARRAY:
            if (arg->value.u64_array_val.items)
            {
                str = ejr::bytes_to_string<uint64_t>(
                    arg->value.u64_array_val.items,
                    arg->value.u64_array_val.count
                );
            }
            break;
        case JSARG_TYPE_FLOAT_ARRAY:
            if (arg->value.float_array_val.items)
            {
                str = ejr::bytes_to_string<float>(
                    arg->value.float_array_val.items,
                    arg->value.float_array_val.count
                );
            }
            break;

        default:
            break;
        }

        return ejr::create_raw_string(str);
    }
}