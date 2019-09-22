#pragma once

#include "xed_lib.hpp"

xed_bool_t read_function_instructions_x86(xed_uint64_t function_start, std::vector<xed_instruction> &instructions);
xed_bool_t read_function_instructions_x64(xed_uint64_t function_start, std::vector<xed_instruction> &instructions);
xed_bool_t read_function_instructions(xed_uint64_t function_start, std::vector<xed_instruction> &instructions, xed_bool_t is_wow64_process);

