#include "pch.h"

#include "xed_operand.hpp"

xed_operand::xed_operand(const xed_operand_t *operand)
{
	this->m_operand = operand;
}

xed_operand::~xed_operand()
{
}

xed_uint32_t xed_operand::get_imm()
{
	return xed_operand_imm(this->m_operand);
}

xed_operand_enum_t xed_operand::get_name()
{
	return xed_operand_name(this->m_operand);
}

xed_operand_visibility_enum_t xed_operand::get_visibility()
{
	return xed_operand_operand_visibility(this->m_operand);
}

xed_operand_type_enum_t xed_operand::get_type()
{
	return xed_operand_type(this->m_operand);
}

xed_operand_element_xtype_enum_t xed_operand::get_xtype()
{
	return xed_operand_xtype(this->m_operand);
}

xed_operand_width_enum_t xed_operand::get_width()
{
	return xed_operand_width(this->m_operand);
}

xed_uint32_t xed_operand::get_width_bits(const xed_uint32_t eosz)
{
	return xed_operand_width_bits(this->m_operand, eosz);
}

xed_bool_t xed_operand::is_memory_addressing_register()
{
	return xed_operand_is_memory_addressing_register(this->get_name());
}

xed_bool_t xed_operand::is_register()
{
	return xed_operand_is_register(this->get_name());
}

xed_bool_t xed_operand::is_conditional_read()
{
	return xed_operand_conditional_read(this->m_operand);
}

xed_bool_t xed_operand::is_conditional_write()
{
	return xed_operand_conditional_write(this->m_operand);
}

xed_bool_t xed_operand::is_read()
{
	return xed_operand_read(this->m_operand);
}

xed_bool_t xed_operand::is_read_and_written()
{
	return xed_operand_read_and_written(this->m_operand);
}

xed_bool_t xed_operand::is_read_only()
{
	return xed_operand_read_only(this->m_operand);
}

xed_bool_t xed_operand::is_written()
{
	return xed_operand_written(this->m_operand);
}

xed_bool_t xed_operand::is_written_only()
{
	return xed_operand_written_only(this->m_operand);
}

xed_operand_action_enum_t xed_operand::get_rw_action()
{
	return xed_operand_rw(this->m_operand);
}

std::string xed_operand::dump()
{
	char buf[64];
	xed_operand_print(this->m_operand, buf, sizeof(buf) - 1);
	return std::string(buf);
}