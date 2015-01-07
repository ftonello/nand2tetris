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
#include <cstdlib>
#include <fstream>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

static void translate_file(const std::string &file_name, std::ostream &os)
{
	vm::parser p(file_name);
	vm::code c(file_name, os);

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
		case vm::command_type::c_label:
		case vm::command_type::c_goto:
		case vm::command_type::c_if:
			c.write_label_command(p.command(), p.arg1());
			break;
		case vm::command_type::none:
			break;
		default:
			std::cerr << "Error: Command (" << static_cast<int>(p.command()) << ") not implemented yet." << std::endl;
		}
	}
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

	std::string asm_file_name;
	std::ofstream ofs;

	if (fs::is_directory(arg_path)) {
		fs::path dir_path(arg_path);
		if (arg_path.stem() == ".")
			dir_path /= arg_path.parent_path().stem();
		else
			dir_path /= arg_path.stem();
		asm_file_name = dir_path.string() + ".asm";
		ofs.open(asm_file_name, std::ofstream::out);
		std::for_each(fs::directory_iterator(arg_path), fs::directory_iterator(),
		              [&ofs](const fs::path &p) {
			              if (p.extension() == ".vm")
				              translate_file(p.string(), ofs);
		              });
	} else if (fs::is_regular_file(arg_path)) {
		if (arg_path.extension() == ".vm") {
			std::string file_name(arg_path.string());
			asm_file_name = file_name.substr(0, file_name.rfind(".")) + ".asm";
			ofs.open(asm_file_name, std::ofstream::out);
			translate_file(file_name, ofs);
		}
		else
			abort_with_usage(argv[0]);
	} else
		abort_with_usage(argv[0]);

	ofs.flush();
	ofs.close();

	std::cout << "Writen Hack assembly to: " << asm_file_name << std::endl;

	return 0;
}
