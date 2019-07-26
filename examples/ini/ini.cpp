#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

#include <pog/parser.h>

using namespace pog;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct NoData{};
using Value = std::variant<NoData, bool, int, double, std::string>;

struct Attribute
{
	std::string key;
	Value value;
};

struct Section
{
	std::string name;
	std::vector<Attribute> attributes;
};

struct Document
{
	Section global;
	std::vector<Section> sections;
};

void print_attributes(const Section& section)
{
	for (const auto& attr : section.attributes)
	{
		fmt::print("{}::{}", section.name, attr.key);
		std::visit(overloaded {
			[](const NoData&) { fmt::print(" (no data)\n"); },
			[](bool b) { fmt::print(" (bool) = {}\n", b); },
			[](int i) { fmt::print(" (int) = {}\n", i); },
			[](double d) { fmt::print(" (double) = {}\n", d); },
			[](const std::string& s) { fmt::print(" (string) = {}\n", s); }
		}, attr.value);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fmt::print("Usage: {} INPUT_FILE\n", argv[0]);
		return 1;
	}

	using ParserType = std::variant<
		Value,
		Attribute,
		Section,
		Document,
		std::vector<Attribute>,
		std::vector<Section>
	>;

	Parser<ParserType> p;

	p.token(R"(\s+)");
	p.token("\\[").symbol("[");
	p.token("\\]").symbol("]");
	p.token("=").symbol("=");
	p.token(R"([0-9]+\.[0-9]+)").symbol("double").action([](std::string_view str) -> ParserType {
		return std::stod(std::string{str});
	});
	p.token(R"([0-9]+)").symbol("int").action([](std::string_view str) -> ParserType {
		return std::stoi(std::string{str});
	});
	p.token("(true|false)").symbol("bool").action([](std::string_view str) -> ParserType {
		return str == "true";
	});
	p.token("[a-zA-Z0-9]+").symbol("id").fullword().action([](std::string_view str) -> ParserType {
		return std::string{str};
	});

	p.set_start_symbol("root");
	p.rule("root")
		.production("attrs", "sections").action([](auto&& args) -> ParserType {
			return Document{
				Section{std::string{}, std::get<std::vector<Attribute>>(args[0])},
				std::get<std::vector<Section>>(args[1])
			};
		})
		.production("attrs").action([](auto&& args) -> ParserType {
			return Document{
				Section{std::string{}, std::get<std::vector<Attribute>>(args[0])},
				std::vector<Section>{}
			};
		})
		.production().action([](auto&&) -> ParserType {
			return Document{
				Section{std::string{}, std::vector<Attribute>{}},
				std::vector<Section>{}
			};
		});

	p.rule("sections")
		.production("sections", "section").action([](auto&& args) -> ParserType {
			std::get<std::vector<Section>>(args[0]).push_back(std::get<Section>(args[1]));
			return std::move(args[0]);
		})
		.production("section").action([](auto&& args) -> ParserType {
			return std::vector<Section>{std::get<Section>(args[0])};
		});

	p.rule("section")
		.production("[", "id", "]", "attrs").action([](auto&& args) -> ParserType {
			return Section{
				std::get<std::string>(std::get<Value>(args[1])),
				std::get<std::vector<Attribute>>(args[3])
			};
		});

	p.rule("attrs")
		.production("attrs", "attr").action([](auto&& args) -> ParserType {
			std::get<std::vector<Attribute>>(args[0]).push_back(std::get<Attribute>(args[1]));
			return std::move(args[0]);
		})
		.production("attr").action([](auto&& args) -> ParserType {
			return std::vector<Attribute>{std::get<Attribute>(args[0])};
		});

	p.rule("attr").production("id", "=", "value").action([](auto&& args) -> ParserType {
		return Attribute{
			std::get<std::string>(std::get<Value>(args[0])),
			std::get<Value>(args[2])
		};
	});

	p.rule("value")
		.production("double").action([](auto&& args) -> ParserType {
			return std::move(args[0]);
		})
		.production("int").action([](auto&& args) -> ParserType {
			return std::move(args[0]);
		})
		.production("bool").action([](auto&& args) -> ParserType {
			return std::move(args[0]);
		})
		.production("id").action([](auto&& args) -> ParserType {
			return std::move(args[0]);
		});

	p.prepare();

	std::ifstream input_file(argv[1]);
	auto document = p.parse(input_file);

	if (document)
	{
		const auto& d = document.value();
		if (std::holds_alternative<Document>(d))
		{
			const auto& dd = std::get<Document>(d);
			print_attributes(dd.global);
			for (const auto& sect : dd.sections)
				print_attributes(sect);
		}
		else
		{
			fmt::print("Parser error\n");
			return 1;
		}
	}
	else
	{
		fmt::print("Parser error\n");
		return 1;
	}
}
