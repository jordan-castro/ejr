#include "utils.hpp"

using namespace std;

#define STR_CHECK_SIZE(l, r) \
    do { if ((l).size() > (r).size()) { return false; } } while(0)

namespace ejr
{
    EJRError<string> load_js_file(const string &file_path)
    {
        string f_path;
        if (str_starts_with(file_path, "./"))
        {
            // Then we are gucci
            f_path = file_path;
        }
        else
        {
            f_path = "./" + file_path;
        }

        // Open file
        fstream file(f_path);

        if (!file.is_open())
        {
            return EJRError<string>::error("Could not open file.");
        }

        // We have a file, read it
        stringstream buffer;
        buffer << file.rdbuf();

        string contents = buffer.str();

        // Close the file
        file.close();

        return EJRError<string>::good(contents);
    }

    bool str_starts_with(const string &haystack, const string &needle)
    {
        STR_CHECK_SIZE(needle, haystack);

        return haystack.compare(0, needle.size(), needle) == 0;
    }

    bool str_ends_with(const string& source, const string &suffix) {
        STR_CHECK_SIZE(suffix, source);

        return source.compare(source.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool str_contains(const string& str, const string& needle) {
        STR_CHECK_SIZE(needle, str);

        return str.find(needle) != string::npos;
    }

    string str_trim(const std::string& s) {
        if (s.empty()) return s;

        size_t start = 0;
        size_t end = s.size() - 1;

        // Find first non-whitespace character
        while (start <= end && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }

        // Find last non-whitespace character
        while (end >= start && std::isspace(static_cast<unsigned char>(s[end]))) {
            --end;
        }

        return s.substr(start, end - start + 1);
    }

    vector<string> str_split(const string &s, const string &delimiter) {
        vector<string> tokens;
        size_t start = 0;
        size_t end;

        while ((end = s.find(delimiter, start)) != string::npos) {
            tokens.push_back(s.substr(start, end - start));
            start = end + delimiter.length();
        }

        // Add the last token
        tokens.push_back(s.substr(start));

        return tokens;
    }
};
