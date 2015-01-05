#include "code.h"
#include "command_type.h"

#include <fstream>
#include <sstream>
#include <map>
#include <cctype>
#include <algorithm>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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
	std::size_t m_label_count;
	std::string m_label_static_name;

	// write assembly
	void w(const std::string &command);

	// helper functions
	void comp_to_stack(const std::string &comp);
	void stack_to_dest(const std::string &dest);
	void seg_to_desc(const std::string &dest, const std::string &label, uint16_t index);
	void load_seg(const std::string &label, uint16_t index);
	void load_constant(uint16_t constant);
	void comp_to_reg(const std::string &comp, const std::string &reg);
	void reg_to_dest(const std::string &dest, const std::string &reg);

	void push_pop_seg(const std::string &seg, vm::command_type cmd, uint16_t index);
	void push_pop_reg(const std::string &reg, command_type cmd, uint16_t index);
	void push_pop_constant(vm::command_type cmd, uint16_t index);
	void push_pop_local(vm::command_type cmd, uint16_t index);
	void push_pop_argument(vm::command_type cmd, uint16_t index);
	void push_pop_this(vm::command_type cmd, uint16_t index);
	void push_pop_that(vm::command_type cmd, uint16_t index);
	void push_pop_temp(vm::command_type cmd, uint16_t index);
	void push_pop_pointer(vm::command_type cmd, uint16_t index);
	void push_pop_static(vm::command_type cmd, uint16_t index);

	void arithmetic_add();
	void arithmetic_sub();
	void arithmetic_neg();
	void arithmetic_eq();
	void arithmetic_gt();
	void arithmetic_lt();
	void arithmetic_and();
	void arithmetic_or();
	void arithmetic_not();

	std::string label_create();
	template<typename T>
	void label_add(const T &label);
	template<typename T>
	void label_at(const T &label);
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
		m_p->sp_dec();
		m_p->eval_push_pop(cmd, segment, index);
		break;
	}
	default:
		BOOST_ASSERT_MSG(false, "Wrong command_type for push_pop functions.");
		return;
	}
}

/************** Private Class **************/

