#include "pch.h"

#include "xed_instruction_utils.hpp"

xed_bool_t read_function_instructions_x86(xed_uint64_t function_start, std::vector<xed_instruction> &instructions)
{
	xed_uint64_t runtime_address = function_start;

	// duplicate check
	auto it = std::find_if(instructions.begin(), instructions.end(), xed_instruction::find_address(runtime_address));
	if (it != instructions.end())
	{
		return false;
	}

	std::vector<xed_uint64_t> relbrs;

	while (true)
	{
		xed_instruction instruction;

		xed_error_enum_t xed_error = instruction.decode(runtime_address, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b);
		if (xed_error != XED_ERROR_NONE)
		{
			return false;
		}

		// add instruction to list
		instructions.push_back(instruction);

		// next instruction
		runtime_address += instruction.get_length();

		// rel je/jne/jae/jbe/jnbe/jnae/jmp... etc
		auto operands = instruction.get_operands();
		if (operands.size() >= 2
			&& operands[0].get_name() == XED_OPERAND_RELBR
			&& instruction.get_register(operands[1].get_name()) == XED_REG_EIP)
		{
			relbrs.push_back(runtime_address + instruction.get_branch_displacement());
		}

		// ret function end.
		if (instruction.get_category() == XED_CATEGORY_RET)
		{
			break;
		}
	}

	for (auto relbr_address : relbrs)
	{
		if (!read_function_instructions_x86(relbr_address, instructions))
		{
			//printf("already instruction %p\n", relbr_address);
		}
	}

	return true;
}

xed_bool_t read_function_instructions_x64(xed_uint64_t function_start, std::vector<xed_instruction> &instructions)
{
	xed_uint64_t runtime_address = function_start;

	// duplicate check
	auto it = std::find_if(instructions.begin(), instructions.end(), xed_instruction::find_address(runtime_address));
	if (it != instructions.end())
	{
		return false;
	}

	std::vector<xed_uint64_t> relbrs;

	while (true)
	{
		xed_instruction instruction;

		xed_error_enum_t xed_error = instruction.decode(runtime_address, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
		if (xed_error != XED_ERROR_NONE)
		{
			return false;
		}

		// add instruction to list
		instructions.push_back(instruction);

		// next instruction
		runtime_address += instruction.get_length();

		// rel je/jne/jae/jbe/jnbe/jnae/jmp... etc
		auto operands = instruction.get_operands();
		if (operands.size() >= 2
			&& operands[0].get_name() == XED_OPERAND_RELBR
			&& instruction.get_register(operands[1].get_name()) == XED_REG_RIP)
		{
			relbrs.push_back(runtime_address + instruction.get_branch_displacement());
		}

		// ret function end.
		if (instruction.get_category() == XED_CATEGORY_RET)
		{
			break;
		}
	}

	for (auto relbr_address : relbrs)
	{
		if (!read_function_instructions_x64(relbr_address, instructions))
		{
			//printf("already instruction %p\n", relbr_address);
		}
	}

	return true;
}

xed_bool_t read_function_instructions(xed_uint64_t function_start, std::vector<xed_instruction> &instructions, xed_bool_t is_wow64_process)
{
	xed_bool_t status;

	if (is_wow64_process)
		status = read_function_instructions_x86(function_start, instructions);
	else
		status = read_function_instructions_x64(function_start, instructions);

	return status;
}

xed_bool_t read_function_no_return_instructions(xed_uint64_t function_start, std::vector<xed_instruction> &instructions)
{
	xed_uint64_t runtime_address = function_start;

	// duplicate check
	{
		auto it = std::find_if(instructions.begin(), instructions.end(), xed_instruction::find_address(runtime_address));
		if (it != instructions.end())
		{
			return false;
		}
	}

	std::vector<xed_uint64_t> relbrs;

	while (true)
	{
		xed_instruction instruction;

		xed_error_enum_t xed_error = instruction.decode(runtime_address, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b);
		if (xed_error != XED_ERROR_NONE)
		{
			return false;
		}

		// add instruction to list
		instructions.push_back(instruction);

		// next instruction
		runtime_address += instruction.get_length();

		// rel je/jne/jae/jbe/jnbe/jnae/jmp... etc
		auto find_relbr = [](
			xed_instruction *p
			) -> xed_bool_t
		{
			auto operands = p->get_operands();
			if (operands.size() >= 2
				&& operands[0].get_name() == XED_OPERAND_RELBR
				&& p->get_register(operands[1].get_name()) == XED_REG_RIP)
			{
				return true;
			}

			return false;
		};

		if (find_relbr(&instruction))
		{
			// jmp
			auto byte_array = instruction.get_bytes();
			if ((byte_array.size() >= 2)
				&& (byte_array[0] == 0xEB)
				&& (byte_array[1] >= 0x80 && byte_array[1] <= 0xFF))
			{
				xed_uint64_t relbr_addr = instruction.get_runtime_address() + instruction.get_branch_displacement();
				xed_uint64_t next_inst_addr = instruction.get_runtime_address() + instruction.get_length();

				if (relbr_addr < next_inst_addr)
				{
					auto it = std::find_if(instructions.begin(), instructions.end(), xed_instruction::find_address(relbr_addr));

					while (it != instructions.end())
					{
						if (find_relbr(&(*it)) && ((it->get_runtime_address() + it->get_branch_displacement()) >= next_inst_addr))
						{
							return true;
						}
						it++;
					}
				}
			}

			relbrs.push_back(runtime_address + instruction.get_branch_displacement());
		}

		// ret function end.
		if (instruction.get_category() == XED_CATEGORY_RET)
		{
			break;
		}
	}

	for (auto relbr_address : relbrs)
	{
		if (!read_function_instructions_x64(relbr_address, instructions))
		{
			//printf("already instruction %p\n", relbr_address);
		}
	}

	return true;
}