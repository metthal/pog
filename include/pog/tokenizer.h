#pragma once

#include <memory>
#include <vector>

#include <re2/set.h>

#include <pog/grammar.h>
#include <pog/token.h>

namespace pog {

template <typename ValueT>
struct TokenMatch
{
	TokenMatch(const Symbol<ValueT>* sym) : symbol(sym), value(), match_length(0) {}
	template <typename T>
	TokenMatch(const Symbol<ValueT>* sym, T&& v, std::size_t len) : symbol(sym), value(std::forward<T>(v)), match_length(len) {}
	TokenMatch(const TokenMatch&) = default;
	TokenMatch(TokenMatch&&) noexcept = default;

	TokenMatch& operator=(const TokenMatch&) = default;
	TokenMatch& operator=(TokenMatch&&) noexcept = default;

	const Symbol<ValueT>* symbol;
	ValueT value;
	std::size_t match_length;
};

template <typename ValueT>
class Tokenizer
{
public:
	using GrammarType = Grammar<ValueT>;
	using SymbolType = Symbol<ValueT>;
	using TokenType = Token<ValueT>;
	using TokenMatchType = TokenMatch<ValueT>;

	Tokenizer(GrammarType* grammar) : _grammar(grammar)
	{
		_re_set = std::make_unique<re2::RE2::Set>(re2::RE2::DefaultOptions, re2::RE2::Anchor::ANCHOR_START);
	}

	void prepare()
	{
		_re_set->Compile();
	}

	TokenType* add_token(const std::string& pattern, const SymbolType* symbol)
	{
		_tokens.push_back(std::make_unique<TokenType>(_tokens.size(), pattern, symbol));
		std::string error;
		_re_set->Add(_tokens.back()->get_pattern(), &error);
		assert(error.empty() && "Error when compiling token regexp");
		return _tokens.back().get();
	}

	void push_input_stream(std::istream& stream)
	{
		std::string input;
		std::vector<char> block(4096);
		while (stream.good())
		{
			stream.read(block.data(), block.size());
			input.append(std::string_view(block.data(), stream.gcount()));
		}

		_input_stack.emplace_back(std::move(input), re2::StringPiece{});
		_input_stack.back().second = re2::StringPiece{_input_stack.back().first};
	}

	void pop_input_stream()
	{
		_input_stack.pop_back();
	}

	void push_state(std::uint32_t state)
	{
		_state_stack.push_back(state);
	}

	void pop_state()
	{
		_state_stack.pop_back();
	}

	std::optional<TokenMatchType> next_token()
	{
		bool repeat = true;
		while (repeat)
		{
			if (_input_stack.empty())
				return std::nullopt;

			auto& current_input = _input_stack.back().second;
			if (current_input.size() > 0)
			{
				// TODO: If consume_matched_token was not called we should remember the token somewhere so we don't unnecessarily match the input
				// once again for the same token
				std::vector<int> matched_patterns;
				_re_set->Match(current_input, &matched_patterns);

				// Haven't matched anything, tokenization failure, we will get into endless loop
				if (matched_patterns.empty())
					return std::nullopt;

				re2::StringPiece submatch;
				const TokenType* best_match = nullptr;
				std::size_t longest_match = 0;
				for (auto pattern_index : matched_patterns)
				{
					_tokens[pattern_index]->get_regexp()->Match(current_input, 0, current_input.size(), re2::RE2::Anchor::ANCHOR_START, &submatch, 1);
					if (longest_match < static_cast<std::size_t>(submatch.size()))
					{
						best_match = _tokens[pattern_index].get();
						longest_match = submatch.size();
					}
				}

				ValueT value{};
				if (best_match->has_action())
					value = best_match->perform_action(std::string_view{current_input.data(), longest_match});

				if (!best_match->has_symbol())
				{
					// explicit consume because there is nothing which reduced token without symbol
					consume_input_characters(longest_match);
					continue;
				}

				return TokenMatchType{best_match->get_symbol(), std::move(value), longest_match};
			}

			return _grammar->get_end_of_input_symbol();
		}

		return std::nullopt;
	}

	void consume_matched_token(const TokenMatchType& token)
	{
		consume_input_characters(token.match_length);
	}

	void consume_input_characters(std::size_t count)
	{
		if (_input_stack.empty())
			assert(false && "Attempting to consume token from non-existing stream");

		_input_stack.back().second.remove_prefix(count);
	}

private:
	GrammarType* _grammar;
	std::vector<std::unique_ptr<TokenType>> _tokens;

	std::unique_ptr<re2::RE2::Set> _re_set;
	std::vector<std::pair<std::string, re2::StringPiece>> _input_stack;
	std::vector<std::uint32_t> _state_stack;
};

//enum class PatternBoundary
//{
//	None,
//	Fullword
//};
//
//template <typename ValueT>
//class Tokenizer
//{
//public:
//	using TokenType = Token<ValueT>;
//
//	Tokenizer() : _tokens(), _input_stack(), _state_stack(), _re_set(re2::RE2::DefaultOptions, re2::RE2::Anchor::ANCHOR_START) {}
//
//	TokenType* add_token(std::string pattern, PatternBoundary boundary = PatternBoundary::None)
//	{
//		if (boundary == PatternBoundary::Fullword)
//			pattern = fmt::format("\\b{}(\\b|$)", pattern);
//	}
//};

} // namespace pog
