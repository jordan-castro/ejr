# TODO

- ~~run_script~~
- ~~eval only (without running)~~
- ~~getting functions~~
- ~~calling functions~~
- ~~setting callbacks~~
- ~~add vector to JSArg type~~
- Regestering C++ classes
    - For now just doing callback first method.
- Regestering custom classes
    - For now just doing callback first method
- ~~Register modules~~
- ~~Macro for loading custom JS files to include at startup.~~
- ~~advanced error logging~~ (Decent enough with clean_js_value for now)...
- add support for all JS Values in _JSArg
- ~~Add import support~~
- Potentially add import support in a repl? But node does not so it's prob fine...

- ~~Fix memory leak when running scripts.~~
- Should I add a GC?
- ~~Write C library~~

- ~~Allow for custom module loaders.~~
- ~~Pass in Context opaque~~
- ~~Allow for internal JS only modules~~.

- Finish Tests

- ~~Fix memory leaks AGAIN!! <- Focop~~
- ~~maybe add ejr_ to all functions? (Like does the client need to know about JSVAD?)~~

- ~~Just create a libejr.dll or .a or whatever~~
~~    - i.e. apply static linking for cpp stuff as to not need to include both the c_api and cpp_api.~~

- ~~Figure out the Assertion failed: list_empty(&rt->gc_obj_list) error~~

- Add async/await support via ejr_await_promise(handle_ptr, val_id)