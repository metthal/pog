#pragma once

#include <memory>
#include <unordered_set>
#include <vector>

#include <pog/rule.h>
#include <pog/symbol.h>
#include <pog/token.h>

namespace pog {

template <typename ValueT>
class Grammar
{
public:
	using RuleType = Rule<ValueT>;
	using SymbolType = Symbol<ValueT>;
	using TokenType = Token<ValueT>;

	Grammar()
	{
		_start_symbol = nullptr;
		_internal_start_symbol = add_symbol(SymbolKind::Start);
		_internal_end_of_input = add_symbol(SymbolKind::End);
	}

	const std::vector<std::unique_ptr<SymbolType>>& get_symbols() const
	{
		return _symbols;
	}

	const std::vector<std::unique_ptr<RuleType>>& get_rules() const
	{
		return _rules;
	}

	const SymbolType* get_end_of_input_symbol() const
	{
		return _internal_end_of_input;
	}

	const RuleType* get_start_rule() const
	{
		auto itr = std::find_if(_rules.begin(), _rules.end(), [](const auto& rule) {
			return rule->get_lhs()->is_start();
		});
		return itr != _rules.end() ? itr->get() : nullptr;
	}

	std::vector<const RuleType*> get_rules_of_symbol(const SymbolType* sym) const
	{
		std::vector<const RuleType*> result;
		transform_if(_rules.begin(), _rules.end(), std::back_inserter(result), [&](const auto& rule) {
			return sym == rule->get_lhs();
		}, [](const auto& rule) {
			return rule.get();
		});
		return result;
	}

	std::vector<const RuleType*> get_rules_with_symbol(const SymbolType* sym) const
	{
		std::vector<const RuleType*> result;
		transform_if(_rules.begin(), _rules.end(), std::back_inserter(result), [&](const auto& rule) {
			return std::find(rule->get_rhs().begin(), rule->get_rhs().end(), sym) != rule->get_rhs().end();
		}, [](const auto& rule) {
			return rule.get();
		});
		return result;
	}

	void set_start_symbol(const SymbolType* symbol)
	{
		_start_symbol = symbol;
		add_rule(_internal_start_symbol, std::vector<const SymbolType*>{_start_symbol, _internal_end_of_input}, [](auto&& args) {
			return std::move(args[0]);
		});
	}

	SymbolType* add_symbol(SymbolKind kind)
	{
		_symbols.push_back(std::make_unique<SymbolType>(_symbols.size(), kind));
		return _symbols.back().get();
	}

	SymbolType* add_symbol(const std::string& name)
	{
		_symbols.push_back(std::make_unique<SymbolType>(_symbols.size(), SymbolKind::Nonterminal, name));
		return _symbols.back().get();
	}

	template <typename... Args>
	RuleType* add_rule(Args&&... args)
	{
		_rules.push_back(std::make_unique<RuleType>(_rules.size(), std::forward<Args>(args)...));
		return _rules.back().get();
	}

	bool empty(const SymbolType* sym) const
	{
		std::unordered_set<const SymbolType*> visited_lhss;
		auto result = empty(sym, visited_lhss);
		_empty_table[sym] = result;
		return result;
	}

	bool empty(const std::vector<const SymbolType*>& seq) const
	{
		std::unordered_set<const SymbolType*> visited_lhss;
		auto result = empty(seq, visited_lhss);
		return result;
	}

	std::unordered_set<const SymbolType*> first(const SymbolType* sym) const
	{
		std::unordered_set<const SymbolType*> visited_lhss;
		auto result = first(sym, visited_lhss);
		_first_table[sym] = result;
		return result;
	}

	std::unordered_set<const SymbolType*> first(const std::vector<const SymbolType*>& seq) const
	{
		std::unordered_set<const SymbolType*> visited_lhss;
		auto result = first(seq, visited_lhss);
		return result;
	}

	std::unordered_set<const SymbolType*> follow(const SymbolType* sym)
	{
		std::unordered_set<const SymbolType*> visited;
		auto result = follow(sym, visited);
		_follow_table[sym] = result;
		return result;
	}

