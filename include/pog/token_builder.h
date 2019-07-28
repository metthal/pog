#pragma once

#include <pog/grammar.h>
#include <pog/token.h>
#include <pog/tokenizer.h>

namespace pog {

template <typename ValueT>
class TokenBuilder
{
public:
	using GrammarType = Grammar<ValueT>;
	using SymbolType = Symbol<ValueT>;
	using TokenType = Token<ValueT>;
	using TokenizerType = Tokenizer<ValueT>;

	TokenBuilder(GrammarType* grammar, TokenizerType* tokenizer) : _grammar(grammar), _tokenizer(tokenizer), _pattern("$"),
		_symbol_name(), _precedence(), _action(), _fullword(false), _end_token(true) {}

	TokenBuilder(GrammarType* grammar, TokenizerType* tokenizer, const std::string& pattern) : _grammar(grammar), _tokenizer(tokenizer), _pattern(pattern),
		_symbol_name(), _precedence(), _action(), _fullword(false), _end_token(false) {}

	void done()
	{
		TokenType* token;
		if (!_end_token)
		{
			auto* symbol = !_symbol_name.empty() ? _grammar->add_symbol(SymbolKind::Terminal, _symbol_name) : nullptr;
			token = _tokenizer->add_token(_fullword ? fmt::format("\\b{}(\\b|$)", _pattern) : _pattern, symbol);
			if (symbol && _precedence)
			{
				const auto& prec = _precedence.value();
				symbol->set_precedence(prec.level, prec.assoc);
			}
		}
		else
			token = _tokenizer->get_end_token();

		if (_action)
			token->set_action(std::move(_action));
	}

	TokenBuilder& symbol(const std::string& symbol_name)
	{
		_symbol_name = symbol_name;
		return *this;
	}

	TokenBuilder& precedence(std::uint32_t level, Associativity assoc)
	{
		_precedence = Precedence{level, assoc};
		return *this;
	}

	template <typename CallbackT>
	TokenBuilder& action(CallbackT&& action)
	{
		_action = std::forward<CallbackT>(action);
		return *this;
	}

	TokenBuilder& fullword()
	{
		_fullword = true;
		return *this;
	}

private:
	GrammarType* _grammar;
	TokenizerType* _tokenizer;
	std::string _pattern;
	std::string _symbol_name;
	std::optional<Precedence> _precedence;
	typename TokenType::CallbackType _action;
	bool _fullword;
	bool _end_token;
};

} // namespace pog
