#include "pch.h"

#include "xed_instruction.hpp"
#include "xed_operand.hpp"

xed_instruction::xed_instruction()
{
	this->m_inst = nullptr;
	this->m_runtime_address = 0;
}

xed_instruction::~xed_instruction()
{
}

xed_error_enum_t xed_instruction::decode(xed_uint64_t runtime_address, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width)
{
	xed_decoded_inst_zero(&this->m_decoded_inst);
	xed_decoded_inst_set_mode(&this->m_decoded_inst, mmode, stack_addr_width);

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];
	xed_uint_t bufsize = XED_MAX_INSTRUCTION_BYTES;
	memcpy((void *)buf, (void *)runtime_address, bufsize);

	xed_error_enum_t xed_error = xed_decode(&this->m_decoded_inst, buf, bufsize);
	if (xed_decoded_inst_valid(&this->m_decoded_inst) == 0)
		return xed_error;

	this->m_inst = xed_decoded_inst_inst(&this->m_decoded_inst);
	this->m_runtime_address = runtime_address;

	for (xed_uint_t i = 0; i < this->get_length(); i++)
		this->m_dec_bytes.push_back(this->m_decoded_inst._byte_array._dec[i]);

	return XED_ERROR_NONE;
}

xed_error_enum_t xed_instruction::decode(xed_uint64_t runtime_address, xed_uint_t read_bytes, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width)
{
	xed_decoded_inst_zero(&this->m_decoded_inst);
	xed_decoded_inst_set_mode(&this->m_decoded_inst, mmode, stack_addr_width);

	xed_uint8_t buf[XED_MAX_INSTRUCTION_BYTES];
	xed_uint_t bufsize = read_bytes;
	if (bufsize > XED_MAX_INSTRUCTION_BYTES)
		bufsize = XED_MAX_INSTRUCTION_BYTES;
	memcpy((void *)buf, (void *)runtime_address, bufsize);

	xed_error_enum_t xed_error = xed_decode(&this->m_decoded_inst, buf, bufsize);
	if (xed_decoded_inst_valid(&this->m_decoded_inst) == 0)
		return xed_error;

	this->m_inst = xed_decoded_inst_inst(&this->m_decoded_inst);
	this->m_runtime_address = runtime_address;

	for (xed_uint_t i = 0; i < this->get_length(); i++)
		this->m_dec_bytes.push_back(this->m_decoded_inst._byte_array._dec[i]);

	return XED_ERROR_NONE;
}

xed_error_enum_t xed_instruction::decode(xed_uint8_t* buf, xed_uint_t read_bytes, xed_machine_mode_enum_t mmode, xed_address_width_enum_t stack_addr_width)
{
	xed_decoded_inst_zero(&this->m_decoded_inst);
	xed_decoded_inst_set_mode(&this->m_decoded_inst, mmode, stack_addr_width);

	xed_error_enum_t xed_error = xed_decode(&this->m_decoded_inst, buf, read_bytes);
	if (xed_decoded_inst_valid(&this->m_decoded_inst) == 0)
		return xed_error;

	this->m_inst = xed_decoded_inst_inst(&this->m_decoded_inst);
	this->m_runtime_address = (xed_uint64_t)buf;

	for (xed_uint_t i = 0; i < this->get_length(); i++)
		this->m_dec_bytes.push_back(this->m_decoded_inst._byte_array._dec[i]);

	return XED_ERROR_NONE;
}

void xed_instruction::set_runtime_address(xed_uint64_t runtime_address)
{
	this->m_runtime_address = runtime_address;
}

xed_uint64_t xed_instruction::get_runtime_address()
{
	return this->m_runtime_address;
}

/* Instruction information related methods. */
xed_isa_set_enum_t xed_instruction::get_isa_set()
{
	return xed_decoded_inst_get_isa_set(&this->m_decoded_inst);
}

xed_category_enum_t xed_instruction::get_category()
{
	return xed_decoded_inst_get_category(&this->m_decoded_inst);
}

xed_extension_enum_t xed_instruction::get_extension()
{
	return xed_decoded_inst_get_extension(&this->m_decoded_inst);
}

xed_iclass_enum_t xed_instruction::get_iclass()
{
	return xed_decoded_inst_get_iclass(&this->m_decoded_inst);
}

xed_iform_enum_t xed_instruction::get_iform()
{
	return xed_decoded_inst_get_iform_enum(&this->m_decoded_inst);
}

xed_uint_t xed_instruction::get_attribute(xed_attribute_enum_t attribute)
{
	return xed_decoded_inst_get_attribute(&this->m_decoded_inst, attribute);
}

xed_uint_t xed_instruction::get_length()
{
	return xed_decoded_inst_get_length(&this->m_decoded_inst);
}

