#pragma once

#include <lib/quickjs.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <type_traits>
#include <functional>
#include <unordered_map>
#include <lib/quickjs_cpp_utils.hpp>
#include <iostream>
#include <tuple>
#include <lib/quickjs_cpp_utils.hpp>

namespace ejr
{
    // Types
    /// @brief a null representation for JSArg.
    struct JSArgNull
    {
    };

    /// @brief a undefined representation for JSArg.
    struct JSArgUndefined
    {
    };

    /// @brief a exception representiation for JSArg.
    struct JSArgException {
        std::string msg;
        std::string name;

        JSArgException(const std::string& msg, const std::string& name) : msg(msg), name(name) {} 
    };

    /// @brief a TypedArray representation for JSArg.
    template <typename T>
    struct JSArgTypedArray
    {
        std::vector<T> values;

        JSArgTypedArray(std::vector<T>&& values) : values(values) {}
    };
    /// @brief A JSArg for dynamic typing.
    struct JSArg
    {
        using ValueType = std::variant<
            int,
            double,
            float,
            long,
            std::string,
            bool,
            int64_t,
            uint32_t,
            JSArgNull,
            JSArgUndefined,
            std::shared_ptr<std::vector<JSArg>>,
            JSArgTypedArray<uint8_t>,
            JSArgTypedArray<int32_t>,
            JSArgTypedArray<uint32_t>,
            JSArgTypedArray<int64_t>,
            JSArgTypedArray<int8_t>,
            JSArgTypedArray<int16_t>,
            JSArgTypedArray<uint16_t>,
            JSArgTypedArray<uint64_t>,
            JSArgTypedArray<float>,
            JSArgException>;

        ValueType value;

        // Constructors for convenience
        JSArg(int v) : value(v) {}
        JSArg(double v) : value(v) {}
        JSArg(float v) : value(v) {}
        JSArg(const std::string &v) : value(v) {}
        JSArg(bool v) : value(v) {}
        JSArg(uint32_t v) : value(v) {}
        JSArg(int64_t v) : value(v) {}
        JSArg(std::nullptr_t) : value(JSArgNull{}) {}
        JSArg(std::monostate) : value(JSArgUndefined{}) {}
        JSArg(std::vector<JSArg> &&vec) : value(std::make_shared<std::vector<JSArg>>(std::move(vec))) {}
        JSArg(const JSArgException& exec) : value(exec) {}

        template<typename T>
        JSArg(JSArgTypedArray<T> v) : value(v) {}
    };

    /// @brief A type for Dynamic Callbacks ([JSArgs]) -> JSArg
    using DynCallback = std::function<JSArg(const std::vector<JSArg> &)>;
    /// @brief Shorthand for std::vector<JSArg>
    using JSArgs = std::vector<JSArg>;
    /// @brief Type for file loader function
    using FileLoaderFn = std::function<std::string(const std::string &)>;

    /// @brief RAII JSValue
    /// Use this inteligently, sometimes you want to free the JSValue manually.
    class EJRValue
    {
    private:
        JSValue val;
        JSContext *ctx;

    public:
        EJRValue(JSContext *ctx, JSValue val);
        ~EJRValue();

        JSValue &get_ref();

        // Don't allow copying
        EJRValue(const EJRValue &) = delete;
        EJRValue &operator=(const EJRValue &) = delete;
        EJRValue(EJRValue &&other) noexcept : val(other.val), ctx(other.ctx)
        {
            other.val = js_undefined();
        }
        EJRValue &operator=(EJRValue &&other) noexcept
        {
            if (this != &other)
            {
                JS_FreeValue(this->ctx, this->val);
                val = other.val;
                this->ctx = other.ctx;
                other.val = js_undefined();
            }
            return *this;
        }
    };

    /// @brief A method.
    struct JSMethod
    {
        std::string name;
        DynCallback callback;

        JSMethod(const std::string &name, DynCallback callback);
    };

    // Templates
    /**
     * Get a JSArg as a certain type.
     */
    template <typename T>
    T jsarg_as(const ejr::JSArg &arg)
    {
        return std::get<T>(arg.value);
    }

    /// @brief get the JSClassID from a type
    template <typename T>
    JSClassID get_js_class_id()
    {
        static JSClassID class_id = 0; // static ensures one per type
        if (class_id == 0)
        {
            JS_NewClassID(&class_id);
        }
        return class_id;
    }

