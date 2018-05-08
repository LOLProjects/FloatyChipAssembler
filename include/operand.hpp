/*
operand.hpp

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
#ifndef OPERAND_HPP
#define OPERAND_HPP

#include <cstddef>

#include <string_view>

#include "opcode_utils.hpp"
#include "stl_utils.hpp"

namespace floaty
{

enum class OperandType
{
    ByteImmediate,
    Address,
    NReg,
    BReg,
    IReg,
    BIndirectReg,
    IIndirectReg,
    StackPointer,
    IndirectAddr,
    IndirectIReg,
    IndirectIRegPlusN,
    IndirectIRegPlusBRegPlusN,
    SoundTimer,
    DelayTimer,
    Invalid
};

static constexpr const char addr_str[] = "addr";
static constexpr const char indir_addr_str[] = "[addr]";
static constexpr const char b_indir_str[] = "B(N";
static constexpr const char i_indir_str[] = "I(N";
static constexpr const char sp_str[] = "SP";
static constexpr const char st_str[] = "ST";
static constexpr const char dt_str[] = "DT";
static constexpr const char i_addr_indir_beg_str[] = "[I";
static constexpr const char i_addr_indir_end_str[] = "+n]";
static constexpr const char i_b_addr_indir_mid_str[] = "+B";

template <const char* String, size_t Offset, size_t Length>
struct Operand
{
    static constexpr std::string_view str()
    {
        return {String + Offset, Length};
    }
    static constexpr OperandType type()
    {
        constexpr OperandType optype = type_impl();
        static_assert(optype != OperandType::Invalid, "Invalid operand type");

        return optype;
    }

    static constexpr char operand_char()
    {
        switch (type())
        {
            case OperandType::ByteImmediate:
                return str()[0];
            case OperandType::NReg:
            case OperandType::BReg:
            case OperandType::IReg:
                return str()[1];
            case OperandType::BIndirectReg:
            case OperandType::IIndirectReg:
                return str()[3];
            case OperandType::IndirectIRegPlusN:
            case OperandType::IndirectIRegPlusBRegPlusN:
                return str()[2];
            default:
                return '\0';
        }
    }

    static constexpr char indexed_breg_char()
    {
        //static_assert(type() == OperandType::IndirectIRegPlusBRegPlusN);
        if (type() != OperandType::IndirectIRegPlusBRegPlusN) return '\0';
        return str()[5];
    }

private:
    static constexpr OperandType type_impl()
    {
        if constexpr (Length == 1 &&
                      (const_str_find<String, Offset, 'n'>() == 0 || const_str_find<String, Offset, 'x'>() == 0 ||
                       const_str_find<String, Offset, 'y'>() == 0 || const_str_find<String, Offset, 'z'>() == 0 ||
                       const_str_find<String, Offset, 'w'>() == 0))
        {
            return OperandType::ByteImmediate;
        }
        else if constexpr (const_str_eq<String, Offset, addr_str, 0, 4>())
        {
            return OperandType::Address;
        }
        else if constexpr (const_str_eq<String, Offset, indir_addr_str, 0, 6>())
        {
            return OperandType::IndirectAddr;
        }
        else if constexpr (const_str_eq<String, Offset, st_str, 0, 2>())
        {
            return OperandType::SoundTimer;
        }
        else if constexpr (const_str_eq<String, Offset, dt_str, 0, 2>())
        {
            return OperandType::DelayTimer;
        }
        else if constexpr (const_str_eq<String, Offset, sp_str, 0, 2>())
        {
            return OperandType::StackPointer;
        }
        else if constexpr (const_str_eq<String, Offset, b_indir_str, 0, 3>())
        {
            return OperandType::BIndirectReg;
        }
        else if constexpr (const_str_eq<String, Offset, i_indir_str, 0, 3>())
        {
            return OperandType::IIndirectReg;
        }
        else if constexpr (const_str_eq<String, Offset, i_addr_indir_beg_str, 0, 2>() &&
                           const_str_eq<String, Offset + 3, i_addr_indir_end_str, 0, 3>())
        {
            return OperandType::IndirectIRegPlusN;
        }
        else if constexpr (const_str_eq<String, Offset, i_addr_indir_beg_str, 0, 2>() &&
                           const_str_eq<String, Offset + 3, i_b_addr_indir_mid_str, 0, 2>() &&
                           const_str_eq<String, Offset + 6, i_addr_indir_end_str, 0, 3>())
        {
            return OperandType::IndirectIRegPlusBRegPlusN;
        }
        else if constexpr (Length == 2)
        {
            //static_assert(str()[0] == 'N' || str()[0] == 'B' || str()[0] == 'I', "Invalid register type");
            switch (str()[0])
            {
                case 'N':
                    return OperandType::NReg;
                case 'B':
                    return OperandType::BReg;
                case 'I':
                    return OperandType::IReg;
            }
        }
        else if constexpr (Length == 4 && const_str_eq<String, Offset, i_addr_indir_beg_str, 0, 2>())
        {
            return OperandType::IndirectIReg;
        }
        return OperandType::Invalid;
    }

    static constexpr bool check()
    {
        static_assert(const_str_count<String, Offset, '['>() == const_str_count<String, Offset, ']'>(), "Unmatched bracket found");
        static_assert(const_str_count<String, Offset, '('>() == const_str_count<String, Offset, ')'>(), "Unmatched parenthesis found");
        static_assert(type_impl() != OperandType::Invalid);

        return true;
    }
    static_assert(check());
};

template <const char* Mnemo, size_t I>
constexpr auto build_operand()
{
    constexpr auto operand = floaty::get_operand<Mnemo, I>().trim();
    return Operand<Mnemo, operand.offset(), operand.size()>{};
}

template<const char* Mnemo, std::size_t... I>
constexpr auto get_operand_tuple_impl(std::index_sequence<I...>)
{
    return std::make_tuple(build_operand<Mnemo, I>()...);
}

template<const char* Mnemo, std::size_t N, typename Indices = std::make_index_sequence<N>>
constexpr auto get_operand_tuple()
{
    return get_operand_tuple_impl<Mnemo>(Indices{});
}

inline bool is_identifier(std::string_view str)
{
    return to_lower(std::string(str)).find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_.") == std::string_view::npos;
}

inline bool is_string(std::string_view str)
{
    if (str.size() < 2) return false;
    if (str.front() != '"' || str.back() != '"') return false;

    return true;
}

inline bool is_immediate(std::string_view str)
{
    return is_number(std::string(str))
            && std::stol(std::string(str), nullptr, 0) >= -127 && std::stol(std::string(str), nullptr, 0) <= 255;
}

inline bool is_signed_immediate(std::string_view str)
{
    return is_number(std::string(str))
            && std::stol(std::string(str), nullptr, 0) >= -127 && std::stol(std::string(str), nullptr, 0) <= 127;
}

inline bool is_address(std::string_view str)
{
    return is_number(std::string(str))
            || is_identifier(str);
}

inline bool is_reg(std::string_view str, char reg)
{
    if (str.size() != 2) return false;
    if (str[0] != reg) return false;
    if (!isxdigit(str[1])) return false;

    return true;
}

inline bool is_indir_reg(std::string_view str, char reg)
{
    if (str.size() != 5) return false;
    if (str.substr(0, 4) != reg + std::string("(N") || str.back() != ')') return false;
    if (!isxdigit(str[3])) return false;

    return true;
}

inline bool is_indir_address(std::string_view str)
{
    if (str.size() <= 2) return false;
    if (str.front() != '[' || str.back() != ']') return false;
    str.remove_prefix(1);
    str.remove_suffix(1);

    return is_address(str);
}

inline bool is_indir_ireg_plus_n(std::string_view str)
{
    if (str.size() <= 3) return false;
    if (str.front() != '[' || str.back() != ']') return false;
    if (str[1] != 'I') return false;
    if (!isxdigit(str[2])) return false;

    return true;
}

inline bool is_indir_address_plus_n(std::string_view str)
{
    if (str.size() <= 3) return false;
    if (str.front() != '[' || str.back() != ']') return false;
    if (str[1] != 'I') return false;
    if (!isxdigit(str[2])) return false;
    // keep only the offset
    str.remove_prefix(3);
    str.remove_suffix(1);

    return true;
}

inline bool is_indir_address_plus_breg_plus_n(std::string_view str)
{
    if (str.size() <= 3) return false;
    if (str.front() != '[' || str.back() != ']') return false;
    if (str[1] != 'I') return false;
    if (!isxdigit(str[2])) return false;
    if (str[3] != '+' || str[4] != 'B') return false;
    if (!isxdigit(str[5])) return false;
    // keep only the offset
    str.remove_prefix(6);
    str.remove_suffix(1);

    return true;
}

}

#endif // OPERAND_HPP
