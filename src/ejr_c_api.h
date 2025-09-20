#ifndef EJR_C_API_H
#define EJR_C_API_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Types

/**
 * @brief The C wrapper for EasyJSR
 */
typedef struct EasyJSRHandle EasyJSRHandle;

/**
 * @brief our JSValue allocator/deallocator
 */
typedef struct JSValueAD JSValueAD;

/**
 * @brief C version of our JSArg union
 */
typedef enum {
    JSARG_TYPE_INT,
    JSARG_TYPE_DOUBLE,
    JSARG_TYPE_STRING,
    JSARG_TYPE_FLOAT,
    JSARG_TYPE_BOOL,
    JSARG_TYPE_INT64_T,
    JSARG_TYPE_UINT32_T,
    JSARG_TYPE_C_ARRAY
} JSArgType;
typedef struct JSArg JSArg;
/**
 * @brief C version of our JSArg union
 */
struct JSArg {
    JSArgType type;
    union {
        int int_val;
        double double_val;
        const char* str_val;
        int jsvalue_id;
        float float_val;
        bool bool_val;
        uint32_t uint32_t_val;
        int64_t int64_t_val;
        struct {
            JSArg* items;
            size_t count;
        } c_array_val;
    } value;
};
// C Callback wrapper for DynCallback
typedef JSArg (*C_Callback)(const JSArg* args, size_t arg_count);

/**
 * @brief C version of JSMethod
 */
struct JSMethod {
    const char* name;
    C_Callback cb;
};

// Constructors

/**
 * @brief Create a new easyjs runtime.
 * 
 * It will have it's own callables, modules, classes, etc. It is all sandboxed within itself.
 */
EasyJSRHandle* ejr_new();

/**
 * @brief Create a new JSValueAD.
 */
JSValueAD* jsvad_new();

/**
 * @brief Create a int JSArg.
 * 
 * @param value The int
 * 
 * @return JSArg
 */
JSArg jsarg_int(int value);

/**
 * @brief Create a const char* JSArg.
 * 
 * @param value The const char*
 * 
 * @return JSArg
 */
JSArg jsarg_str(const char* value);

/**
 * @brief Create a double JSArg.
 * 
 * @param value The double
 * 
 * @return JSArg
 */
JSArg jsarg_double(double value);

/**
 * @brief Create a float JSArg.
 * 
 * @param value The float
 * 
 * @return JSArg
 */
JSArg jsarg_float(float value);

/**
 * @brief Create a int64_t JSArg.
 * 
 * @param value The int64_t
 * 
 * @return JSArg
 */
JSArg jsarg_int64t(int64_t value);

/**
 * @brief Create a uint32_t JSArg.
 * 
 * @param value The uint32_t
 * 
 * @return JSArg
 */
JSArg jsarg_uint32t(uint32_t value);

/**
 * @brief Create a C-Array JSArg.
 * 
 * @param value The size of the array
 * 
 * @return JSArg
 */
JSArg jsarg_carray(size_t count);

/**
 * @brief Add a JSArg value to a array.
 * 
 * @param arg Pointer to the array.
 * @param value the JSArg value.
 */
void jsarg_add_value_to_c_array(JSArg* arg, JSArg value);

// Deleters
/**
 * @brief Free a easyjs runtime
 */
void ejr_free(EasyJSRHandle* handle);

/**
 * @brief Free a JSValueAD
 * 
 * @param handle The associate easyjsr runtime.
 */
void jsvad_free(JSValueAD* jsvad, EasyJSRHandle* handle);

void jsarg_free(JSArg* arg);

// EasyJSR specific
/**
 * @brief Evaluate a JS script at the global level.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param js The JS code.
 * @param file_name The name of the file.
 * 
 * @return The id of the created JSValue.
 */
int ejr_eval_script(JSValueAD* jsvad, EasyJSRHandle* handle, const char* js, const char* file_name);

/**
 * @brief Evaluate a JS script as a module level.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param js The JS code.
 * @param file_name The name of the file.
 * 
 * @return The id of the created JSValue.
 */
int ejr_eval_module(JSValueAD* jsvad, EasyJSRHandle* handle, const char* js, const char* file_name);

/**
 * @brief Evaluate a JS function in the current scope/runtime.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param fn_name The JS function name.
 * @param args The args to pass into the function.
 * @param arg_count Number of args being passed.
 * 
 * @return The id of the resulted JSValue.
 */
int ejr_eval_function(JSValueAD* jsvad, EasyJSRHandle* handle, const char* fn_name, JSArg* args, size_t arg_count);

/**
 * @brief Convert a JSValue into a c_string.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param value_id The id of the JSValue to be converted.
 * 
 * @return The c_string.
 */
const char * ejr_val_to_string(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id);

/**
 * @brief Convert a JSValue into a c_string.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param name The name of the JSValue to be get.
 * 
 * @return the id of the resulted JSValue
 */
int ejr_get_from_global(JSValueAD* jsvad, EasyJSRHandle* handle, const char* name);

/**
 * @brief Evaluate a JS function in the current scope/runtime on a class or object.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param value_id The objects/classes id in jsvad
 * @param fn_name The JS function name.
 * @param args The args to pass into the function.
 * @param arg_count Number of args being passed.
 * 
 * @return The id of the resulted JSValue.
 */
int ejr_eval_class_function(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id, const char* fn_name, JSArg* args, size_t arg_count);

/**
 * @brief Get a property from a object.
 * 
 * @param jsvad The JSValueAD for this runtime.
 * @param handle the easyjsr runtime.
 * @param value_id The objects/classes id in jsvad
 * @param property The property name.
 * 
 * @return the Id of the resulted value.
 */
int ejr_get_property_from(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id, const char* property);

/**
 * @brief Register a callback in JS.
 * 
 * @param handle The easyjsr runtime.
 * @param fn_name Name to give the callback.
 * @param cb The actual C callback.
 */
void ejr_register_callback(EasyJSRHandle* handle, const char* fn_name, C_Callback cb);

/**
 * @brief Register a module in JS.
 * 
 * @param handle The easyjsr runtime.
 * @param module_name Name to give the module.
 * @param methods A array of methods to add to the runtime.
 * @param method_count number of methods.
 */
void ejr_register_module(EasyJSRHandle* handle, const char* module_name, JSMethod* methods, size_t method_count);

/**
 * @brief Free a C string.
 * 
 * @param handle The easyjsr runtime.
 * @param c_string The c string to free.
 */
void ejr_free_string(EasyJSRHandle* handle, const char* c_string);

// JSValueAD specific
/**
 * @brief Free a JSValue within the JSValueAD.
 * 
 * @param jsvad The JSValueAD pointer.
 * @param handle The easyjsr runtime associated.
 * @param value_id id of the value to free.
 */
void jsvad_free_jsvalue(JSValueAD* jsvad, EasyJSRHandle* handle, int value_id);

#ifdef __cplusplus
}
#endif /// __cplusplus

#endif // EJR_C_API_H