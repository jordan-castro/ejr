
#ifndef INCLUDE_CONSOLE 
#define INCLUDE_CONSOLE
        
inline constexpr const char* console_contents = R"zmpqz(globalThis.console = {
    /**
     * Print to the console.
     * @param  {...any} v the arguments to print.
     */
    log(...v) {
        globalThis.___print(`${v.join(" ")}`);
    },

    /**
     * Print with ERROR. 
     * @param  {...any} v the arguments to show when erroring
     */
    error(...v) {
        globalThis.___error(`${v.join(" ")}`);
    },

    /**
     * Print with WARNING.
     * @param {...arny} v the arguments to show when warning.
     */
    warn(...v) {
        globalThis.___warn(`${v.join(" ")}`);
    }
})zmpqz";
        
#endif // INCLUDE_CONSOLE