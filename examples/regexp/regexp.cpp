#include <iostream>
#include <vector>
#include <sstream>

#include <pog/parser.h>
#include <pog/html_report.h>

using namespace pog;

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
	Parser<Value> _parser;

	_parser.token(R"(\s+)");
	_parser.token(R"(\/)").states("@default").symbol("SLASH").action( [&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"(\|)").states("@default").symbol("REGEXP_OR").action( [&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"(\*)").states("@default").symbol("REGEXP_OR").action( [&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"(\+)").states("@default").symbol("REGEXP_PITER").action([&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"(\?)").states("@default").symbol("REGEXP_OPTIONAL").action([&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"(\.)").states("@default").symbol("REGEXP_ANY_CHAR").action([&](std::string_view str) -> Value { return std::string{str}; });
	_parser.token(R"([^\\\[\(\)\|\$\.\^\+\+*\?])").states("@default").symbol("REGEXP_CHAR").action( [&](std::string_view str) -> Value { return std::string(1, str[0]); });
	_parser.token(R"(\\w)").states("@default").symbol("REGEXP_WORD_CHAR").action( [&](std::string_view str) -> Value { return std::string{str};} );


	_parser.set_start_symbol("regexp");

	_parser.rule("regexp") //shared_ptr<String>
		.production("SLASH", "regexp_concat", "SLASH", [&](auto&& args) -> Value {
			return std::move(args[1]);
		})
		;
	//// _parser.rule("regexp_body") //shared_ptr<String>
	//// 	.production("regexp_or", [&](auto&& args) -> Value {
	//// 		return std::move(args[0]);
	//// 	});

	//// _parser.rule("regexp_or") //shared_ptr<RegexpUnit>
		//// .production("regexp_concat", [](auto&& args) -> Value { return std::move(args[0]); })
		//// .production("regexp_or", "REGEXP_OR", "regexp_concat", [](auto&& args) -> Value {
		//// 	// std::cout << "concat composed" << std::endl;
		//// 	std::string arg = std::move(args[0].getString());
		//// 	std::vector<std::string> concat = std::move(args[2].getVector());
		//// 	concat.push_back(std::move(arg));
		//// 	return Value(std::move(concat));
		//// })
		;
	_parser.rule("regexp_concat") //vector<shared_ptr<RegexpUnit>>
		.production("regexp_repeat", [](auto&& args) -> Value {
			//// std::cout << "leaf repeat matched " << std::endl;
			std::vector<std::string> output;
			output.push_back(std::move(args[0].getString()));
			return Value(std::move(output));
		})
		.production("regexp_concat", "regexp_repeat", [](auto&& args) -> Value {
			//// std::cout << "nonleaf repeat matched " << std::endl;
			std::vector<std::string> output = std::move(args[0].getVector());
			output.push_back(std::move(args[1].getString()));
			return Value(std::move(output));
		})
		;
	_parser.rule("regexp_repeat") //shared_ptr<RegexpUnit>
		// .production("regexp_single", "REGEXP_PITER", /*"regexp_greedy",*/ [](auto&& args) -> Value {
		// 	std::stringstream ss;
		// 	ss << args[0].getString() << args[2].getString();
		// 	return Value(std::move(ss.str()));
		// })
		.production("regexp_single", [](auto&& args) -> Value {
			//// std::cout << "matched single" << std::endl;
			return std::move(args[0]);
		})
		;
	//// _parser.rule("regexp_greedy")
	//// 	.production([](auto&&) -> Value { return {}; })
	//// 	.production("REGEXP_OPTIONAL", [](auto&& args) -> Value { return std::move(args[0]); })
	//// 	;

	_parser.rule("regexp_single")
		.production("LP", "regexp_or", "RP", [](auto&& args) -> Value { return std::move(args[0]); })
		.production("REGEXP_ANY_CHAR", [](auto&& args) -> Value { return std::move(args[0]); })
		.production("REGEXP_CHAR", [](auto&& args) -> Value { return std::move(args[0]); })
		.production("REGEXP_WORD_CHAR", [](auto&& args) -> Value { return std::move(args[0]); })
		;



	auto report = _parser.prepare();

	pog::HtmlReport html(_parser);
	html.save("html_index_commented.html");

	if (!report)
	{
		fmt::print("{}\n", report.to_string());
		return 1;
	}

	std::stringstream input("/abcd/");
	auto result = _parser.parse(input);
//	fmt::print("Result: {}\n", result.value().getVector());
	auto output = std::move(result.value().getVector());
	for(const auto& str : output)
		std::cout << str << std::endl;
}

