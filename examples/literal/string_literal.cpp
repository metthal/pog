#include <iostream>
#include <vector>
#include <sstream>

#define POG_DEBUG 1
#include <pog/parser.h>
#include <pog/html_report.h>

using namespace pog;

// sudo ./run.sh
// ./build/examples/expression/example-expression

class Value
{
public:
	using Variant = std::variant<
		std::string, //0
		std::vector<std::string> //1
	>;

	/// @name Constructors
	/// @{
	Value( const std::string& v ) : _value(v) {}
	Value( const std::vector<std::string>& v ) : _value(v) {}
	Value( std::string&& v ) : _value( std::move(v) ) {}
	Value( std::vector<std::string>&& v ) : _value(std::move(v)) {}
	Value() = default;
	/// @}


	/// @name Getter methods
	/// @{
	std::string&& getString()
	{
		return moveValue<std::string>();
	}
	std::vector<std::string>&& getVector()
	{
		return moveValue<std::vector<std::string>>();
	}
	/// @}

protected:
	template<typename T>
	const T& getValue() const
	{
		try
      {
         return std::get<T>(_value);
      }
      catch (std::bad_variant_access& exp)
      {
         std::cerr << "Called Value.getValue() with incompatible type. Actual index is '" << _value.index() << "'" << std::endl << exp.what() << std::endl;
         // std::cerr << "Call: '" << __PRETTY_FUNCTION__ << "'" << std::endl;
         assert(false && "Called getValue<T>() with incompatible type T.");
      }
	}
	template< typename T>
	T&& moveValue()
	{
		try
      {
         return std::move(std::get<T>(std::move(_value)));
      }
      catch (std::bad_variant_access& exp)
      {
          std::cerr << "Called Value.moveValue() with incompatible type. Actual index is '" << _value.index() << "'" << std::endl << exp.what() << std::endl;
         // std::cerr << __PRETTY_FUNCTION__ << std::endl;
         assert(false && "Called getValue<T>() with incompatible type T.");
      }
	}

private:
	Variant _value;
};


int main()
{
	std::cout << "Starting parsing " << std::endl;
	Parser<Value> _parser;
	std::string _strLiteral;

		_parser.token(R"(\")").states("@default").enter_state("$str").action([&](std::string_view) -> Value {
        _strLiteral.clear();
        return {};
    });
    _parser.token(R"(\\t)").states("$str").action([&](std::string_view) -> Value {
        _strLiteral += "\\t";
        return {};
    });

	_parser.token(R"(\\n)").states("$str").action([&](std::string_view) -> Value {
        _strLiteral += "\\n";
       // currentLocation().addLine();
        return {};
    });
    _parser.token(R"(\\x[0-9a-fA-F]{2})").states("$str").action([&](std::string_view str) -> Value {
        _strLiteral += "\\x";
        _strLiteral += std::string{str}.substr(2);
        return {};
    });
    _parser.token(R"(\\\")").states("$str").action([&](std::string_view) -> Value { _strLiteral += "\\\""; return {}; });
    _parser.token(R"(\\\\)").states("$str").action([&](std::string_view) -> Value { _strLiteral += "\\\\"; return {}; });
    _parser.token(R"(\\\.)").states("$str").action([&](std::string_view str) -> Value { std::cerr<< "Error: Unknown escape sequence '" << std::string{str} << "'" << std::endl; return {}; });
    _parser.token("[^\\]+").states("$str").action([&](std::string_view str) -> Value { std::cout << "Matched " << std::string{str} << std::endl; _strLiteral += std::string{str}; return {}; });
    _parser.token(R"(\")").states("$str").symbol("STRING_LITERAL").enter_state("@default").action([&](std::string_view) -> Value {
        return _strLiteral;
    });

   std::stringstream input2(R"("Here are \" hehe")");
	std::stringstream input3(R"("Here are \t\n")");
	std::stringstream input4(R"("Here are \"\t\n\\\x01\xff")");
	std::stringstream input5(R"("Double \"\t\n\\\x01\xff quotes")");

	_parser.set_start_symbol("literal");

	_parser.rule("literal")
		.production("STRING_LITERAL", [&](auto&& args) -> Value {
			std::string output;
			std::cout << "Matched '" << args[0].getString() << "'" << std::endl;
			output = std::move(args[0].getString());
			return output;
		})
		;

	auto report = _parser.prepare();

	pog::HtmlReport html(_parser);
	html.save("html_index_expression_conflict.html");

	if (!report)
	{
		fmt::print("{}\n", report.to_string());
		return 1;
	}

	// std::stringstream input("filesize<(filesize)");
	// auto result = _parser.parse(input);
	// auto output = std::move(result.value().getVector());
	// for(const auto& str : output)
	// 	std::cout << str << std::endl;

	std::cout << std::endl << "### Hohoho2 " << input2.str() << std::endl;
	auto result2 = _parser.parse(input2);
	auto output2 = std::move(result2.value().getString());
	std::cout << "### Result is:" << std::endl;
	std::cout << output2 << std::endl;

	std::cout << std::endl << "### Hohoho3" << input3.str() << std::endl;
	auto result3 = _parser.parse(input3);
	auto output3 = std::move(result3.value().getString());
	std::cout << "### Result is:" << std::endl;
	std::cout << output3 << std::endl;

	std::cout << std::endl << "### Hohoho4" << input4.str() << std::endl;
	auto result4 = _parser.parse(input4);
	auto output4 = std::move(result4.value().getString());
	std::cout << "### Result is:" << std::endl;
	std::cout << output4 << std::endl;

	std::cout << std::endl << "### Hohoho5" << input5.str() << std::endl;
	auto result5 = _parser.parse(input5);
	auto output5 = std::move(result5.value().getString());
	std::cout << "### Result is:" << std::endl;
	std::cout << output5 << std::endl;

	// std::stringstream input6(R"(Here are \n)");
	// auto result6 = _parser.parse(input6);
	// auto output6 = std::move(result6.value().getString());
	// std::cout << output6 << std::endl;

}

