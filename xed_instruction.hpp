#pragma once

#include "xed_interface_c.hpp"

#include <string>
#include <vector>

class xed_operand;

class xed_instruction
{
public:
	xed_instruction();
	~xed_instruction();

	operator xed_decoded_inst_t*() 
	{
		return &this->m_decoded_inst;
	}

	bool operator<(const xed_instruction &r) const 
	{
		return this->m_runtime_address < r.m_runtime_address;
	}

	struct find_address : std::unary_function<xed_instruction, bool> {
		xed_uint64_t runtime_address;

		find_address(xed_uint64_t runtime_address) : runtime_address(runtime_address) {}
		bool operator()(const xed_instruction &r) const
		{
			return this->runtime_address == r.m_runtime_address;
		}
	};

	/* Instruction decode */
	xed_error_enum_t decode(xed_uint64_t runtime_address, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width);
	xed_error_enum_t decode(xed_uint64_t runtime_address, xed_uint_t read_bytes, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width);
	xed_error_enum_t decode(xed_uint8_t* buf, xed_uint_t read_bytes, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width);

	void set_runtime_address(xed_uint64_t runtime_address);
	xed_uint64_t get_runtime_address();

	/* Instruction information related methods. */
	xed_isa_set_enum_t get_isa_set();
	xed_category_enum_t get_category();
	xed_extension_enum_t get_extension();
	xed_iclass_enum_t get_iclass();
	xed_iform_enum_t get_iform();
	xed_uint_t get_attribute(xed_attribute_enum_t attribute);
	xed_uint_t get_length();
	xed_bool_t conditionally_writes_registers();
	xed_uint8_t get_byte(xed_uint_t byte_index = 0);
	std::vector<xed_uint8_t> get_bytes();
	std::string dump_intel_format();

	/* EFLAGS/RFLAGS information related methods. */
	xed_bool_t uses_rflags();
	xed_flag_set_t get_rflags_read();
	xed_flag_set_t get_rflags_undefined();
	xed_flag_set_t get_rflags_written();

	/* Branch displacement related methods (getters & setters). */
	xed_int32_t get_branch_displacement();
	xed_uint_t get_branch_displacement_width();
	xed_uint_t get_branch_displacement_width_bits();
	void set_branch_displacement(xed_int32_t disp, xed_uint_t length_bytes);
	void set_branch_displacement_bits(xed_int32_t disp, xed_uint_t length_bits);

	/* Memory displacement related methods (getters & setters). */
	xed_int64_t get_memory_displacement(unsigned int mem_idx = 0);
	xed_uint_t get_memory_displacement_width(unsigned int mem_idx = 0);
	xed_uint_t get_memory_displacement_width_bits(unsigned int mem_idx = 0);
	void set_memory_displacement(xed_int64_t disp, xed_uint_t length_bytes);
	void set_memory_displacement_bits(xed_int64_t disp, xed_uint_t length_bits); 

	/* Operand information related methods. */
	unsigned int get_noperands();
	xed_operand get_operand(unsigned int i = 0);
	std::vector<xed_operand> get_operands();
	unsigned int get_operand_length(unsigned int operand_index = 0);
	unsigned int get_operand_length_bits(unsigned int operand_index = 0);
	xed_reg_enum_t get_register(xed_operand_enum_t reg_operand);

	/* Immediate information related methods. */
	xed_bool_t is_immediate_signed();
	xed_uint_t get_immediate_width();
	xed_uint_t get_immediate_width_bits();
	xed_uint8_t get_second_immediate();
	xed_int32_t get_signed_immediate();
	xed_uint64_t get_unsigned_immediate();

	/* Memory operand related methods. */
	xed_uint_t get_number_of_memory_operands();
	xed_reg_enum_t get_seg_register(unsigned int mem_idx = 0);
	std::vector<xed_reg_enum_t> get_seg_registers();
	xed_reg_enum_t get_base_register(unsigned int mem_idx = 0);
	xed_reg_enum_t get_index_register(unsigned int mem_idx = 0);
	unsigned int get_memop_address_width(unsigned int memop_idx = 0);
	unsigned int get_memory_operand_length(unsigned int memop_idx = 0);
	xed_bool_t is_mem_read(unsigned int mem_idx = 0);
	xed_bool_t is_mem_written(unsigned int mem_idx = 0);
	xed_bool_t is_mem_written_only(unsigned int mem_idx = 0);
	xed_uint_t get_scale(unsigned int mem_idx = 0);

private:
	xed_decoded_inst_t	m_decoded_inst;
	const xed_inst_t*	m_inst;
	xed_uint64_t		m_runtime_address;
	std::vector<xed_uint8_t> m_dec_bytes;
};