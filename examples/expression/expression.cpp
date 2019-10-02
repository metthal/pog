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
         std::cerr << "Call: '" << __PRETTY_FUNCTION__ << "'" << std::endl;
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
         std::cerr << __PRETTY_FUNCTION__ << std::endl;
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

	_parser.token("\\(").symbol("LP").action( [&](std::string_view str) -> Value { return std::string{str}; } );
	_parser.token("\\)").symbol("RP").precedence(1, Associativity::Left).action( [&](std::string_view str) -> Value { return std::string{str}; } );
	_parser.token("<").symbol("LT").action( [&](std::string_view str) -> Value { return std::string{str}; } )
		.precedence(10, pog::Associativity::Left);
	_parser.token("filesize").symbol("FILESIZE").action( [&](std::string_view str) -> Value { return std::string{str}; } );


	_parser.set_start_symbol("expression");

	_parser.rule("expression")
		.production("primary_expression", "LT", "primary_expression", [&](auto&& args) -> Value {
			std::vector<std::string> output;
			auto left = std::move(args[0].getVector());
			output.insert(output.begin(), left.begin(), left.end() );
			output.push_back(args[1].getString());
			auto right = std::move(args[2].getVector());
			output.insert(output.begin(), right.begin(), right.end() );
			return Value(std::move(output));
		})
		.production("primary_expression", [&](auto&& args) -> Value {
			return std::move(args[0]);
		}).precedence(0, pog::Associativity::Left)
		.production("LP", "expression", "RP", [&](auto&& args) -> Value {
			std::cout << "expression -> ( expression )" << std::endl;
			auto output = std::vector<std::string>();
			output.push_back(std::move(args[0].getString()));
			auto expr = std::move(args[1].getVector());
			output.insert(output.begin(), expr.begin(), expr.end());
			output.push_back(std::move(args[2].getString()));
			return Value(std::move(output));
		})
		;

	_parser.rule("primary_expression")
		.production("LP", "primary_expression", "RP", [&](auto&& args) -> Value {
			std::cout << "primary_expression -> ( primary_expression )" << std::endl;
			auto output = std::vector<std::string>();
			output.push_back(std::move(args[0].getString()));
			auto expr = std::move(args[1].getVector());
			output.insert(output.begin(), expr.begin(), expr.end());
			output.push_back(std::move(args[2].getString()));
			return Value(std::move(output));
		})
		.production("FILESIZE", [&](auto&& args) -> Value {
			std::vector<std::string> output;
			output.emplace_back(std::move(args[0].getString()));
			return output;
		})
		;// end of primary_expression


	auto report = _parser.prepare();

	pog::HtmlReport html(_parser);
	html.save("html_index_expression_conflict.html");

	if (!report)
	{
		fmt::print("{}\n", report.to_string());
		return 1;
	}

	std::stringstream input("(filesize)<(filesize)");
	auto result = _parser.parse(input);
	auto output = std::move(result.value().getVector());
	for(const auto& str : output)
		std::cout << str << std::endl;
}