code_p::code_p(const std::string &file)
	: m_file(file, std::ofstream::out),
	  m_label_count(0)
{
	fs::path p(file);
	m_label_static_name = "STATIC" + p.stem().string();
	m_label_static_name.erase(std::remove_if(m_label_static_name.begin(), m_label_static_name.end(), ::isspace),
	                          m_label_static_name.end());

	m_push_pop_function = {
		{ "constant", &code_p::push_pop_constant },
		{ "local", &code_p::push_pop_local },
		{ "argument", &code_p::push_pop_argument },
		{ "this", &code_p::push_pop_this },
		{ "that", &code_p::push_pop_that },
		{ "temp", &code_p::push_pop_temp },
		{ "pointer", &code_p::push_pop_pointer },
		{ "static", &code_p::push_pop_static },
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

inline void code_p::sp_inc()
{
	label_at("SP");
	w("M=M+1");
}

inline void code_p::sp_dec()
{
	label_at("SP");
	w("M=M-1");
}

// Private API

inline void code_p::w(const std::string &command)
{
	m_file << command << std::endl;
}

/************** Commands **************/

inline void code_p::comp_to_stack(const std::string &comp)
{
	label_at("SP");
	w("A=M");
	w("M=" + comp);
}

inline void code_p::stack_to_dest(const std::string &dest)
{
	label_at("SP");
	w("A=M");
	w(dest + "=M");
}

inline void code_p::seg_to_desc(const std::string &dest, const std::string &label, uint16_t index)
{
	load_constant(index);
	label_at(label);
	w("A=D+M");
	w(dest + "=M");
}

inline void code_p::load_seg(const std::string &label, uint16_t index)
{
	load_constant(index);
	label_at(label);
	w("D=D+M");
}

inline void code_p::load_constant(uint16_t constant)
{
	label_at(constant);
	w("D=A");
}

inline void code_p::comp_to_reg(const std::string &comp, const std::string &reg)
{
	label_at(reg);
	w("M=" + comp);
}

inline void code_p::reg_to_dest(const std::string &dest, const std::string &reg)
{
	label_at(reg);
	w(dest + "=M");
}

/************** Push and Pop **************/

void code_p::push_pop_seg(const std::string &seg, command_type cmd, uint16_t index)
{
	switch (cmd) {
	case command_type::c_push:
		seg_to_desc("D", seg, index);
		comp_to_stack("D");
		break;
	case command_type::c_pop:
		load_seg(seg, index);
		comp_to_reg("D", "R13");
		stack_to_dest("D");
		reg_to_dest("A", "R13");
		w("M=D");
		break;
	default:
		BOOST_ASSERT_MSG(false, "Wrong command_type for push_pop functions.");
	}
}

void code_p::push_pop_reg(const std::string &reg, command_type cmd, uint16_t index)
{
	switch (cmd) {
	case command_type::c_push:
		load_seg(reg, 0);
		comp_to_stack("D");
		break;
	case command_type::c_pop:
		stack_to_dest("D");
		label_at(reg);
		w("M=D");
		break;
	default:
		BOOST_ASSERT_MSG(false, "Wrong command_type for push_pop functions.");
	}
}

// The constant segment can be only used for push commands.
inline void code_p::push_pop_constant(command_type cmd, uint16_t index)
{
	load_constant(index);
	comp_to_stack("D");
}

void code_p::push_pop_local(command_type cmd, uint16_t index)
{
	push_pop_seg("LCL", cmd, index);
}

void code_p::push_pop_argument(command_type cmd, uint16_t index)
{
	push_pop_seg("ARG", cmd, index);
}

void code_p::push_pop_this(command_type cmd, uint16_t index)
{
	push_pop_seg("THIS", cmd, index);
}

void code_p::push_pop_that(command_type cmd, uint16_t index)
{
	push_pop_seg("THAT", cmd, index);
}

void code_p::push_pop_temp(command_type cmd, uint16_t index)
{
	push_pop_reg("R" + std::to_string(5 + index), cmd, index);
}

void code_p::push_pop_pointer(command_type cmd, uint16_t index)
{
	push_pop_reg("R" + std::to_string(3 + index), cmd, index);
}

void code_p::push_pop_static(command_type cmd, uint16_t index)
{
	std::string static_label = m_label_static_name + std::to_string(index);

	if (cmd == command_type::c_push)
		label_at(static_label);

	push_pop_reg(static_label, cmd, index);
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
inline void code_p::arithmetic_add()
{
	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("M=D+M");
}

// X - Y
inline void code_p::arithmetic_sub()
{
	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("M=M-D");
}

// -Y
inline void code_p::arithmetic_neg()
{
	label_at("SP");
	w("A=M-1");
	w("M=-M");
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X == Y)
inline void code_p::arithmetic_eq()
{
	std::string eq_label = label_create();
	std::string neq_label = label_create();
	std::string end_label = label_create();

	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("D=M-D");

	label_at(eq_label);
	w("D;JEQ");

	label_at(neq_label);
	w("D;JNE");

	label_add(eq_label);
	label_at("SP");
	w("A=M-1");
	w("M=-1");
	label_at(end_label);
	w("0;JMP");

	label_add(neq_label);
	label_at("SP");
	w("A=M-1");
	w("M=0");

	label_add(end_label);
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X > Y)
inline void code_p::arithmetic_gt()
{
	std::string gt_label = label_create();
	std::string le_label = label_create();
	std::string end_label = label_create();

	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("D=M-D");

	label_at(gt_label);
	w("D;JGT");

	label_at(le_label);
	w("D;JLE");

	label_add(gt_label);
	label_at("SP");
	w("A=M-1");
	w("M=-1");
	label_at(end_label);
	w("0;JMP");

	label_add(le_label);
	label_at("SP");
	w("A=M-1");
	w("M=0");

	label_add(end_label);
}

// 0xffff (-1) is true, and 0x0000 is false
// X = (X < Y)
inline void code_p::arithmetic_lt()
{
	std::string lt_label = label_create();
	std::string ge_label = label_create();
	std::string end_label = label_create();

	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("D=M-D");

	label_at(lt_label);
	w("D;JLT");

	label_at(ge_label);
	w("D;JGE");

	label_add(lt_label);
	label_at("SP");
	w("A=M-1");
	w("M=-1");
	label_at(end_label);
	w("0;JMP");

	label_add(ge_label);
	label_at("SP");
	w("A=M-1");
	w("M=0");
	label_add(end_label);
}

// X AND Y
inline void code_p::arithmetic_and()
{
	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("M=D&M");
}

// X OR Y
inline void code_p::arithmetic_or()
{
	label_at("SP");
	w("AM=M-1");
	w("D=M");
	w("A=A-1");
	w("M=D|M");
}

// NOT Y
inline void code_p::arithmetic_not()
{
	label_at("SP");
	w("A=M-1");
	w("M=!M");
}

/************** Label **************/

inline std::string code_p::label_create()
{
	return "VMLABEL" + std::to_string(m_label_count++);
}

template<typename T>
inline void code_p::label_add(const T &label)
{
	std::ostringstream oss;
	oss << "(" << label << ")";
	w(oss.str());
}

template<typename T>
inline void code_p::label_at(const T &label)
{
	std::ostringstream oss;
	oss << "@" << label;
	w(oss.str());
}