xed_bool_t xed_instruction::conditionally_writes_registers()
{
	return xed_decoded_inst_conditionally_writes_registers(&this->m_decoded_inst);
}

xed_uint8_t xed_instruction::get_byte(xed_uint_t byte_index)
{
	return this->m_dec_bytes[byte_index];
}

std::vector<xed_uint8_t> xed_instruction::get_bytes()
{
	return this->m_dec_bytes;
}

std::string xed_instruction::dump_intel_format()
{
	char buf[64];
	xed_format_context(XED_SYNTAX_INTEL, &this->m_decoded_inst,
		buf, sizeof(buf) - 1, this->m_runtime_address, nullptr, nullptr);

	return std::string(buf);
}

/* EFLAGS/RFLAGS information related methods. */
xed_bool_t xed_instruction::uses_rflags()
{
	return xed_decoded_inst_uses_rflags(&this->m_decoded_inst);
}

xed_flag_set_t xed_instruction::get_rflags_read()
{
	const xed_simple_flag_t *flags;

	flags = xed_decoded_inst_get_rflags_info(&this->m_decoded_inst);
	if (flags != nullptr)
		return flags->read;

	return xed_flag_set_t();
}

xed_flag_set_t xed_instruction::get_rflags_undefined()
{
	const xed_simple_flag_t *flags;

	flags = xed_decoded_inst_get_rflags_info(&this->m_decoded_inst);
	if (flags != nullptr)
		return flags->undefined;

	return xed_flag_set_t();
}

xed_flag_set_t xed_instruction::get_rflags_written()
{
	const xed_simple_flag_t *flags;

	flags = xed_decoded_inst_get_rflags_info(&this->m_decoded_inst);
	if (flags != nullptr)
		return flags->written;

	return xed_flag_set_t();
}

/* Branch displacement related methods (getters & setters). */
xed_int32_t xed_instruction::get_branch_displacement()
{
	return xed_decoded_inst_get_branch_displacement(&this->m_decoded_inst);
}

xed_uint_t xed_instruction::get_branch_displacement_width()
{
	return xed_decoded_inst_get_branch_displacement_width(&this->m_decoded_inst);
}

xed_uint_t xed_instruction::get_branch_displacement_width_bits()
{
	return xed_decoded_inst_get_branch_displacement_width_bits(&this->m_decoded_inst);
}

void xed_instruction::set_branch_displacement(xed_int32_t disp, xed_uint_t length_bytes)
{
	xed_decoded_inst_set_branch_displacement(&this->m_decoded_inst, disp, length_bytes);
}

void xed_instruction::set_branch_displacement_bits(xed_int32_t disp, xed_uint_t length_bits)
{
	xed_decoded_inst_set_branch_displacement_bits(&this->m_decoded_inst, disp, length_bits);
}

/* Memory displacement related methods (getters & setters). */
xed_int64_t xed_instruction::get_memory_displacement(unsigned int mem_idx)
{
	xed_uint_t memops;

	memops = xed_decoded_inst_number_of_memory_operands(&this->m_decoded_inst);
	if (mem_idx >= memops)
		return -1;

	return xed_decoded_inst_get_memory_displacement(&this->m_decoded_inst, mem_idx);
}

xed_uint_t xed_instruction::get_memory_displacement_width(unsigned int mem_idx)
{
	xed_uint_t memops;

	memops = xed_decoded_inst_number_of_memory_operands(&this->m_decoded_inst);
	if (mem_idx >= memops)
		return -1;

	return xed_decoded_inst_get_memory_displacement_width(&this->m_decoded_inst, mem_idx);
}

xed_uint_t xed_instruction::get_memory_displacement_width_bits(unsigned int mem_idx)
{
	xed_uint_t memops;

	memops = xed_decoded_inst_number_of_memory_operands(&this->m_decoded_inst);
	if (mem_idx >= memops)
		return -1;

	return xed_decoded_inst_get_memory_displacement_width_bits(&this->m_decoded_inst, mem_idx);
}

void xed_instruction::set_memory_displacement(xed_int64_t disp, xed_uint_t length_bytes)
{
	xed_decoded_inst_set_memory_displacement(&this->m_decoded_inst, disp, length_bytes);
}

void xed_instruction::set_memory_displacement_bits(xed_int64_t disp, xed_uint_t length_bits)
{
	xed_decoded_inst_set_memory_displacement_bits(&this->m_decoded_inst, disp, length_bits);
}

/* Operand information related methods. */
unsigned int xed_instruction::get_noperands()
{
	return xed_decoded_inst_noperands(&this->m_decoded_inst);
}

xed_operand xed_instruction::get_operand(unsigned int i)
{
	if (i >= this->get_noperands())
		return xed_operand(nullptr);

	return xed_operand(xed_inst_operand(this->m_inst, i));
}

