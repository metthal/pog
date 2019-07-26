#include <iostream>
#include <vector>
#include <sstream>

#include <pog/parser.h>

using namespace pog;

int main()
{
	Parser<int> p;

	auto E = p.add_symbol("E");
	auto plus = p.add_symbol("+");
	auto mult = p.add_symbol("*");
	auto subt = p.add_symbol("-");
	auto lp = p.add_symbol("(");
	auto rp = p.add_symbol(")");
	auto number = p.add_symbol("num");

	mult->set_precendence(2, Associativity::Left);
	plus->set_precendence(1, Associativity::Left);
	subt->set_precendence(1, Associativity::Left);

	p.add_token(R"(\s+)");
	p.add_token(R"(\+)", plus);
	p.add_token(R"(\*)", mult);
	p.add_token(R"(-)", subt);
	p.add_token("\\(", lp);
	p.add_token("\\)", rp);
	p.add_token("[0-9]+", number, [](std::string_view str) {
		return std::stoi(std::string{str});
	});

	p.set_start_symbol(E);
	p.add_rule(E, std::vector<const Symbol<int>*>{E, plus, E}, [](auto&& args) -> int {
		return args[0] + args[2];
	});
	p.add_rule(E, std::vector<const Symbol<int>*>{E, subt, E}, [](auto&& args) -> int {
		return args[0] - args[2];
	});
	p.add_rule(E, std::vector<const Symbol<int>*>{E, mult, E}, [](auto&& args) -> int {
		return args[0] * args[2];
	});
	p.add_rule(E, std::vector<const Symbol<int>*>{lp, E, rp}, [](auto&& args) -> int {
		return args[1];
	});
	p.add_rule(E, std::vector<const Symbol<int>*>{number}, [](auto&& args) {
		return args[0];
	});
	p.add_rule(E, std::vector<const Symbol<int>*>{subt, E}, [](auto&& args) -> int {
		return -args[1];
	})->set_precendence(3, Associativity::Right);

	p.prepare();

	std::stringstream input("11 + 4");
	auto result = p.parse(input);
	std::cout << "Result: " << result.value() << std::endl;
}

