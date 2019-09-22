#pragma once

#include "xed_interface_c.hpp"

#include <string>

class xed_operand
{
public:
	xed_operand(const xed_operand_t *operand);
	~xed_operand();
	operator const xed_operand_t*() { return this->m_operand; }

	xed_uint32_t get_imm();
	xed_operand_enum_t get_name();
	xed_operand_visibility_enum_t get_visibility();
	xed_operand_type_enum_t get_type();
	xed_operand_element_xtype_enum_t get_xtype();
	xed_operand_width_enum_t get_width();
	xed_uint32_t get_width_bits(const xed_uint32_t eosz);

	xed_bool_t is_memory_addressing_register();
	xed_bool_t is_register();
	xed_bool_t is_conditional_read();
	xed_bool_t is_conditional_write();
	xed_bool_t is_read();
	xed_bool_t is_read_and_written();
	xed_bool_t is_read_only();
	xed_bool_t is_written();
	xed_bool_t is_written_only();

	xed_operand_action_enum_t get_rw_action();

	std::string dump();


private:
	const xed_operand_t *m_operand;
};