    /// @brief Create a JS Typed Array
    template<typename T>
    JSValue create_js_array_typed(JSContext* ctx, const JSArgTypedArray<T>& typed_array) {
        // Get array type from T
        JSTypedArrayEnum array_type;

        if constexpr (std::is_same_v<T, uint8_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_UINT8;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_INT32;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_UINT32;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_BIG_INT64;
        } else if constexpr (std::is_same_v<T, int8_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_INT8;
        } else if constexpr (std::is_same_v<T, int16_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_INT16;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_UINT16;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_BIG_UINT64;
        } else if constexpr (std::is_same_v<T, float>) {
            array_type = JSTypedArrayEnum::JS_TYPED_ARRAY_FLOAT32;
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type for JS typed array");
        }

        const uint8_t* values = reinterpret_cast<const uint8_t*>(typed_array.values.data());

        JSValue buffer = JS_NewArrayBufferCopy(ctx, values, typed_array.values.size() * sizeof(T));
        JSValue argv[3] = { buffer, js_undefined(), js_undefined() };
        JSValue array = JS_NewTypedArray(ctx, 3, argv, array_type);

        // Free buffer
        JS_FreeValue(ctx, buffer);

        return array;
    }

    // Utils
    /// @brief Convert a JSArgs type into a JSValue.
    JSValue to_js(JSContext *ctx, const JSArg &args);
    /// @brief Convert a JSValue into a JSArg
    JSArg from_js(JSContext *ctx, JSValue value, bool force_free = true);
    /// @brief Convert a JSArg into a string, will return "unkown" if not vaild JSArg to string
    std::string jsarg_to_str(const JSArg &arg);

    // EasyJSR class
    /**
     * @brief The easyjs runtime.
     */
    class EasyJSR
    {
    private:
        /// @brief The runtime from quickjs.
        /// Each EasyJSR instance has its own sandboxed runtime.
        JSRuntime *runtime = nullptr;
        /// @brief The context from quickjs.
        JSContext *ctx = nullptr;

        /// @brief Callbacks
        std::unordered_map<std::string, DynCallback> callbacks;

        /// @brief Make sure if the value is a exception we return a string of cause: $cause, message: $message
        std::tuple<JSValue, bool> clean_js_value(JSValue val);

        /// @brief Create a trampoline that calls a callback from callbacks.
        JSValue create_trampoline(const std::string &cb_name, DynCallback cb);

        /// @brief Unmangled names of methods with their JSValue.
        std::unordered_map<std::string, std::vector<std::tuple<std::string, JSValue>>> methods_by_module;

        /// @brief internal file loader. Set via set_file_loader
        FileLoaderFn file_loader_fn;

    public:
        EasyJSR();
        ~EasyJSR();

        /// @brief initiate a module statically
        static int module_init(JSContext *ctx, JSModuleDef *m);

        /// @brief Set a file loader function.
        void set_file_loader(FileLoaderFn func);

        /// @brief Convert a JSValue into a JSArg.
        JSArg jsvalue_to_jsarg(JSValue value, bool force_free);

        /// @brief a File Loader.
        std::string load_file(const std::string &file_path);

        /// @brief registered Modules
        std::unordered_map<std::string, JSModuleDef *> modules;

        /// @brief a really easy way to run a JS script. No strings attached, very plain way.
        JSValue eval_script(const std::string &js, const std::string &file_name);

        /// @brief a really easy way to run a JS module. No strings attached, very plain way.
        JSValue eval_module(const std::string &js, const std::string &file_name);

        /// @brief free a JSValue using this runtimes context.
        void free_jsval(JSValue value);

        /// @brief Free a vector of JSValue
        void free_jsvals(const std::vector<JSValue> &js_args);

        /// @brief evaluate JS with strings attached.
        JSValue eval(const std::string &js, const std::string &file_name, int eval_flags);

        /// @brief evalute a function from the current global context.
        JSValue eval_function(const std::string &fnName, const std::vector<JSArg> &args);

        /// @brief convert a JSValue into a std::string with the current context.
        std::string val_to_string(JSValue value, bool free = true);

        /// @brief Get a Value from globalThis
        JSValue get_from_global(const std::string &name);

        /// @brief Evalute a function on a class/object.
        JSValue eval_class_function(JSValue obj, const std::string &fn_name, const std::vector<JSArg> &args);

        /// @brief get a property from a JSValue object
        JSValue get_property_from(JSValue object, std::string property);

        /// @brief Wrap a JSValue in a EJRValue for RAII
        EJRValue wrap_js_val(JSValue val);

        /// @brief register a callback (Runtime only)
        void register_callback(const std::string &fn_name, DynCallback callback);

        /// @brief register a module (Runtime only)
        void register_module(const std::string &module_name, const std::vector<JSMethod> &methods);

        // /// @brief register a CPP class (Runtime only).
        // template<typename T>
        // inline void register_class(const std::string &class_name, std::vector<PublicMethod> public_methods) {
        //     // Register a new class id
        //     static JSClassID class_id = get_js_class_id<T>();

