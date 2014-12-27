#include "parser.h"
#include "command_type.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace vm;

parser::parser(const std::string &file)
	: m_file(file, std::ifstream::in),
	  m_command_type(command_type::none)
{
	m_command_lookup = {
		{ "add",      command_type::c_arithmetic },
		{ "sub",      command_type::c_arithmetic },
		{ "neg",      command_type::c_arithmetic },
		{ "eq",       command_type::c_arithmetic },
		{ "gt",       command_type::c_arithmetic },
		{ "lt",       command_type::c_arithmetic },
		{ "and",      command_type::c_arithmetic },
		{ "or",       command_type::c_arithmetic },
		{ "not",      command_type::c_arithmetic },
		{ "push",     command_type::c_push },
		{ "pop",      command_type::c_pop },
		{ "label",    command_type::c_label },
		{ "goto",     command_type::c_goto },
		{ "if-goto",  command_type::c_if },
		{ "function", command_type::c_function },
		{ "return",   command_type::c_return },
		{ "call",     command_type::c_call },
	};
}

parser::~parser()
{
	if (m_file.is_open())
		m_file.close();
}

bool parser::has_more_commands() const
{
	return m_file.good();
}

void parser::advance()
{
	std::string line;
	m_command_type = command_type::none;
	do {
		std::getline(m_file >> std::ws, line);

		if (line.empty())
			continue;

		// remove comments from string
		std::string::size_type comment_pos = line.find("//");

		// check if line has comments
		if (comment_pos != 0) {

			// remove comments if after command
			if (comment_pos != std::string::npos)
				line.erase(comment_pos);

			boost::split(m_token, line, boost::is_any_of(" \r\n"));

			m_command_type = m_command_lookup[m_token[0]];
		}
	} while (m_file.good() && m_command_type == command_type::none);

	// line.insert(0, "\"");
	// line.insert(line.size()-1, "\"");

	// std::cout << line << std::endl;
}

command_type parser::command() const
{
	return m_command_type;
}

std::string parser::arg1() const
{
	switch (m_command_type) {
	case command_type::none:
	case command_type::c_return:
		return std::string("");
		break;
	case command_type::c_arithmetic:
		return m_token[0];
		break;
	default:
		return m_token[1];
	}
}

uint16_t parser::arg2() const
{
	switch (m_command_type) {
	case command_type::c_push:
	case command_type::c_pop:
	case command_type::c_function:
	case command_type::c_call:
		return std::stoi(m_token[2]);
	default:
		return -1;
	}
}
