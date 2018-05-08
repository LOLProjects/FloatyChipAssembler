/*
assembler.hpp

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
#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <stdexcept>

#include <gsl/gsl_span.hpp>

namespace floaty
{

struct assembler_error : std::runtime_error
{
        using std::runtime_error::runtime_error;
};

[[noreturn]] inline void assembler_error_throw(const std::string& why, unsigned line, const std::string& filename)
{
    throw assembler_error("Assembler error : " + why + ", line " + std::to_string(line) + ", in " + filename);
}

struct Label
{
    std::string name;
};

struct CommonDirective
{
    unsigned line { 0 };
    std::string filename;
};

struct Instruction : public CommonDirective
{
    std::optional<Label> label;
    std::string mnemo;
    std::vector<std::string> arguments;
};

using AssemblerDirective = std::variant<Instruction>;

std::vector<uint8_t> assemble(gsl::span<const AssemblerDirective> instructions);

}

#endif // ASSEMBLER_HPP
