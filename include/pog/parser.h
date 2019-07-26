#pragma once

#include <deque>
#include <unordered_map>

#include <pog/action.h>
#include <pog/automaton.h>
#include <pog/grammar.h>
#include <pog/parsing_table.h>
#include <pog/state.h>
#include <pog/symbol.h>
#include <pog/tokenizer.h>

#include <pog/operations/read.h>
#include <pog/operations/follow.h>
#include <pog/operations/lookahead.h>
#include <pog/relations/includes.h>
#include <pog/relations/lookback.h>

namespace pog {

template <typename ValueT>
class Parser
{
public:
	using ActionType = Action<ValueT>;
	using ShiftActionType = Shift<ValueT>;
	using ReduceActionType = Reduce<ValueT>;

	using BacktrackingInfoType = BacktrackingInfo<ValueT>;
	using ItemType = Item<ValueT>;
	using RuleType = Rule<ValueT>;
	using StateType = State<ValueT>;
	using SymbolType = Symbol<ValueT>;
	using StateAndRuleType = StateAndRule<ValueT>;
	using StateAndSymbolType = StateAndSymbol<ValueT>;
	using TokenType = Token<ValueT>;

	Parser() : _grammar(), _tokenizer(&_grammar), _automaton(&_grammar),
		_includes(&_automaton, &_grammar), _lookback(&_automaton, &_grammar), _read_operation(&_automaton, &_grammar),
		_follow_operation(&_automaton, &_grammar, _includes, _read_operation), _lookahead_operation(&_automaton, &_grammar, _lookback, _follow_operation),
		_parsing_table(&_automaton, &_grammar, _lookahead_operation)
	{
		static_assert(std::is_default_constructible_v<ValueT>, "Value type needs to be default constructible");
	}

	Parser(const Parser<ValueT>&) = delete;
	Parser(Parser<ValueT>&&) noexcept = default;

	void prepare()
	{
		_automaton.construct_states();
		_includes.calculate();
		_lookback.calculate();
		_read_operation.calculate();
		_follow_operation.calculate();
		_lookahead_operation.calculate();
		_parsing_table.calculate();
	}

	const Grammar<ValueT>& get_grammar() const
	{
		return _grammar;
	}

	template <typename... Args>
	SymbolType* add_symbol(Args&&... args)
	{
		return _grammar.add_symbol(std::forward<Args>(args)...);
	}

	template <typename... Args>
	RuleType* add_rule(Args&&... args)
	{
		return _grammar.add_rule(std::forward<Args>(args)...);
	}

	template <typename... Args>
	const TokenType* add_token(Args&&... args)
	{
		return _tokenizer.add_token(std::forward<Args>(args)...);
	}

	void set_start_symbol(const SymbolType* symbol)
	{
		_grammar.set_start_symbol(symbol);
	}

	std::optional<ValueT> parse(std::istream& input)
	{
		_tokenizer.prepare();
		_tokenizer.push_input_stream(input);

		std::deque<std::pair<std::uint32_t, std::optional<ValueT>>> stack;
		stack.emplace_back(0, std::nullopt);

		while (!stack.empty())
		{
			auto maybe_token = _tokenizer.next_token();
			if (!maybe_token)
				assert(false && "Unexpected end of input");

			auto token = std::move(maybe_token).value();
			const auto* next_symbol = token.symbol;

			auto maybe_action = _parsing_table.get_action(_automaton.get_state(stack.back().first), next_symbol);
			if (!maybe_action)
				assert(false && "Syntax error");

			// TODO: use visit
			auto action = maybe_action.value();
			if (std::holds_alternative<ReduceActionType>(action))
			{
				const auto& reduce = std::get<ReduceActionType>(action);

				// Each symbol on right-hand side of the rule should have record on the stack
				// We'll pop them out and put them in reverse order so user have them available
				// left-to-right and not right-to-left.
				std::vector<ValueT> action_arg;
				action_arg.reserve(reduce.rule->get_rhs().size());
				assert(stack.size() >= reduce.rule->get_rhs().size() && "Stack is too small");

				for (std::size_t i = 0; i < action_arg.capacity(); ++i)
				{
					// Notice how std::move() is only around optional itself and not the whole expressions
					// We need to do this in order to perform move together with value_or()
					// See: https://en.cppreference.com/w/cpp/utility/optional/value_or
					// std::move(*this) is performed only when value_or() is called from r-value
					action_arg.insert(action_arg.begin(), std::move(stack.back().second).value_or(ValueT{}));
					stack.pop_back();
				}

				// What left on the stack now determines what state we get into now
				auto maybe_next_state = _parsing_table.get_transition(_automaton.get_state(stack.back().first), reduce.rule->get_lhs());
				if (!maybe_next_state)
					assert(false && "Syntax error");

				stack.emplace_back(
					maybe_next_state.value()->get_index(),
					reduce.rule->perform_action(std::move(action_arg))
				);
			}
			else if (std::holds_alternative<ShiftActionType>(action))
			{
				stack.emplace_back(
					std::get<ShiftActionType>(action).state->get_index(),
					std::move(token.value)
				);

				_tokenizer.consume_matched_token(token);
			}
			else if (std::holds_alternative<Accept>(action))
			{
				// Notice how std::move() is only around optional itself and not the whole expressions
				// We need to do this in order to perform move together with value()
				// See: https://en.cppreference.com/w/cpp/utility/optional/value
				// Return by rvalue is performed only when value() is called from r-value
				return std::move(stack.back().second).value();
			}
		}

		assert(false && "Syntax error");
		return std::nullopt;
	}

	std::string generate_automaton_graph()
	{
		return _automaton.generate_graph();
	}

	std::string generate_includes_relation_graph()
	{
		return _includes.generate_relation_graph();
	}

private:
	Grammar<ValueT> _grammar;
	Tokenizer<ValueT> _tokenizer;
	Automaton<ValueT> _automaton;
	Includes<ValueT> _includes;
	Lookback<ValueT> _lookback;
	Read<ValueT> _read_operation;
	Follow<ValueT> _follow_operation;
	Lookahead<ValueT> _lookahead_operation;
	ParsingTable<ValueT> _parsing_table;
};

} // namespace pog
