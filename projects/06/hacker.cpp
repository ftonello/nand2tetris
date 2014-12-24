/**
 * Hacker is the Hack Assembler.
 *
 * To compile:
 * g++ hacker.cpp -std=c++11 -Wall -o hacker
 */

#include <cstdint>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <bitset>

namespace hacker {

	class symbol_table {
	public:
		void add_label(const std::string &symbol, uint16_t address)
		{
			m_symbol_table[symbol] = address;
		}

		void add_var(const std::string &symbol)
		{
			m_symbol_table[symbol] = m_var_address++;
		}

		bool contains(const std::string &symbol)
		{
			return m_symbol_table.find(symbol) != m_symbol_table.end();
		}

		uint16_t address(const std::string &symbol)
		{
			return m_symbol_table[symbol];
		}

	private:
		std::map<std::string, uint16_t> m_symbol_table = {
			{"0",      0x0000}, // workaround because stoi doesn't recognize 0
			{"SP",     0x0000},
			{"LCL",    0x0001},
			{"ARG",    0x0002},
			{"THIS",   0x0003},
			{"THAT",   0x0004},
			{"R0",     0x0000},
			{"R1",     0x0001},
			{"R2",     0x0002},
			{"R3",     0x0003},
			{"R4",     0x0004},
			{"R5",     0x0005},
			{"R6",     0x0006},
			{"R7",     0x0007},
			{"R8",     0x0008},
			{"R9",     0x0009},
			{"R10",    0x000A},
			{"R11",    0x000B},
			{"R12",    0x000C},
			{"R13",    0x000D},
			{"R14",    0x000E},
			{"R15",    0x000F},
			{"SCREEN", 0x4000},
			{"KBD",    0x6000},
		};
		uint16_t m_var_address = 0x0010;
	};

	class parser {
	public:
		enum class command_type {
			none,
			a,
			c,
			l,
		};

		parser(const std::string &file)
			: m_file(file, std::ifstream::in),
			  m_command_type(NO_COMMAND),
			  m_line_num(0)
		{
		}

		~parser()
		{
			if (m_file.is_open())
				m_file.close();
		}

		void reset()
		{
			m_command_type = command_type::none;
			m_line_num = 0;
			m_file.clear();
			m_file.seekg(0);
		}

		bool has_more_commands() const
		{
			return m_file.good();
		}

		void advance()
		{
			std::string line;
			m_command_type = command_type::none;
			do {
				std::getline(m_file >> std::ws, line);

				// remove all spaces
				line.erase(std::remove_if(line.begin(),
				                          line.end(),
				                          [](char x){return std::isspace(x);}),
				           line.end());

				if (line.empty())
					continue;

				// remove comments from string
				std::string::size_type comment_pos = line.find("//");

				// check if line has comments
				if (comment_pos != 0) {

					// remove comments if after command
					if (comment_pos != std::string::npos)
						line.erase(comment_pos);

					if (line.find("@") == 0)
						m_command_type = command_type::a;
					else if (line.find("(") == 0 &&
					         line.rfind(")") == line.size() - 1)
						m_command_type = command_type::l;
					else
						m_command_type = command_type::c;
				}
			} while (m_file.good() && m_command_type == command_type::none);

			if (m_command_type != command_type::none) {
				if (m_command_type != command_type::l)
					m_line_num++;
				m_command = line;
			} else
				m_command = "";
		}

		uint16_t line() const
		{
			return m_line_num;
		}

		command_type command() const
		{
			return m_command_type;
		}

		std::string symbol() const
		{
			switch (m_command_type) {
			case command_type::a:
				return m_command.substr(1);
				break;
			case command_type::l:
				return m_command.substr(1, m_command.size() - 2);
				break;
			default:
				return std::string("");
			};
		}

		std::string dest() const
		{
			if (m_command_type != command_type::c)
				return std::string("");

			std::string::size_type find_len = m_command.find("=");
			if (find_len == std::string::npos)
				return std::string("");

			return m_command.substr(0, find_len);
		}

		std::string comp() const
		{
			if (m_command_type != command_type::c)
				return std::string("");

			std::string::size_type find_len = m_command.find("=");
			return m_command.substr(find_len + 1, m_command.find(";") - find_len - 1);
		}

