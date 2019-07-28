#include <iostream>
#include <vector>
#include <sstream>

#include <pog/parser.h>

using namespace pog;

int main()
{
	Parser<int> p;

	p.token(R"(\s+)");
	p.token(R"(\+)").symbol("+").precedence(1, Associativity::Left);
	p.token(R"(\*)").symbol("*").precedence(2, Associativity::Left);
	p.token(R"(-)").symbol("-").precedence(1, Associativity::Left);
	p.token("\\(").symbol("(");
	p.token("\\)").symbol(")");
	p.token("[0-9]+").symbol("num").action([](std::string_view str) {
		return std::stoi(std::string{str});
	});

	p.set_start_symbol("E");
	p.rule("E") // E ->
		.production("E", "+", "E").action([](auto&& args) { // E + E
			return args[0] + args[2];
		})
		.production("E", "-", "E").action([](auto&& args) { // E - E
			return args[0] - args[2];
		})
		.production("E", "*", "E").action([](auto&& args) { // E * E
			return args[0] * args[2];
		})
		.production("(", "E", ")").action([](auto&& args) { // ( E )
			return args[1];
		})
		.production("num").action([](auto&& args) { // num
			return args[0];
		})
		.production("-", "E").precedence(3, Associativity::Right).action([](auto&& args) { // - E
			return -args[1];
		});

	if (auto report = p.prepare(); !report)
	{
		fmt::print("{}\n", report.to_string());
		return 1;
	}

	std::stringstream input("11 + 4 * 3 + 2");
	auto result = p.parse(input);
	fmt::print("Result: {}\n", result.value());
}

