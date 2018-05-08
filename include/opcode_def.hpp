/*
opcode.hpp

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
#ifndef OPCODE_HPP
#define OPCODE_HPP

#include <cstdint>
#include <cassert>

#include <string_view>
#include <array>

#include "operand.hpp"

#include "opcode_utils.hpp"
#include "type_utils.hpp"
#include "stl_utils.hpp"

#include <iostream>

namespace floaty
{

template <const char* OpcodePattern, const char* MnemonicFormat>
struct Opcode
{
public:
    static constexpr auto Pattern = OpcodePattern;
    static constexpr auto MnemoFmt = MnemonicFormat;

    static_assert(check_pattern<Pattern>());
    static_assert(check_mnemo_fmt<MnemoFmt>());

    // Opcode operands
    uintn_t<get_operand_bit_size<OpcodePattern, 'n'>()> n;
    uintn_t<get_operand_bit_size<OpcodePattern, 'w'>()> w;
    uintn_t<get_operand_bit_size<OpcodePattern, 'x'>()> x;
    uintn_t<get_operand_bit_size<OpcodePattern, 'y'>()> y;
    uintn_t<get_operand_bit_size<OpcodePattern, 'z'>()> z;

public:
    static constexpr std::string_view mnemonic()
    {
        constexpr auto mnemo = get_mnemo<MnemoFmt>();
        return {mnemo.str(), mnemo.size()};
    }
    static constexpr auto operands()
    {
        return get_operand_tuple<MnemoFmt, operand_count()>();
    }
    static constexpr size_t operand_count()
    {
        return floaty::operand_count<MnemoFmt>();
    }

    static constexpr bool matches(uint32_t opcode)
    {
        return (opcode & mask()) == 0;
    }

    template <char c>
    static constexpr uint8_t operand_offset()
    {
        constexpr size_t index = const_str_find<Pattern, 0, c>();
        //static_assert(index != const_npos);
        return 5 - index;
    }

    static constexpr uint32_t base()
    {
        uint32_t base { 0 };
        for (size_t i { 0 }; i < const_strlen<Pattern>(); ++i)
        {
            base <<= 4;
            if (isxdigit(Pattern[i]))
            {
                if (isalpha(Pattern[i]))
                {
                    base |= tolower(Pattern[i]) - 'a' + 0xa;
                }
                else
                {
                    base |= Pattern[i] - '0';
                }
            }
        }
        return base;
    }

    static constexpr uint32_t mask()
    {
        uint32_t mask { 0 };
        for (size_t i { 0 }; i < const_strlen<Pattern>(); ++i)
        {
            mask <<= 4;
            if (Pattern[i] == '_' || Pattern[i] == 'x' || Pattern[i] == 'y' || Pattern[i] == 'z'
                    || Pattern[i] == 'z' || Pattern[i] == 'n')
            {
                mask |= 0xF;
            }
        }

        return mask;
    }
    static_assert(mask() != 0xFFFFFF);
};

}

#endif // OPCODE_HPP