		std::string jump() const
		{
			if (m_command_type != command_type::c)
				return std::string("");

			std::string::size_type find_len = m_command.find(";");
			if (find_len == std::string::npos)
				return std::string("");
			return m_command.substr(find_len + 1);
		}

	private:
		std::ifstream m_file;
		command_type_t m_command_type;
		std::string m_command;
		uint16_t m_line_num;
	};

	class code {
	public:
		code(const std::string &file, symbol_table *symbol_table)
			: m_file(file, std::ofstream::out),
			  m_symbol_table(symbol_table)
		{
		}

		~code()
		{
			if (m_file.is_open())
				m_file.close();
		}

		void c_instruction(const std::string &dest, const std::string &comp, const std::string &jump)
		{
			m_file << "111" << std::bitset<7>(m_comp[comp])
			       << std::bitset<3>(m_dest[dest])
			       << std::bitset<3>(m_jump[jump])
			       << std::endl;
		}

		void a_instruction(const std::string &symbol)
		{
			uint16_t address;
			try {
				if ((address = std::stoi(symbol)));
			} catch(std::invalid_argument) {
				if (!m_symbol_table->contains(symbol))
					m_symbol_table->add_var(symbol);

				address = m_symbol_table->address(symbol);
			}
			m_file << std::setfill('0') << std::setw(16) << std::bitset<16>(address) << std::endl;
		}

	private:
		std::ofstream m_file;
		symbol_table *m_symbol_table;

		std::map<std::string, uint16_t> m_dest = {
			{"",    0b000},
			{"M",   0b001},
			{"D",   0b010},
			{"MD",  0b011},
			{"A",   0b100},
			{"AM",  0b101},
			{"AD",  0b110},
			{"AMD", 0b111},
		};
		std::map<std::string, uint16_t> m_comp = {
			{"0",   0b0101010},
			{"1",   0b0111111},
			{"-1",  0b0111010},
			{"D",   0b0001100},
			{"A",   0b0110000},
			{"!D",  0b0001101},
			{"!A",  0b0110001},
			{"-D",  0b0001111},
			{"-A",  0b0110011},
			{"D+1", 0b0011111},
			{"A+1", 0b0110111},
			{"D-1", 0b0001110},
			{"A-1", 0b0110010},
			{"D+A", 0b0000010},
			{"D-A", 0b0010011},
			{"A-D", 0b0000111},
			{"D&A", 0b0000000},
			{"D|A", 0b0010101},
			{"M",   0b1110000},
			{"!M",  0b1110001},
			{"-M",  0b1110011},
			{"M+1", 0b1110111},
			{"M-1", 0b1110010},
			{"D+M", 0b1000010},
			{"D-M", 0b1010011},
			{"M-D", 0b1000111},
			{"D&M", 0b1000000},
			{"D|M", 0b1010101},
		};
		std::map<std::string, uint16_t> m_jump = {
			{"",    0b000},
			{"JGT", 0b001},
			{"JEQ", 0b010},
			{"JGE", 0b011},
			{"JLT", 0b100},
			{"JNE", 0b101},
			{"JLE", 0b110},
			{"JMP", 0b111},
		};
	};

} // namespace hacker

int main(int argc, char *argv[])
{
	hacker::symbol_table symbol_table;
	std::string file_name(argv[1]);
	std::string hack_file_name(file_name.substr(0, file_name.rfind(".")).append(".hack"));

	hacker::parser p(file_name);
	hacker::code c(hack_file_name, &symbol_table);

	while (p.has_more_commands()) {
		p.advance();
		if (p.command() == hacker::parser::command_type::l)
			symbol_table.add_label(p.symbol(), p.line());
	}

	p.reset();

	while (p.has_more_commands()) {
		p.advance();
		if (p.command() == hacker::parser::command_type::a)
			c.a_instruction(p.symbol());
		else if (p.command() == hacker::parser::command_type::c)
			c.c_instruction(p.dest(), p.comp(), p.jump());
	}

	std::cout << "Writen binary to: " << hack_file_name << std::endl;

	return 0;
}