std::vector<xed_operand> xed_instruction::get_operands()
{
	unsigned int nope = this->get_noperands();
	std::vector<xed_operand> operands;

	for (unsigned int i = 0; i < nope; i++)
		operands.push_back(xed_operand(xed_inst_operand(this->m_inst, i)));

	return operands;
}

unsigned int xed_instruction::get_operand_length(unsigned int operand_index)
{
	if (operand_index >= this->get_noperands())
		return -1;

	return xed_decoded_inst_operand_length(&this->m_decoded_inst, operand_index);
}

unsigned int xed_instruction::get_operand_length_bits(unsigned int operand_index)
{
	if (operand_index >= this->get_noperands())
		return -1;

	return xed_decoded_inst_operand_length_bits(&this->m_decoded_inst, operand_index);
}

xed_reg_enum_t xed_instruction::get_register(xed_operand_enum_t reg_operand)
{
	if (reg_operand <= XED_OPERAND_INVALID || reg_operand >= XED_OPERAND_LAST)
	{
		return XED_REG_INVALID;
	}

	return xed_decoded_inst_get_reg(&this->m_decoded_inst, reg_operand);
}

/* Immediate information related methods. */
xed_bool_t xed_instruction::is_immediate_signed()
{
	return xed_decoded_inst_get_immediate_is_signed(&this->m_decoded_inst);
}

xed_uint_t xed_instruction::get_immediate_width()
{
	return xed_decoded_inst_get_immediate_width(&this->m_decoded_inst);
}

xed_uint_t xed_instruction::get_immediate_width_bits()
{
	return xed_decoded_inst_get_immediate_width_bits(&this->m_decoded_inst);
}

xed_uint8_t xed_instruction::get_second_immediate()
{
	return xed_decoded_inst_get_second_immediate(&this->m_decoded_inst);
}

xed_int32_t xed_instruction::get_signed_immediate()
{
	return xed_decoded_inst_get_signed_immediate(&this->m_decoded_inst);
}

xed_uint64_t xed_instruction::get_unsigned_immediate()
{
	return xed_decoded_inst_get_unsigned_immediate(&this->m_decoded_inst);
}

/* Memory operand related methods. */
xed_uint_t xed_instruction::get_number_of_memory_operands()
{
	return xed_decoded_inst_number_of_memory_operands(&this->m_decoded_inst);
}

xed_reg_enum_t xed_instruction::get_seg_register(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return XED_REG_INVALID;

	return xed_decoded_inst_get_seg_reg(&this->m_decoded_inst, mem_idx);
}

std::vector<xed_reg_enum_t> xed_instruction::get_seg_registers()
{
	unsigned int mem_opes;
	std::vector<xed_reg_enum_t> seg_registers;

	mem_opes = this->get_number_of_memory_operands();
	for (unsigned int i = 0; i < mem_opes; i++)
		seg_registers.push_back(xed_decoded_inst_get_seg_reg(&this->m_decoded_inst, i));

	return seg_registers;
}

xed_reg_enum_t xed_instruction::get_base_register(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return XED_REG_INVALID;

	return xed_decoded_inst_get_base_reg(&this->m_decoded_inst, mem_idx);
}

xed_reg_enum_t xed_instruction::get_index_register(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return XED_REG_INVALID;

	return xed_decoded_inst_get_index_reg(&this->m_decoded_inst, mem_idx);
}

unsigned int xed_instruction::get_memop_address_width(unsigned int memop_idx)
{
	if (memop_idx >= this->get_number_of_memory_operands())
		return -1;

	return xed_decoded_inst_get_memop_address_width(&this->m_decoded_inst, memop_idx);
}

unsigned int xed_instruction::get_memory_operand_length(unsigned int memop_idx)
{
	if (memop_idx >= this->get_number_of_memory_operands())
		return -1;

	return xed_decoded_inst_get_memory_operand_length(&this->m_decoded_inst, memop_idx);
}

xed_bool_t xed_instruction::is_mem_read(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return 0;

	return xed_decoded_inst_mem_read(&this->m_decoded_inst, mem_idx);
}

xed_bool_t xed_instruction::is_mem_written(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return 0;

	return xed_decoded_inst_mem_written(&this->m_decoded_inst, mem_idx);
}

xed_bool_t xed_instruction::is_mem_written_only(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return 0;

	return xed_decoded_inst_mem_written_only(&this->m_decoded_inst, mem_idx);
}

xed_uint_t xed_instruction::get_scale(unsigned int mem_idx)
{
	if (mem_idx >= this->get_number_of_memory_operands())
		return -1;

	return xed_decoded_inst_get_scale(&this->m_decoded_inst, mem_idx);
}