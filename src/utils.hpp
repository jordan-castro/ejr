#pragma once

#include <string>
#include <fstream>
#include <include/ejr_error.hpp>
#include <lib/quickjs.h>
#include <sstream>

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
    
}