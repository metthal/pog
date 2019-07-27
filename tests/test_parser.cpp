#include <gtest/gtest.h>

#include <pog/parser.h>

using namespace pog;

class TestParser : public ::testing::Test {};

TEST_F(TestParser,
RepeatingAs) {
	Parser<int> p;

	p.token("a").symbol("a");

	p.set_start_symbol("A");
	p.rule("A")
		.production("A", "a").action([](auto&& args) {
			return 1 + args[0];
		})
		.production("a").action([](auto&&) {
			return 1;
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("a");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 1);

	std::stringstream input2("aaaa");
	result = p.parse(input2);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 4);

	try
	{
		std::stringstream input3("aa aaa");
		p.parse(input3);
		FAIL() << "Expected syntax error";
	}
	catch (const SyntaxError& e)
	{
		EXPECT_STREQ(e.what(), "Syntax error: Unknown symbol on input, expected one of @end, a");
	}
}

TEST_F(TestParser,
RepeatingAsWithIgnoringWhitespaces) {
	Parser<int> p;

	p.token(R"(\s+)");
	p.token("a").symbol("a");

	p.set_start_symbol("A");
	p.rule("A")
		.production("A", "a").action([](auto&& args) {
			return 1 + args[0];
		})
		.production("a").action([](auto&&) {
			return 1;
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("a");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 1);

	std::stringstream input2("aaaa");
	result = p.parse(input2);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 4);

	std::stringstream input3("aa aaa");
	result = p.parse(input3);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 5);
}

TEST_F(TestParser,
SameNumberOfAsAndBs) {
	Parser<int> p;

	p.token("a").symbol("a");
	p.token("b").symbol("b");

	p.set_start_symbol("S");
	p.rule("S")
		.production("a", "S", "b").action([](auto&& args) {
			return 1 + args[1];
		})
		.production("a", "b").action([](auto&&) {
			return 1;
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("ab");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 1);

	std::stringstream input2("aaabbb");
	result = p.parse(input2);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 3);

	try
	{
		std::stringstream input3("aabbb");
		p.parse(input3);
		FAIL() << "Expected syntax error";
	}
	catch (const SyntaxError& e)
	{
		EXPECT_STREQ(e.what(), "Syntax error: Unexpected b, expected one of @end");
	}

	try
	{
		std::stringstream input4("aaabb");
		p.parse(input4);
		FAIL() << "Expected syntax error";
	}
	catch (const SyntaxError& e)
	{
		EXPECT_STREQ(e.what(), "Syntax error: Unexpected @end, expected one of b");
	}
}

TEST_F(TestParser,
LalrButNotLrNorNqlalr) {
	Parser<int> p;

	p.token("a").symbol("a");
	p.token("b").symbol("b");
	p.token("c").symbol("c");
	p.token("d").symbol("d");
	p.token("g").symbol("g");

	p.set_start_symbol("S");
	p.rule("S")
		.production("a", "g", "d")
		.production("a", "A", "c")
		.production("b", "A", "d")
		.production("b", "g", "c");
	p.rule("A")
		.production("B");
	p.rule("B")
		.production("g");
	EXPECT_TRUE(p.prepare());

	std::stringstream input("agc");
	auto result = p.parse(input);
	EXPECT_TRUE(result);
}

TEST_F(TestParser,
Precedence) {
	Parser<int> p;

	p.token(R"(\s+)");
	p.token(R"(\+)").symbol("+").precedence(0, Associativity::Left);
	p.token(R"(-)").symbol("-").precedence(0, Associativity::Left);
	p.token(R"(\*)").symbol("*").precedence(1, Associativity::Left);
	p.token("[0-9]+").symbol("int").action([](std::string_view str) {
		return std::stoi(std::string{str});
	});

	p.set_start_symbol("E");
	p.rule("E")
		.production("E", "+", "E").action([](auto&& args) {
			return args[0] + args[2];
		})
		.production("E", "-", "E").action([](auto&& args) {
			return args[0] - args[2];
		})
		.production("E", "*", "E").action([](auto&& args) {
			return args[0] * args[2];
		})
		.production("-", "E").action([](auto&& args) {
			return -args[1];
		}).precedence(2, Associativity::Right)
		.production("int").action([](auto&& args) {
			return args[0];
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("2 + 3 * 4 + 5");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 19);

	std::stringstream input2("-5 - 3 - -10");
	result = p.parse(input2);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), 2);

	std::stringstream input3("5 + -3 * 10");
	result = p.parse(input3);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), -25);
}

