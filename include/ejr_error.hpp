#pragma once

#include <string>

namespace ejr
{
    template <typename T>
    class EJRError
    {
    public:
        bool has_error;
        std::string msg;
        T result;

        EJRError(T result, bool has_error, const std::string &msg) : has_error(has_error), msg(msg), result(result) {}

        static EJRError error(const std::string &msg)
        {
            return EJRError(T(), true, msg);
        }

        static EJRError good(const T &result)
        {
            return EJRError(result, false, "");
        }
    };
};