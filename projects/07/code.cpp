#include "code.h"
#include "command_type.h"

#include <fstream>
#include <map>

using namespace vm;

class code_p {
public:
	code_p(const std::string &file);
	~code_p();

	void eval_push_pop(vm::command_type cmd, const std::string &segment, uint16_t index);
	void eval_arithmetic(const std::string &cmd);

	void sp_inc();
	void sp_dec();

private:
	using command_function = void(code_p::*)(vm::command_type, uint16_t);
	using arithmetic_function = void(code_p::*)();

	std::ofstream m_file;
	std::map<std::string, command_function> m_push_pop_function;
	std::map<std::string, arithmetic_function> m_arithmetic_function;

	void push_pop_constant(vm::command_type cmd, uint16_t index);

	void arithmetic_add();
	void arithmetic_sub();
	void arithmetic_neg();
	void arithmetic_eq();
	void arithmetic_gt();
	void arithmetic_lt();
	void arithmetic_and();
	void arithmetic_or();
	void arithmetic_not();
};

code::code(const std::string &file)
	: m_p(new ::code_p(file))
{
}

code::~code() = default;

void code::write_arithmetic(const std::string &cmd)
{
	m_p->eval_arithmetic(cmd);
}

void code::write_push_pop(command_type cmd, const std::string &segment, uint16_t index)
{
	switch(cmd) {
	case command_type::c_push: {
		m_p->eval_push_pop(cmd, segment, index);
		m_p->sp_inc();
		break;
	}
	case command_type::c_pop: {
		m_p->eval_push_pop(cmd, segment, index);
		m_p->sp_dec();
		break;
	}
	default:
		return;
	}
}

/************** Private Class **************/

code_p::code_p(const std::string &file)
	: m_file(file, std::ofstream::out)
{
	m_push_pop_function = {
		{ "constant", &code_p::push_pop_constant },
	};

	m_arithmetic_function = {
		{ "add", &code_p::arithmetic_add },
		{ "sub", &code_p::arithmetic_sub },
		{ "neg", &code_p::arithmetic_neg },
		{ "eq", &code_p::arithmetic_eq },
		{ "gt", &code_p::arithmetic_gt },
		{ "lt", &code_p::arithmetic_lt },
		{ "and", &code_p::arithmetic_and },
		{ "or", &code_p::arithmetic_or },
		{ "not", &code_p::arithmetic_not },
	};
}

code_p::~code_p()
{
	if (m_file.is_open())
		m_file.close();
}

void code_p::eval_push_pop(vm::command_type cmd, const std::string &segment, uint16_t index)
{
	auto func = m_push_pop_function[segment];
	if (func)
		(this->*func)(cmd, index);
}

void code_p::eval_arithmetic(const std::string &cmd)
{
	auto func = m_arithmetic_function[cmd];
	if (func)
		(this->*func)();
}

void code_p::sp_inc()
{
	m_file << "@SP" << std::endl;
	m_file << "M=M+1" << std::endl;
}

void code_p::sp_dec()
{
	m_file << "@SP" << std::endl;
	m_file << "M=M-1" << std::endl;
}

/************** Commands **************/

// The constant segment can be only used for push commands.
void code_p::push_pop_constant(command_type cmd, uint16_t index)
{
	m_file << "@" << index << std::endl;
	m_file << "D=A" << std::endl;
	m_file << "@SP" << std::endl;
	m_file << "A=M" << std::endl;
	m_file << "M=D" << std::endl;
}

/************** Arithmetics **************/

/**
 * Stack arithmetic operations:
 * -------
 * | ... |
 * -------
 * |  x  |
 * -------
 * |  Y  |
 * -------
 * |     | <- SP
 * -------
 */

// X + Y
void code_p::arithmetic_add()
{
	m_file << "@SP" << std::endl;
	m_file << "AM=M-1" << std::endl;
	m_file << "D=M" << std::endl;
	m_file << "A=A-1" << std::endl;
	m_file << "M=D+M" << std::endl;
}

// X - Y
void code_p::arithmetic_sub()
{
	m_file << "@SP" << std::endl;
	m_file << "AM=M-1" << std::endl;
	m_file << "D=M" << std::endl;
	m_file << "A=A-1" << std::endl;
	m_file << "M=M-D" << std::endl;
}

// -Y
void code_p::arithmetic_neg()
{
	m_file << "@SP" << std::endl;
	m_file << "A=M-1" << std::endl;
	m_file << "M=-M" << std::endl;
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X == Y)
void code_p::arithmetic_eq()
{
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X > Y)
void code_p::arithmetic_gt()
{
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X < Y)
void code_p::arithmetic_lt()
{
}

// X AND Y
void code_p::arithmetic_and()
{
	m_file << "@SP" << std::endl;
	m_file << "AM=M-1" << std::endl;
	m_file << "D=M" << std::endl;
	m_file << "A=A-1" << std::endl;
	m_file << "M=D&M" << std::endl;
}

// X OR Y
void code_p::arithmetic_or()
{
	m_file << "@SP" << std::endl;
	m_file << "AM=M-1" << std::endl;
	m_file << "D=M" << std::endl;
	m_file << "A=A-1" << std::endl;
	m_file << "M=D|M" << std::endl;
}

// NOT Y
void code_p::arithmetic_not()
{
	m_file << "@SP" << std::endl;
	m_file << "A=M-1" << std::endl;
	m_file << "M=!M" << std::endl;
}