TEST_F(TestParser,
Conflicts1) {
	Parser<int> p;

	p.token("a").symbol("a");

	p.set_start_symbol("sequence");
	p.rule("sequence")
		.production("sequence", "a")
		.production("maybea")
		.production();
	p.rule("maybea")
		.production("a")
		.production();

	auto report = p.prepare();
	EXPECT_FALSE(report);
	EXPECT_EQ(report.number_of_issues(), 3u);
	EXPECT_EQ(report.to_string(),
		"Shift-reduce conflict of symbol 'a' and rule 'sequence -> <eps>' in state 0\n"
		"Reduce-reduce conflict of rule 'sequence -> <eps>' and rule 'maybea -> <eps>' in state 0\n"
		"Shift-reduce conflict of symbol 'a' and rule 'maybea -> <eps>' in state 0"
	);
}

TEST_F(TestParser,
Conflicts2) {
	Parser<int> p;

	p.token("b").symbol("b");
	p.token("c").symbol("c");

	p.set_start_symbol("Y");
	p.rule("Y")
		.production("c", "c", "Z", "b");
	p.rule("Z")
		.production("c", "Z", "b")
		.production("c", "Z")
		.production();

	auto report = p.prepare();
	EXPECT_FALSE(report);
	EXPECT_EQ(report.number_of_issues(), 1u);
	EXPECT_EQ(report.to_string(), "Shift-reduce conflict of symbol 'b' and rule 'Z -> c Z' in state 6");
}

TEST_F(TestParser,
Conflicts3) {
	Parser<std::vector<std::string>> p;

	p.token("\\(").symbol("(");
	p.token("\\)").symbol(")");
	p.token("a").symbol("a");

	p.set_start_symbol("E");
	p.rule("E")
		.production("(", "E", ")").action([](auto&& args) {
			args[1].push_back("E -> ( E )");
			return std::move(args[1]);
		})
		.production("PE").action([](auto&& args) {
			args[0].push_back("E -> PE");
			return std::move(args[0]);
		});
	p.rule("PE")
		.production("(", "PE", ")").action([](auto&& args) {
			args[1].push_back("PE -> ( PE )");
			return std::move(args[1]);
		})
		.production("a").action([](auto&&) {
			return std::vector<std::string>{"PE -> a"};
		});

	auto report = p.prepare();
	EXPECT_FALSE(report);
	EXPECT_EQ(report.number_of_issues(), 1u);
	EXPECT_EQ(report.to_string(), "Shift-reduce conflict of symbol ')' and rule 'E -> PE' in state 6");

	std::stringstream input1("(((a)))");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), (std::vector<std::string>{
		"PE -> a",
		"PE -> ( PE )",
		"PE -> ( PE )",
		"PE -> ( PE )",
		"E -> PE"
	}));
}

TEST_F(TestParser,
ResolveConflictWithPrecedence) {
	Parser<std::vector<std::string>> p;

	p.token("\\(").symbol("(");
	p.token("\\)").symbol(")").precedence(0, Associativity::Left);
	p.token("a").symbol("a");

	p.set_start_symbol("E");
	p.rule("E")
		.production("(", "E", ")").action([](auto&& args) {
			args[1].push_back("E -> ( E )");
			return std::move(args[1]);
		})
		.production("PE").action([](auto&& args) {
			args[0].push_back("E -> PE");
			return std::move(args[0]);
		}).precedence(1, Associativity::Left);
	p.rule("PE")
		.production("(", "PE", ")").action([](auto&& args) {
			args[1].push_back("PE -> ( PE )");
			return std::move(args[1]);
		})
		.production("a").action([](auto&&) {
			return std::vector<std::string>{"PE -> a"};
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("(((a)))");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(result.value(), (std::vector<std::string>{
		"PE -> a",
		"E -> PE",
		"E -> ( E )",
		"E -> ( E )",
		"E -> ( E )"
	}));
}

TEST_F(TestParser,
MoveOnlyType) {
	Parser<std::unique_ptr<int>> p;

	p.token("a").symbol("a");

	p.set_start_symbol("A");
	p.rule("A")
		.production("A", "a").action([](auto&& args) {
			*(args[0].get()) += 1;
			return std::move(args[0]);
		})
		.production("a").action([](auto&&) {
			return std::make_unique<int>(1);
		});
	EXPECT_TRUE(p.prepare());

	std::stringstream input1("a");
	auto result = p.parse(input1);
	EXPECT_TRUE(result);
	EXPECT_EQ(*result.value().get(), 1);

	std::stringstream input2("aaaa");
	result = p.parse(input2);
	EXPECT_TRUE(result);
	EXPECT_EQ(*result.value().get(), 4);

	try
	{
		std::stringstream input3("aa aaa");
		p.parse(input3);
		FAIL() << "Expected syntax error";
	}
	catch (const SyntaxError& e)
	{
		EXPECT_STREQ(e.what(), "Syntax error: Unknown symbol on input, expected one of @end, a");
	}
}
