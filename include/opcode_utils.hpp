/*
opcode_utils.hpp

Copyright (c) 04 Yann BOUCHER (yann)

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
#ifndef OPCODE_UTILS_HPP
#define OPCODE_UTILS_HPP

#include "constexpr_utils.hpp"

#include <tuple>

namespace floaty
{

constexpr const char opcode_char_list[] = "0123456789ABCDEFabcdef_xyzwn";

constexpr bool const_isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}

constexpr bool const_isspace(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

template <const char* str, char op_type>
constexpr size_t get_operand_bit_size()
{
    size_t count { 0 };
    size_t len = const_strlen<str>();
    for (size_t i { 0 }; i < len; ++i)
    {
        if (str[i] == op_type) ++count;
    }

    if (count <= 2) return 8;
    else if (count <= 4) return 16;
    else return 32;
}

template <const char* mnemo_fmt>
constexpr auto get_mnemo()
{
    constexpr size_t len = const_str_find<mnemo_fmt, 0, ' '>() != const_npos ?
                const_str_find<mnemo_fmt, 0, ' '>() :
                const_strlen<mnemo_fmt>();
    auto first_token = const_str_span<mnemo_fmt, 0, len>{};
    return first_token;
}

template <const char* mnemo_fmt>
constexpr size_t operand_count()
{
    size_t i { 0 };
    while (i < const_strlen<mnemo_fmt>() && !const_isspace(mnemo_fmt[i])) ++i; // skip the mnemo
    if (i == const_strlen<mnemo_fmt>()) return 0;
    while (i < const_strlen<mnemo_fmt>() && const_isspace(mnemo_fmt[i]))  ++i; // skip the spaces after the mnemo

    size_t count { 1 };

    for (; i < const_strlen<mnemo_fmt>(); ++i)
    {
        if (mnemo_fmt[i] == ',') ++count;
    }

    return count;
}

template <const char* mnemo_fmt, size_t idx>
constexpr size_t get_operand_offset()
{
    if (idx >= operand_count<mnemo_fmt>()) return const_npos;
    static_assert(idx < operand_count<mnemo_fmt>(), "Invalid operand index");

    constexpr size_t str_len = const_strlen<mnemo_fmt>();
    size_t i { 0 };
    size_t op_idx { 0 };
    while (!const_isspace(mnemo_fmt[i])) ++i; // skip the mnemo
    while (const_isspace(mnemo_fmt[i]))  ++i; // skip the spaces after the mnemo

    while (true)
    {
        if (op_idx == idx)
        {
            return i;
        }
        else
        {
            while (i < str_len && mnemo_fmt[i] != ',') ++i;
            if (mnemo_fmt[i] == ',') ++i; // skip the comma
            ++op_idx;
        }
    }
}

template <const char* mnemo_fmt, size_t idx>
constexpr auto get_operand()
{
    constexpr size_t base = get_operand_offset<mnemo_fmt, idx>();
    constexpr size_t len = (const_str_find<mnemo_fmt, base, ','>() != const_npos
            ? const_str_find<mnemo_fmt, base, ','>()
            : const_strlen<mnemo_fmt>())
           - base;
    return const_str_span<mnemo_fmt, base, len>{};
}

template <const char* format>
constexpr bool check_mnemo_fmt()
{
    static_assert(const_strlen<format>() != 0, "The mnemonic format musn't be empty !");
    auto mnemo = get_mnemo<format>();
    static_assert(const_str_check<mnemo.str(), 0, mnemo.size(), const_isupper>(), "The mnemonic must be uppercase");

    return true;
}

template <const char* pattern>
constexpr bool check_pattern()
{
    static_assert(const_strlen<pattern>() == 6, "The opcode pattern must have a length of 6 chars (24-bit)");
    static_assert(const_is_made_of<pattern, opcode_char_list>(), "The opcode pattern must be made of the following : \"0123456789ABCDEFabcdef_xyzwn\"");

    return true;
}

}

#endif // OPCODE_UTILS_HPP
