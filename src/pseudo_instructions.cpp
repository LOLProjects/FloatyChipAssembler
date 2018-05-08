/*
pseudo_instructions.cpp

Copyright (c) 08 Yann BOUCHER (yann)

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

#include "pseudo_instructions.hpp"

#include "assembler.hpp"
#include "operand.hpp"

#include <iostream>

namespace floaty
{

bool is_pseudo_ins(const Instruction &ins)
{
    return is_seek(ins) || is_data_insert(ins) || is_dup(ins);
}

bool is_seek(const Instruction &ins)
{
    return to_upper(ins.mnemo) == "SEEK";
}

bool is_data_insert(const Instruction &ins)
{
    if (ins.mnemo.size() == 2 && toupper(ins.mnemo[0]) == 'D')
    {
        if (toupper(ins.mnemo[1]) == 'S')
        {
            return true;
        }
        else
        {
            size_t width = toupper(ins.mnemo[1]) == 'B' ? 1 : toupper(ins.mnemo[1]) == 'W' ? 2 : toupper(ins.mnemo[1]) == 'D' ? 4 : 0;
            if (width == 0) return false;

            return true;
        }
    }

    return false;
}

bool is_dup(const Instruction &ins)
{
    return to_upper(ins.mnemo) == "DUP";
}

size_t handle_seek_directive(const Instruction &ins, size_t old_idx)
{
    if (ins.arguments.size() != 1 || !is_number(ins.arguments[0]))
    {
        assembler_error_throw("invalid SEEK directive", ins.line, ins.filename);
    }
    auto seek_addr = std::stoul(ins.arguments[0], nullptr, 0);
    if (seek_addr < old_idx)
    {
        assembler_error_throw("cannot SEEK backwards", ins.line, ins.filename);
    }

    return seek_addr;
}

std::vector<uint8_t> handle_data_insert_directive(const Instruction &ins)
{
    if (toupper(ins.mnemo[1]) == 'S')
    {
        if (ins.arguments.size() != 1 || !is_string(ins.arguments[0]))
        {
            assembler_error_throw("invalid ds directive", ins.line, ins.filename);
        }

        // TODO : handle escape codes
        auto str = unquoted(ins.arguments[0]);
        return std::vector<uint8_t>{str.begin(), str.end()};
    }
    else
    {
        size_t width = toupper(ins.mnemo[1]) == 'B' ? 1 : toupper(ins.mnemo[1]) == 'W' ? 2 : toupper(ins.mnemo[1]) == 'D' ? 4 : 0;
        if (width == 0) assembler_error_throw("invalid data pseudo instruction width", ins.line, ins.filename);

        std::vector<uint8_t> data;
        for (size_t i { 0 }; i < ins.arguments.size(); ++i)
        {
            const auto& arg = ins.arguments[i];
            if (!is_number(arg))
                assembler_error_throw("argument " + std::to_string(i) + " of data pseudo instruction is invalid", ins.line, ins.filename);

            uint32_t value = std::stoi(arg, nullptr, 0);
            // Add every byte, least significant byte first (little-endian)
            for (size_t j { 0 }; j < width; ++j)
            {
                data.emplace_back(value &= 0xFF);
                value >>= 8;
            }
        }
        return data;
    }
}

void handle_dup_directive(const Instruction &ins, std::function<void (const Instruction &)> callback)
{
    if (ins.arguments.size() < 2 || !is_number(ins.arguments[0])) assembler_error_throw("invalid DUP directive", ins.line, ins.filename);
    Instruction to_repeat = ins;
    to_repeat.mnemo = ins.arguments[1];
    if (ins.arguments.size() > 2)
        to_repeat.arguments = std::vector<std::string>{ins.arguments.begin()+2, ins.arguments.end()};

    for (int i { 0 }; i < std::stol(ins.arguments[0], nullptr, 0); ++i)
    {
        callback(to_repeat);
    }
}

}
