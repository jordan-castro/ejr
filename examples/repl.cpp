#include <iostream>
#include <string>
#include <include/ejr.hpp>
#include <memory>
#include <stdlib/ejr_console.hpp>

using namespace std;
using namespace ejr;

int main() {
    unique_ptr<EasyJSR> easyjsr = make_unique<EasyJSR>();

    include_console_std(*easyjsr);
 
    while (1) {
        string input;
        cout << ">> ";
        getline(cin, input);

        if (input == "q") {
            break;
        }

        // Run JS and get as string
        auto val = easyjsr->run_script(input, "repl.js");
        try {
        cout << easyjsr->val_to_string(val) << endl;
        } catch (...) {
            cout << "Could not do " << input << endl;
        }
    }

    cout << "So long, and thanks for all the fish!" << endl;
    return 0;
}