	bool empty(const SymbolType* sym, std::unordered_set<const SymbolType*>& visited_lhss) const
	{
		if (auto itr = _empty_table.find(sym); itr != _empty_table.end())
			return itr->second;

		// Empty of terminal or end of input marker '$' remains the same
		if (sym->is_terminal() || sym->is_end())
			return false;

		// In case of nonterminal A, there exist rules A -> ... and we need to inspect them all
		auto rules = get_rules_of_symbol(sym);
		for (const auto* rule : rules)
		{
			visited_lhss.insert(rule->get_lhs());
			if (empty(rule->get_rhs(), visited_lhss))
				return true;
		}

		return false;
	}

	bool empty(const std::vector<const SymbolType*>& seq, std::unordered_set<const SymbolType*>& visited_lhss) const
	{
		for (const auto* sym : seq)
		{
			if (auto itr = visited_lhss.find(sym); itr == visited_lhss.end())
			{
				if (!empty(sym, visited_lhss))
					return false;
			}
			else
				return false;
		}

		return true;
	}

	std::unordered_set<const SymbolType*> first(const SymbolType* sym, std::unordered_set<const SymbolType*>& visited_lhss) const
	{
		if (auto itr = _first_table.find(sym); itr != _first_table.end())
			return itr->second;

		if (sym->is_terminal() || sym->is_end())
			return {sym};
		else if (sym->is_nonterminal())
		{
			std::unordered_set<const SymbolType*> result;

			if (auto itr = visited_lhss.find(sym); itr == visited_lhss.end())
			{
				visited_lhss.insert(sym);
				auto rules = get_rules_of_symbol(sym);
				for (const auto* rule : rules)
				{
					auto tmp = first(rule->get_rhs(), visited_lhss);
					std::set_union(result.begin(), result.end(), tmp.begin(), tmp.end(), std::inserter(result, result.begin()));
				}
			}

			return result;
		}

		return {};
	}

	std::unordered_set<const SymbolType*> first(const std::vector<const SymbolType*>& seq, std::unordered_set<const SymbolType*>& visited_lhss) const
	{
		std::unordered_set<const SymbolType*> result;

		for (const auto* sym : seq)
		{
			auto tmp = first(sym, visited_lhss);
			std::set_union(result.begin(), result.end(), tmp.begin(), tmp.end(), std::inserter(result, result.begin()));
			if (!empty(sym))
				break;
		}

		return result;
	}

	std::unordered_set<const SymbolType*> follow(const SymbolType* sym, std::unordered_set<const SymbolType*>& visited)
	{
		if (auto itr = _follow_table.find(sym); itr != _follow_table.end())
			return itr->second;

		std::unordered_set<const SymbolType*> result;
		if (visited.find(sym) != visited.end())
			return result;

		visited.insert(sym);
		auto rules = get_rules_with_symbol(sym);
		for (const auto* rule : rules)
		{
			for (auto itr = rule->get_rhs().begin(), end = rule->get_rhs().end(); itr != end; ++itr)
			{
				if (*itr == sym)
				{
					bool can_be_last_in_production = true;
					// If we have a production A -> a B b and we are doing Follow(B), we need to inspect everything
					// right of B (so in this case 'b') whether there exist production b =>* eps.
					// If so, B can be last in production and and therefore we need to add Follow(A) to Follow(B)
					if (itr + 1 != end)
					{
						std::vector<const SymbolType*> tail(itr + 1, end);
						auto tmp = first(tail);
						std::set_union(result.begin(), result.end(), tmp.begin(), tmp.end(), std::inserter(result, result.begin()));
						can_be_last_in_production = empty(tail);
					}

					// There exists production b =>* eps so add Follow(A) to Follow(B)
					if (can_be_last_in_production)
					{
						auto tmp = follow(rule->get_lhs(), visited);
						std::set_union(result.begin(), result.end(), tmp.begin(), tmp.end(), std::inserter(result, result.begin()));
					}
				}
			}
		}

		return result;
	}

private:
	std::vector<std::unique_ptr<RuleType>> _rules;
	std::vector<std::unique_ptr<SymbolType>> _symbols;
	const SymbolType* _start_symbol;
	const SymbolType* _internal_start_symbol;
	const SymbolType* _internal_end_of_input;

	mutable std::unordered_map<const SymbolType*, bool> _empty_table;
	mutable std::unordered_map<const SymbolType*, std::unordered_set<const SymbolType*>> _first_table;
	std::unordered_map<const SymbolType*, std::unordered_set<const SymbolType*>> _follow_table;
};

} // namespace pog
