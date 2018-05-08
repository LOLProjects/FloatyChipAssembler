/*
assembler.cpp

Copyright (c) 06 Yann BOUCHER (yann)

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

#include "assembler.hpp"

#include <unordered_map>
#include <iostream>

#include "opcode_def.hpp"
#include "pseudo_instructions.hpp"

namespace floaty
{

constexpr const char test_pat[] = "10_xnn";
constexpr const char test_fmt[] = "LD Bx, n, B(Nx), [Iy+n], [Iy+Bx+n], [addr]";
constexpr const char test_pat2[] = "11_xnn";
constexpr const char test_fmt2[] = "LD [addr], Bx";

static_assert(std::get<0>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::BReg);
static_assert(std::get<1>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::ByteImmediate);
static_assert(std::get<2>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::BIndirectReg);
static_assert(std::get<3>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::IndirectIRegPlusN);
static_assert(std::get<4>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::IndirectIRegPlusBRegPlusN);
static_assert(std::get<5>(Opcode<test_pat, test_fmt>::operands()).type() == OperandType::IndirectAddr);

static_assert(std::get<0>(Opcode<test_pat2, test_fmt2>::operands()).type() == OperandType::IndirectAddr);

using SymbolTable = std::unordered_map<std::string, uint16_t>;

template <typename Opcode>
bool matches(const Instruction& ins)
{
    if (Opcode::mnemonic() != ins.mnemo) return false;
    // try to transform <op> rx, ry into <op> rx, rx, ry
    if (Opcode::operand_count() != ins.arguments.size() && ins.arguments.size() == 2)
    {
        Instruction new_ins = ins;
        new_ins.arguments = {ins.arguments[0], ins.arguments[0], ins.arguments[1]};
        return matches<Opcode>(new_ins);
    }

    bool is_valid = true;
    for_each_in_tuple(Opcode::operands(), [&ins, &is_valid](auto operand, size_t idx)
    {
        switch (operand.type())
        {
            case OperandType::ByteImmediate:
                is_valid &= is_immediate(ins.arguments[idx]);
                return;
            case OperandType::Address:
                is_valid &= is_address(ins.arguments[idx]);
                return;
            case OperandType::IndirectAddr:
                is_valid &= is_indir_address(ins.arguments[idx]);
                return;
            case OperandType::NReg:
                is_valid &= is_reg(ins.arguments[idx], 'N');
                return;
            case OperandType::BReg:
                is_valid &= is_reg(ins.arguments[idx], 'B');
                return;
            case OperandType::IReg:
                is_valid &= is_reg(ins.arguments[idx], 'I');
                return;
            case OperandType::BIndirectReg:
                is_valid &= is_indir_reg(ins.arguments[idx], 'B');
                return;
            case OperandType::IIndirectReg:
                is_valid &= is_indir_reg(ins.arguments[idx], 'I');
                return;
            case OperandType::StackPointer:
                is_valid &= ins.arguments[idx] == "SP";
                return;
            case OperandType::IndirectIReg:
                is_valid &= is_indir_ireg_plus_n(ins.arguments[idx]);
                return;
            case OperandType::IndirectIRegPlusN:
                is_valid &= is_indir_address_plus_n(ins.arguments[idx]);
                return;
            case OperandType::IndirectIRegPlusBRegPlusN:
                is_valid &= is_indir_address_plus_breg_plus_n(ins.arguments[idx]);
                return;
            case OperandType::SoundTimer:
                is_valid &= ins.arguments[idx] == "ST";
                return;
            case OperandType::DelayTimer:
                is_valid &= ins.arguments[idx] == "DT";
                return;
            case OperandType::Invalid:
                __builtin_unreachable();
        }
    });

    return is_valid;
}

template <typename Opcode>
uint32_t assemble_opcode(const Instruction& ins, const SymbolTable& tbl)
{
    // try to transform <op> rx, ry into <op> rx, rx, ry
    if (Opcode::operand_count() != ins.arguments.size() && ins.arguments.size() == 2)
    {
        Instruction new_ins = ins;
        new_ins.arguments = {ins.arguments[0], ins.arguments[0], ins.arguments[1]};
        return assemble_opcode<Opcode>(new_ins, tbl);
    }

    uint32_t opcode { Opcode::base() };

    for_each_in_tuple(Opcode::operands(), [&ins, &opcode, &tbl](auto operand, size_t idx)
    {
        switch (operand.type())
        {
            case OperandType::ByteImmediate:
                opcode |= std::stoi(ins.arguments[idx], nullptr, 0) << Opcode::template operand_offset<operand.operand_char()>()*4;
                return;
            case OperandType::NReg:
            case OperandType::BReg:
            case OperandType::IReg:
                opcode |= xdigit_to_num(ins.arguments[idx][1]) << Opcode::template operand_offset<operand.operand_char()>()*4;
                return;
            case OperandType::BIndirectReg:
            case OperandType::IIndirectReg:
                opcode |= xdigit_to_num(ins.arguments[idx][3]) << Opcode::template operand_offset<operand.operand_char()>()*4;
                return;
            case OperandType::Address:
            case OperandType::IndirectAddr:
                if (is_number(ins.arguments[idx]))
                {
                    opcode |= std::stoi(ins.arguments[idx], nullptr, 0);
                }
                else
                {
                    if (!tbl.count(ins.arguments[idx]))
                    {
                        assembler_error_throw("label '" + ins.arguments[idx] + "' doesn't exist", ins.line, ins.filename);
                    }
                    opcode |= tbl.at(ins.arguments[idx]);
                }
                return;
            case OperandType::IndirectIRegPlusBRegPlusN:
                opcode |= xdigit_to_num(ins.arguments[idx][5]) << Opcode::template operand_offset<operand.indexed_breg_char()>()*4;
                [[fallthrough]];
            case OperandType::IndirectIRegPlusN:
                if (has_indirect_offset(ins.arguments[idx]))
                {
                    if (is_number(std::string{get_indirect_offset(ins.arguments[idx])}))
                    {
                        const int value = std::stoi(std::string{get_indirect_offset(ins.arguments[idx])}, nullptr, 0);
                        if ((operand.type() == OperandType::IndirectIRegPlusN         && (value <= -128 || value >= 128)) ||
                            (operand.type() == OperandType::IndirectIRegPlusBRegPlusN && (value < 0 || value >= 16)))
                            assembler_error_throw("indexed operand offset "
                                                  + std::string{get_indirect_offset(ins.arguments[idx])}
                                                  + " is out of range", ins.line, ins.filename);
                        opcode |= (int8_t)value << Opcode::template operand_offset<'n'>()*4;
                    }
                    else
                    {
                        assembler_error_throw("invalid indexed operand offset '"
                                              + std::string{get_indirect_offset(ins.arguments[idx])} + "'", ins.line, ins.filename);
                    }
                }
                [[fallthrough]];
            case OperandType::IndirectIReg:
                opcode |= xdigit_to_num(ins.arguments[idx][2]) << Opcode::template operand_offset<operand.operand_char()>()*4;
                return;
            case OperandType::Invalid:
                __builtin_unreachable();
            default:
                return;
        }
    });

    return opcode;
}

#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y)

// Explicitly instantiate functions for the opcode list so we can implement the function in the source file
#define OPCODE_DEF(pattern, fmt) \
    static constexpr const char TOKENPASTE(opcode_pattern_, pattern)[] = #pattern; \
    static constexpr const char TOKENPASTE(opcode_fmt_, pattern)[] = fmt;
#include "opcodes.def"

std::pair<bool(*)(const Instruction&), uint32_t(*)(const Instruction&, const SymbolTable&)> call_table[] =
{
    // Explicitly instantiate functions for the opcode list so we can implement the function in the source file
    #define OPCODE_DEF(pattern, fmt) \
{&matches<Opcode<TOKENPASTE(opcode_pattern_, pattern), TOKENPASTE(opcode_fmt_, pattern)>>,   \
        &assemble_opcode<Opcode<TOKENPASTE(opcode_pattern_, pattern), TOKENPASTE(opcode_fmt_, pattern)>> \
},

    #include "opcodes.def"
};

void apply_ins_offset(const Instruction& ins, size_t& index)
{
    // Check if it is a pseudo instruction
    if (is_seek(ins))
    {
        index = handle_seek_directive(ins, index);
    }
    else if (is_data_insert(ins))
    {
        index += handle_data_insert_directive(ins).size();
    }
    else if (is_dup(ins))
    {
        handle_dup_directive(ins, [&index](const Instruction& ins)
        {
            apply_ins_offset(ins, index);
        });
    }
    else
    {
        // regular instruction
        index += 3;
    }
}

SymbolTable build_symbol_table(gsl::span<const AssemblerDirective> instructions, size_t& index)
{
    SymbolTable tbl;

    for (auto dir : instructions)
    {
        if (std::holds_alternative<Instruction>(dir))
        {
            const auto& ins = std::get<Instruction>(dir);
            if (ins.label)
            {
                if (tbl.count(ins.label->name))
                {
                    assembler_error_throw("multiple definition of label " + ins.label->name, ins.line, ins.filename);
                }

                tbl[ins.label->name] = index;
            }

            apply_ins_offset(ins, index);
        }
    }

    return tbl;
}

class AssemblerOutput
{
public:
    enum OutputEndianess
    {
        BigEndian,
        LittleEndian
    };

public:
    AssemblerOutput(size_t max_size)
        : data(max_size, 0)
    {}

    template <typename T, size_t byte_size = sizeof(T), OutputEndianess endian = OutputEndianess::LittleEndian>
    void output_data(T value)
    {
        static_assert(byte_size <= sizeof(T));
        if constexpr (endian == OutputEndianess::LittleEndian)
        {
            for (size_t i { 0 }; i < byte_size; ++i)
            {
                data[idx++] = value&0xFF;
                if constexpr (byte_size > 1) value >>= 8;
            }
        }
        else
        {
            for (size_t i { 0 }; i < byte_size; ++i)
            {
                data[idx + (byte_size-i-1)] = value&0xFF;
                if constexpr (byte_size > 1) value >>= 8;
            }
            idx += byte_size;
        }
    }

    void relocate(size_t new_idx)
    {
        assert(new_idx >= idx);
        idx = new_idx;
    }

    std::vector<uint8_t> data;
    size_t idx { 0 };
};

void assemble_instruction(const Instruction& ins, const SymbolTable& sym_tbl, AssemblerOutput& out)
{
    // handle pseudo instructions
    if (is_seek(ins))
    {
        out.relocate(handle_seek_directive(ins, out.idx));
        return;
    }
    if (is_data_insert(ins))
    {
        for (uint8_t byte : handle_data_insert_directive(ins))
        {
            out.output_data(byte);
        }
        return;
    }
    if (is_dup(ins))
    {
        handle_dup_directive(ins, [&sym_tbl, &out](const Instruction& ins)
        {
            assemble_instruction(ins, sym_tbl, out);
        });
        return;
    }

    for (auto call_pair : call_table)
    {
        if (call_pair.first(ins))
        {
            out.output_data<uint32_t, 3, AssemblerOutput::BigEndian>(call_pair.second(ins, sym_tbl));
            return;
        }
    }

    std::string ins_str = ins.mnemo + " ";
    for (size_t i { 0 }; i < ins.arguments.size(); ++i)
    {
        ins_str += ins.arguments[i];
        if (i < ins.arguments.size() - 1)
            ins_str += ", ";
    }
    assembler_error_throw("invalid instruction '" + ins_str + "'", ins.line, ins.filename);
}

std::vector<uint8_t> assemble(gsl::span<const AssemblerDirective> instructions)
{
    size_t max_index { 0 };
    auto sym_tbl = build_symbol_table(instructions, max_index);

    for (auto pair : sym_tbl)
    {
    }

    AssemblerOutput asm_output(max_index);

    for (auto dir : instructions)
    {
        if (std::holds_alternative<Instruction>(dir))
        {
            const auto& ins = std::get<Instruction>(dir);
            assemble_instruction(ins, sym_tbl, asm_output);
        }
    }

    return asm_output.data;
}

}
