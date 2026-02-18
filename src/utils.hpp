#pragma once

#include <string>
#include <fstream>
#include <include/ejr_error.hpp>
#include <lib/quickjs.h>
#include <sstream>
#include <vector>

namespace ejr
{
    /// @brief Load a .js file from './'
    EJRError<std::string> load_js_file(const std::string &file_path);
    /// @brief Check if string starts with x
    bool str_starts_with(const std::string &str, const std::string &prefix);

    /// @brief check if string ends with x
    bool str_ends_with(const std::string &str, const std::string &suffix);

    /// @brief check if strign contains x
    bool str_contains(const std::string &str, const std::string&needle);

    /// @brief trim string
    std::string str_trim(const std::string &str);

    /// @brief split a string
    std::vector<std::string> str_split(const std::string& s, const std::string& delimiter);

    /// @brief Create a char* from a string.
    char* create_raw_string(const std::string& contents);

    /// @brief Craete a char* from a const char*
    char* create_raw_string(const char* contents);

    /// @brief Convert a list of byte items to a string
    template<typename T>
    std::string bytes_to_string(const T* items, size_t count) {
        if (items == nullptr) {
            return "[]";
        }
        std::string res = "[";
        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                res += ", ";
            }
            res += std::to_string(items[i]);
        }
        res += "]";
        return res;
    }
}