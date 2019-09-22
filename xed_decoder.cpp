#include "pch.h"

#include "xed_decoder.hpp"
#include "xed_instruction.hpp"

xed_decoder::xed_decoder(xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width)
{
	this->set_mode(mmode, stack_addr_width);
	this->m_runtime_address = 0;
}

xed_decoder::xed_decoder()
{
	this->set_mode(XED_MACHINE_MODE_INVALID, XED_ADDRESS_WIDTH_INVALID);
	this->m_runtime_address = 0;
}

xed_decoder::~xed_decoder()
{
}

void xed_decoder::set_mode(xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width)
{
	this->m_mmode = mmode;
	this->m_stack_addr_width = stack_addr_width;
}

xed_decoded_inst_t *xed_decoder::decode(xed_uint64_t runtime_address)
{
	xed_decoded_inst_t *decoded_inst = new xed_decoded_inst_t;
	xed_decoded_inst_zero(decoded_inst);
	xed_decoded_inst_set_mode(decoded_inst, this->m_mmode, this->m_stack_addr_width);

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];
	xed_uint_t bufsize = XED_MAX_INSTRUCTION_BYTES;
	memcpy((void *)buf, (void *)runtime_address, bufsize);

	xed_decode(decoded_inst, buf, bufsize);
	if (xed_decoded_inst_valid(decoded_inst) == 0)
	{
		delete decoded_inst;
		return nullptr;
	}

	return decoded_inst;
}