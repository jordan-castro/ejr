# TODO

- Regestering C++ classes
    - For now just doing callback first method.
- Regestering custom classes
    - For now just doing callback first method
- add support for all JS Values
- Potentially add import support in a repl? But node does not so it's prob fine...

- Finish Tests

- ~~Add async/await support via ejr_await_promise(handle_ptr, val_id)~~
- Add error handling via ejr_catch_error().
- Add double -> Float64 array

- Add optional script load to module:
    - So import { File } from 'ejr:io'
        - Does: 
            - Look for ejr:io module specific functions (actual module)
            - call JS_EvalScript(ejr_instance->module_scripts[this_instance])
            - Reutrn actual module as usual