#include "code.h"
#include "command_type.h"

using namespace vm;

code::code(const std::string &file)
	: m_file(file, std::ofstream::out)
{
	m_command_function = {
		{ "constant", &code::push_pop_constant },
	};

	m_arithmetic_function = {
		{ "add", &code::arithmetic_add },
	};
}

code::~code()
{
	if (m_file.is_open())
		m_file.close();
}

void code::write_arithmetic(const std::string &cmd)
{
	auto func = m_arithmetic_function[cmd];
	if (func)
		(this->*func)();
}

void code::write_push_pop(command_type cmd, const std::string &segment, uint16_t index)
{
	switch(cmd) {
	case command_type::c_push: {
		auto func = m_command_function[segment];
		if (func)
			(this->*func)(cmd, index);

		// increase stack pointer
		m_file << "@SP" << std::endl;
		m_file << "M=M+1" << std::endl;
		break;
	}
	case command_type::c_pop: {
		auto func = m_command_function[segment];
		if (func)
			(this->*func)(cmd, index);

		// decrease stack pointer
		m_file << "@SP" << std::endl;
		m_file << "M=M-1" << std::endl;
		break;
	}
	default:
		return;
	}
}

/************** Commands **************/

// The constant segment can be only used for push commands.
void code::push_pop_constant(command_type cmd, uint16_t index)
{
	m_file << "@" << index << std::endl;
	m_file << "D=A" << std::endl;
	m_file << "@SP" << std::endl;
	m_file << "A=M" << std::endl;
	m_file << "M=D" << std::endl;
}

/************** Arithmetics **************/

void code::arithmetic_add()
{
	m_file << "@SP" << std::endl;
	m_file << "AM=M-1" << std::endl;
	m_file << "D=M" << std::endl;
	m_file << "A=A-1" << std::endl;
	m_file << "M=D+M" << std::endl;
}
