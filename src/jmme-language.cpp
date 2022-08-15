#include "jmme-language.hpp"

#include <cassert>

namespace JMMExplorer
{

str get_mnemonic(ArithmeticOpType op_type)
{
	switch (op_type)
	{
		case ArithmeticOpType::Add:
			return "add";
		case ArithmeticOpType::Subtract:
			return "sub";
		case ArithmeticOpType::Multiply:
			return "mul";
		case ArithmeticOpType::Divide:
			return "div";
		case ArithmeticOpType::Modulo:
			return "modulo";
		default:
			assert(false);
	}
}

}
