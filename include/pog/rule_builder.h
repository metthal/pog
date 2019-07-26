#pragma once

#include <pog/grammar.h>
#include <pog/rule.h>

namespace pog {

template <typename ValueT>
class RuleBuilder
{
public:
	using GrammarType = Grammar<ValueT>;
	using RuleType = Rule<ValueT>;
	using SymbolType = Symbol<ValueT>;

	struct RightHandSide
	{
		std::vector<std::string> symbols;
		typename RuleType::CallbackType action;
		std::optional<Precedence> precedence;
	};

	RuleBuilder(GrammarType* grammar, const std::string& lhs) : _grammar(grammar), _lhs(lhs), _rhss() {}

	void done()
	{
		const auto* lhs_symbol = _grammar->add_symbol(SymbolKind::Nonterminal, _lhs);
		for (auto&& rhs : _rhss)
		{
			std::vector<const SymbolType*> rhs_symbols(rhs.symbols.size());
			std::transform(rhs.symbols.begin(), rhs.symbols.end(), rhs_symbols.begin(), [this](const auto& sym_name) {
				return _grammar->add_symbol(SymbolKind::Nonterminal, sym_name);
			});

			auto rule = _grammar->add_rule(lhs_symbol, rhs_symbols, std::move(rhs.action));
			if (rhs.precedence)
			{
				const auto& prec = rhs.precedence.value();
				rule->set_precedence(prec.level, prec.assoc);
			}
		}
	}

	template <typename... Args>
	RuleBuilder& production(Args&&... args)
	{
		_rhss.push_back(RightHandSide{
			std::vector<std::string>{std::forward<Args>(args)...},
			{},
			std::nullopt
		});
		return *this;
	}

	template <typename CallbackT>
	RuleBuilder& action(CallbackT&& act)
	{
		_rhss.back().action = act;
		return *this;
	}

	RuleBuilder& precedence(std::uint32_t level, Associativity assoc)
	{
		_rhss.back().precedence = Precedence{level, assoc};
		return *this;
	}

private:
	GrammarType* _grammar;
	std::string _lhs;
	std::vector<RightHandSide> _rhss;
};

} // namespace pog
