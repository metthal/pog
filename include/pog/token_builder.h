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

	TokenBuilder(GrammarType* grammar, TokenizerType* tokenizer, const std::string& pattern) : _grammar(grammar), _tokenizer(tokenizer), _pattern(pattern) {}

	void done()
	{
		auto* symbol = !_symbol_name.empty() ? _grammar->add_symbol(SymbolKind::Terminal, _symbol_name) : nullptr;

		auto token = _tokenizer->add_token(_pattern, symbol);
		if (_action)
			token->set_action(std::move(_action));

		if (symbol && _precedence)
		{
			const auto& prec = _precedence.value();
			symbol->set_precedence(prec.level, prec.assoc);
		}
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

private:
	GrammarType* _grammar;
	TokenizerType* _tokenizer;
	std::string _pattern;
	std::string _symbol_name;
	std::optional<Precedence> _precedence;
	typename TokenType::CallbackType _action;
};

} // namespace pog
