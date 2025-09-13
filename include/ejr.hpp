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

namespace ejr
{
    // Types
    /// @brief A JSArg for dynamic typing.
    using _JSArg = std::variant<int, double, float, long, std::string, bool, int64_t, uint32_t, uintptr_t>;
    /// @brief A JSArgs type
    using JSArg = std::variant<_JSArg, std::vector<_JSArg>>;
    /// @brief A type for Dynamic Callbacks ([JSArgs]) -> JSArg
    using DynCallback = std::function<JSArg(const std::vector<JSArg>&)>;
    
    /// @brief PublicMethod for adding classes.
    struct PublicMethod {
        std::string name;
        DynCallback callback;

        PublicMethod(const std::string name, DynCallback callback) : name(name), callback(callback) {}
    };
 
    // Templates
    /**
     * Get a JSArg as a certain type.
     */ 
    template<typename T>
    T jsarg_as(const ejr::JSArg& arg) {
        if (std::holds_alternative<_JSArg>(arg)) {
            return std::get<T>(std::get<_JSArg>(arg));
        }
        throw std::bad_variant_access(); // not a scalar
    }

    /// @brief get the JSClassID from a type
    template<typename T>
    JSClassID get_js_class_id() {
        static JSClassID class_id = 0; // static ensures one per type
        if (class_id == 0) {
            JS_NewClassID(&class_id);
        }
        return class_id;
    }

    /// @brief Get a T from a JSArg pointer
    template<typename T>
    T* get_obj_from_ptr(JSArg arg) {
        int64_t ptr_val = jsarg_as<uintptr_t>(arg);
        T* self = reinterpret_cast<T*>(ptr_val);
        return self;
    }

    // Utils
    /// @brief Convert a JSArg into a JSValue
    JSValue __to_js(JSContext *ctx, const _JSArg &arg);
    /// @brief Convert a JSArgs type into a JSValue.
    JSValue to_js(JSContext *ctx, const JSArg &args);
    /// @brief Convert a JSValue into a JSArg
    JSArg from_js(JSContext *ctx, JSValue value, bool force_free=true);

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

        // /// @brief EasyJSR class as a JSValue pointer
        // JSValue js_ptr;

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

        /// @brief register a callback (Runtime only)
        void register_callback(const std::string &fn_name, DynCallback callback);
        
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