        //     // Register the class definition
        //     JSClassDef def{};
        //     def.class_name = class_name.c_str();
        //     def.finalizer = [](JSRuntime* rt, JSValue val) {
        //         // Free memory
        //         if (auto* t = static_cast<T*>(JS_GetOpaque(val, class_id))) {
        //             delete t;
        //         }
        //     };

        //     // Register new class
        //     if (JS_NewClass(this->runtime, class_id, &def) < 0) {
        //         // Some kind of error happened here
        //         // TODO: save some kind of error.
        //         return;
        //     }

        //     // Constructor
        //     auto t_ctor = [](JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv) {
        //         // Build vector<JSArg>
        //         std::vector<JSArg> args;
        //         args.reserve(argc);
        //         for (int i = 0; i < argc; ++i) {
        //             args.push_back(from_js(ctx, argv[i]));
        //         }

        //         // Allocate JS Object
        //         JSValue obj = JS_NewObjectClass(ctx, class_id);
        //         if (JS_IsException(obj)) {
        //             return obj;
        //         }

        //         // Construct native object
        //         T* t = nullptr;
        //         try {
        //             t = new T(args);
        //         } catch (const std::exception& e) {
        //             JS_FreeValue(ctx, obj);
        //             return JS_ThrowInternalError(ctx, ("Constructor failed: " + std::string(e.what())).c_str());
        //         }

        //         // Link native object to JS object
        //         JS_SetOpaque(obj, t);

        //         return obj;
        //     };

        //     // Constructor func
        //     JSValue t_ctor_func = JS_NewCFunction2(
        //         this->ctx,
        //         t_ctor,
        //         class_name.c_str(),
        //         1,
        //         JS_CFUNC_constructor,
        //         0
        //     );

        //     // Create prototype object
        //     JSValue proto = JS_NewObject(this->ctx);

        //     // Mangle the prototype name to the callbcks names
        //     std::string mangled_name = "___" + class_name + "___";

        //     // Bind methods to prototype
        //     for (size_t i = 0 ; i < public_methods.size(); i++) {
        //         auto& m = public_methods[i];

        //         auto method_wrapper = [](JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv, int magic, JSValue* func_data) -> JSValue {
        //             std::cout << "Before recover PublicMethod ptr" << std::endl;
        //             EasyJSR *self = reinterpret_cast<EasyJSR *>(JS_VALUE_GET_PTR(func_data[0]));

        //             // Collect args as JSArg
        //             std::vector<JSArg> args;
        //             args.reserve(argc + 1);
        //             std::cout << "Here 3" << std::endl;

        //             // Push "this" pointer
        //             if (T* obj_ptr = static_cast<T*>(JS_GetOpaque(this_val, class_id))) {
        //                 args.push_back(reinterpret_cast<uintptr_t>(obj_ptr));
        //             }
        //             std::cout << "Here 4" << std::endl;

        //             // Convert argv[]
        //             for (int i = 0; i < argc; ++i) {
        //                 args.push_back(from_js(ctx, argv[i], false));
        //             }
        //             std::cout << "Here 5" << std::endl;

        //             // Lookup callback by magic
        //             const char *cbname_cstr = JS_ToCString(ctx, func_data[1]);
        //             std::string cb_name(cbname_cstr);
        //             JS_FreeCString(ctx, cbname_cstr);

        //             auto it = self->callbacks.find(cb_name);
        //             if (it == self->callbacks.end())
        //             {
        //                 return js_undefined();
        //             }

        //             // Call C++ callback
        //             JSArg result = it->second(args);

        //             // Convert result back to JS
        //             return to_js(ctx, result);
        //         };

        //         // Copy callback into callbcaks
        //         std::string mangled_callback_name = mangled_name + m.name;
        //         this->callbacks[mangled_callback_name] = public_methods[i].callback;

        //         // Store pointer in func_data
        //         JSValue func_data[2];

        //         // Store name and EasyJSR pointer
        //         func_data[0] = this->js_ptr;
        //         func_data[1] = JS_NewString(this->ctx, mangled_callback_name.c_str());

        //         JSValue fn = JS_NewCFunctionData(this->ctx, method_wrapper, 0, 0, 2, func_data);

        //         JS_SetPropertyStr(this->ctx, proto, m.name.c_str(), fn);
        //     }

        //     // Attach prototype constructor
        //     JS_SetConstructor(this->ctx, t_ctor_func, proto);

        //     // Expose globally
        //     auto global = JS_GetGlobalObject(this->ctx);
        //     JS_SetPropertyStr(this->ctx, global, class_name.c_str(), t_ctor_func);

        //     // Free global
        //     this->free_jsval(global);
        // }

        // /// @brief Register a class dynamically (Runtime only)
        // void register_dynamic_class();
    };
};
