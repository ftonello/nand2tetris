/**
 * Jack VM implementation
 *
 * To compile:
 * g++ vm.cpp -std=c++11 -Wall -o vm
 */

#include "command_type.h"
#include "parser.h"
#include "code.h"

#include <iostream>

int main(int argc, char *argv[])
{
	std::string file_name(argv[1]);
	std::string asm_file_name(file_name.substr(0, file_name.rfind(".")).append(".asm"));
	vm::parser p(file_name);
	vm::code c(asm_file_name);

	while (p.has_more_commands()) {
		p.advance();
		switch (p.command()) {
		case vm::command_type::c_push:
			c.write_push_pop(p.command(), p.arg1(), p.arg2());
			break;
		case vm::command_type::c_arithmetic:
			c.write_arithmetic(p.arg1());
			break;
		defaul:
			std::cerr << "Error: Command not implemented yet." << std::endl;
		}
	}

	std::cout << "Writen Hack assembly to: " << asm_file_name << std::endl;

	return 0;
}
