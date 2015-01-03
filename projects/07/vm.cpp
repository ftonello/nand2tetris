/**
 * Jack VM implementation
 *
 * To compile:
 *   $ mkdir build
 *   $ cd $_
 *   $ cmake ..
 *   $ make
 */

#include "command_type.h"
#include "parser.h"
#include "code.h"

#include <algorithm>
#include <iostream>
#include <cstdlib>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

static void translate_file(const std::string &file_name)
{
	std::string asm_file_name(file_name.substr(0, file_name.rfind(".")).append(".asm"));
	vm::parser p(file_name);
	vm::code c(asm_file_name);

	while (p.has_more_commands()) {
		p.advance();
		switch (p.command()) {
		case vm::command_type::c_push:
		case vm::command_type::c_pop:
			c.write_push_pop(p.command(), p.arg1(), p.arg2());
			break;
		case vm::command_type::c_arithmetic:
			c.write_arithmetic(p.arg1());
			break;
		case vm::command_type::none:
			break;
		default:
			std::cerr << "Error: Command (" << static_cast<int>(p.command()) << ") not implemented yet." << std::endl;
		}
	}

	std::cout << "Writen Hack assembly to: " << asm_file_name << std::endl;
}

static void abort_with_usage(const char *argv0)
{
	std::cerr << "usage: " << argv0 << " [file.vm or dir(with *.vm)]" << std::endl;
	std::abort();
}

int main(int argc, char *argv[])
{
	if (argc != 2)
		abort_with_usage(argv[0]);

	std::string arg_name(argv[1]);
	fs::path arg_path(arg_name);

	if (!fs::exists(arg_path))
		abort_with_usage(argv[0]);

	if (fs::is_directory(arg_path)) {
		std::for_each(fs::directory_iterator(arg_path), fs::directory_iterator(),
		              [](const fs::path &p) {
			              if (p.extension() == ".vm")
				              translate_file(p.string());
		              });
	} else if (fs::is_regular_file(arg_path)) {
		if (arg_path.extension() == ".vm")
			translate_file(arg_path.string());
		else
			abort_with_usage(argv[0]);
	} else
		abort_with_usage(argv[0]);

	return 0;
}
