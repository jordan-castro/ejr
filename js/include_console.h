
#ifndef INCLUDE_CONSOLE 
#define INCLUDE_CONSOLE
        
inline constexpr const char* console_contents = R"qwleg(globalThis.console = {
    /**
     * Print to the console.
     * @param  {...any} v the arguments to print.
     */
    log(...v) {
        globalThis.___print(`${v.join(" ")}`);
    },

    // /**
    //  * Panic with a message. 
    //  * @param  {...any} v the arguments to show when panicing
    //  */
    // error(...v) {
    //     globalThis.___error(`${v.join(" ")}`);
    // },
})qwleg";
        
#endif // INCLUDE_CONSOLE