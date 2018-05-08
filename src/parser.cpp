/*
parser.cpp

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

#include "parser.hpp"

#include "stl_utils.hpp"

#include <gsl/gsl_span.hpp>

#include <optional>
#include <iostream>

#include "stl_utils.hpp"

namespace floaty
{

std::optional<Label> pending_label;

void handle_line_directive(gsl::span<std::string_view> toks, unsigned& line, std::string& filename)
{
    if (toks.size() != 2 && toks.size() != 3)
    {
        parser_error_throw("malformed #line directive", line, filename);
    }

    try
    {
        line = std::stoul(toks[1].data(), nullptr, 0);
    }
    catch (const std::invalid_argument&)
    {
        parser_error_throw("invalid line number for #line directive", line, filename);
    }
    catch (const std::out_of_range&)
    {
        parser_error_throw("invalid line number for #line directive", line, filename);
    }

    if (toks.size() == 3)
    {
        filename = unquoted(toks[2]);
    }
}

Instruction handle_instruction(gsl::span<std::string_view> toks, unsigned& line, std::string& filename)
{
    Instruction ins;
    std::optional<Label> label = pending_label;

    if (toks[0].back() == ':')
    {
        if (label)
        {
            parser_error_throw("an instruction can only have one label", line, filename);
        }
        label = Label{};
        label->name = toks[0].substr(0, toks[0].size()-1); // minus the ':'
        toks = toks.subspan<1>();
    }

    if (label)
    {
        ins.label = label;
        pending_label.reset();
    }

    if (toks.empty())
    {
        parser_error_throw("invalid instruction", line, filename);
    }

    ins.mnemo = to_upper(std::string(toks[0]));
    toks = toks.subspan<1>();
    ins.arguments = std::vector<std::string>{toks.begin(), toks.end()};
    ins.line = line;
    ins.filename = filename;

    ++line;

    return ins;
}

void handle_lone_label(gsl::span<std::string_view> toks, unsigned& line, std::string&)
{
    pending_label = Label{};
    pending_label->name = toks[0].substr(0, toks[0].size() - 1); // minus the :

    ++line;
}

std::optional<AssemblerDirective> process_line(std::string_view input, unsigned& line, std::string& filename)
{
    auto tokens = split(input, " ,");
    if (tokens.empty())
    {
        ++line;
        return {};
    }

    if (tokens[0] == "#line")
    {
        handle_line_directive(tokens, line, filename);
        return {};
    }
    else if (tokens.size() == 1 && tokens[0].back() == ':')
    {
        handle_lone_label(tokens, line, filename);
        return {};
    }
    else
    {
        return handle_instruction(tokens, line, filename);
    }
}

std::vector<AssemblerDirective> parse(std::string_view input, std::string_view filename)
{
    std::vector<AssemblerDirective> directives;
    unsigned line_number { 1 };
    std::string current_filename { filename };

    auto lines = split(input, "\n", false, false);

    for (auto line : lines)
    {
        auto dir = process_line(line, line_number, current_filename);
        if (dir) directives.emplace_back(dir.value());
    }

    return directives;
}

}
