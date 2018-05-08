/*
stl_utils.hpp

Copyright (c) 05 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef STL_UTILS_HPP
#define STL_UTILS_HPP

#include <cstdlib>
#include <cassert>

#include <vector>
#include <string_view>
#include <string>

namespace floaty
{

inline std::vector<std::string_view> split(std::string_view str, std::string_view chars = " ", bool ignore_quotes = false, bool skip_empty = true)
{
    std::vector<std::string_view> result;

    size_t idx { 0 };
    bool in_quote { false };
    do
    {
        const auto begin = idx;

        while (idx < str.size() && (chars.find(str[idx]) == std::string_view::npos || in_quote))
        {
            if (str[idx] == '"' && !ignore_quotes) in_quote = !in_quote;

            ++idx;
        }

        if (!skip_empty || !std::string_view(&str[begin], idx - begin).empty())
        {
            result.push_back(std::string_view(&str[begin], idx - begin));
        }
    } while (++idx < str.size());

    return result;
}

inline bool is_quoted(std::string_view str)
{
    return !str.empty() && str.front() == '"' && str.back() == '"';
}

inline std::string_view unquoted(std::string_view str)
{
    if (is_quoted(str))
    {
        str.remove_prefix(1);
        str.remove_suffix(1);
    }

    return str;
}

inline bool is_number(const std::string& str)
{
    if (str.empty()) return false;

    char* p;
    std::strtol(str.c_str(), &p, 0);

    return *p == '\0';
}

inline uint8_t xdigit_to_num(char c)
{
    assert(isxdigit(c));

    if (isalpha(c))
    {
        return c - 'a' + 0xa;
    }
    else
    {
        return c - '0';
    }
}

inline std::string to_upper(std::string str)
{
    for (auto & c: str) c = toupper(c);
    return str;
}


inline std::string to_lower(std::string str)
{
    for (auto & c: str) c = tolower(c);
    return str;
}
}

#endif // STL_UTILS_HPP
