#pragma once

namespace vm {
	enum class command_type {
		none,
		c_arithmetic,
		c_push,
		c_pop,
		c_label,
		c_goto,
		c_if,
		c_function,
		c_return,
		c_call,
	};
} // namespace vm
