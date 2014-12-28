#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <map>
#include <functional>

namespace vm {
	enum class command_type;

	class code {
	public:
		code(const std::string &file);
		~code();

		void write_arithmetic(const std::string &cmd);
		void write_push_pop(vm::command_type cmd, const std::string &segment, uint16_t index);

	private:

		using command_function = void(code::*)(vm::command_type, uint16_t);
		using arithmetic_function = void(code::*)();

		std::ofstream m_file;

		std::map<std::string, command_function> m_command_function;
		std::map<std::string, arithmetic_function> m_arithmetic_function;

		void push_pop_constant(vm::command_type cmd, uint16_t index);

		void arithmetic_add();
		void arithmetic_sub();
		void arithmetic_neg();
	};
} // namespace vm
