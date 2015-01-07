#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <ostream>

class code_p; // private class

namespace vm {
	enum class command_type;

	class code {
	public:
		code(const std::string &file, std::ostream &os);
		~code();

		void write_arithmetic(const std::string &cmd);
		void write_push_pop(vm::command_type cmd, const std::string &segment, uint16_t index);
		void write_label_command(vm::command_type cmd, const std::string &label);

	private:
		std::unique_ptr<code_p> m_p;
	};
} // namespace vm
