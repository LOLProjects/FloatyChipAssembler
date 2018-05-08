/*
constexpr_utility.hpp

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
#ifndef CONSTEXPR_UTILITY_HPP
#define CONSTEXPR_UTILITY_HPP

#include <cstddef>
#include <cctype>

#include <utility>
#include <tuple>

namespace floaty
{

constexpr size_t const_npos = (size_t)-1;

template <const char* str>
constexpr size_t const_strlen()
{
    size_t idx { 0 };
    while (str[idx]) idx++;

    return idx;
}

template <const char* str, size_t off>
constexpr size_t const_strlen()
{
    size_t idx { off };
    while (str[idx]) idx++;

    return idx - off;
}

template <const char* str, size_t Base, size_t Len, bool(*pred)(char)>
constexpr bool const_str_check()
{
    for (size_t i { Base }; i < Len; ++i)
    {
        if (!pred(str[i])) return false;
    }

    return true;
}

template <const char* str, size_t off, char c>
constexpr size_t const_str_find()
{
    for (size_t i { off }; i < const_strlen<str>(); ++i)
    {
        if (str[i] == c) return i - off;
    }

    return const_npos;
}

template <const char* str, size_t off, const char* other, size_t other_off>
constexpr bool const_str_eq()
{
    auto len = const_strlen<str, off>();
    if (len != const_strlen<other, other_off>()) return false;

    for (size_t i { 0 }; i < len; ++i)
    {
        if (str[off + i] != other[other_off + i]) return false;
    }

    return true;
}

template <const char* str, size_t off, const char* other, size_t other_off, size_t length>
constexpr bool const_str_eq()
{
    if (const_strlen<str, off>() > length && const_strlen<other, other_off>() > length)
    {
        return false;
    }

    for (size_t i { 0 }; i < length; ++i)
    {
        if (str[off + i] != other[other_off + i]) return false;
    }

    return true;
}

template <const char* str, size_t off, char c>
constexpr size_t const_str_count()
{
    size_t count { 0 };

    for (size_t i { off }; i < const_strlen<str>(); ++i)
    {
        if (str[i] == c) ++count;
    }

    return count;
}

template <const char* str, size_t off, char c>
constexpr size_t const_str_find_first_not()
{
    for (size_t i { off }; i < const_strlen<str>(); ++i)
    {
        if (str[i] != c) return i - off;
    }

    return const_npos;
}

template <const char* str, const char* char_list>
constexpr bool const_is_made_of()
{
    size_t str_len  = const_strlen<str>();
    size_t list_len = const_strlen<char_list>();

    for (size_t i { 0 }; i < str_len; ++i)
    {
        bool found = false;
        for (size_t j { 0 }; j < list_len; ++j)
        {
            if (str[i] == char_list[j]) found = true;
        }

        if (!found) return false;
    }

    return true;
}

template <const char* TpString, size_t TpOffset, size_t TpLength>
struct const_str_span
{
    static constexpr auto String = TpString;
    static constexpr auto Offset = TpOffset;
    static constexpr auto Length = TpLength;

    constexpr auto offset() const { return Offset; }
    constexpr auto str() const { return String + Offset; }
    constexpr auto size() const { return Length; }

    constexpr auto trim() const
    {
        constexpr size_t left_offset_tmp = const_str_find_first_not<String, Offset, ' '>() != const_npos
                ? const_str_find_first_not<String, Offset, ' '>()
            : 0;
        constexpr size_t left_offset = (String + Offset)[left_offset_tmp] == ',' ? left_offset_tmp-1 : left_offset_tmp;

        constexpr size_t length_tmp = const_str_find<String, Offset + left_offset, ' '>() != const_npos
                ? const_str_find<String, Offset + left_offset, ' '>() - left_offset
            : Length - left_offset;
        // Remove trailing comma
        constexpr size_t length = (String + Offset + left_offset - 1)[length_tmp] == ',' ? length_tmp-1 : length_tmp;

        return const_str_span<String, Offset + left_offset, length>{};
    }
};

// from https://stackoverflow.com/a/16387374/8429846
namespace detail
{
template<typename T, typename F, size_t... Is>
inline void for_each(T&& t, [[maybe_unused]] F f, std::index_sequence<Is...>)
{
    if constexpr (sizeof...(Is) > 0)
    {
        auto l = { (f(std::get<Is>(t), Is), 0)... };
        (void)l;
    }
}
}

template<typename... Ts, typename F>
inline void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
{
    detail::for_each(t, f, std::make_index_sequence<sizeof...(Ts)>());
}

}

#endif // CONSTEXPR_UTILITY_HPP
