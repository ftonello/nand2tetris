#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include <fstream>

namespace vm {
	enum class command_type;

	class parser {
	public:
		parser(const std::string &file);
		~parser();

		bool has_more_commands() const;
		void advance();

		vm::command_type command() const;
		std::string arg1() const;
		uint16_t arg2() const;

	private:
		std::ifstream m_file;
		std::string m_command;
		std::vector<std::string> m_token;
		vm::command_type m_command_type;
		std::map<std::string, vm::command_type> m_command_lookup;
	};
}
