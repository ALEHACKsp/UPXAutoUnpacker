#pragma once

#include "xed_interface_c.hpp"

class xed_decoder
{
public:
	xed_decoder(xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width);
	xed_decoder();
	~xed_decoder();

	void set_mode(xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width);
	xed_decoded_inst_t *decode(xed_uint64_t runtime_address);

private:
	xed_uint64_t m_runtime_address;
	xed_machine_mode_enum_t m_mmode;
	xed_address_width_enum_t m_stack_addr_width;
};