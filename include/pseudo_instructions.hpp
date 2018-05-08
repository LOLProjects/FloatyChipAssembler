/*
pseudo_instructions.hpp

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
#ifndef PSEUDO_INSTRUCTIONS_HPP
#define PSEUDO_INSTRUCTIONS_HPP

#include <cstddef>
#include <cstdint>

#include <vector>
#include <functional>

namespace floaty
{

class Instruction;

bool is_seek(const Instruction& ins);
bool is_data_insert(const Instruction& ins);
bool is_dup(const Instruction& ins);

bool is_pseudo_ins(const Instruction& ins);

size_t handle_seek_directive(const Instruction& ins, size_t old_idx);
std::vector<uint8_t> handle_data_insert_directive(const Instruction& ins);
void handle_dup_directive(const Instruction& ins, std::function<void(const Instruction&)> callback);

}

#endif // PSEUDO_INSTRUCTIONS_HPP
