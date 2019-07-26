#pragma once

#include <functional>
#include <memory>
#include <string>

#include <re2/re2.h>

#include <pog/symbol.h>

namespace pog {

template <typename ValueT>
class Token
{
public:
	using SymbolType = Symbol<ValueT>;
	using CallbackType = std::function<ValueT(std::string_view)>;

	Token(std::uint32_t index, const std::string& pattern) : Token(index, pattern, nullptr) {}
	Token(std::uint32_t index, const std::string& pattern, const SymbolType* symbol)
		: _index(index), _pattern(pattern), _symbol(symbol), _regexp(std::make_unique<re2::RE2>(_pattern)), _action() {}

	const std::string& get_pattern() const { return _pattern; }
	const SymbolType* get_symbol() const { return _symbol; }
	const re2::RE2* get_regexp() const { return _regexp.get(); }

	bool has_symbol() const { return _symbol != nullptr; }
	bool has_action() const { return static_cast<bool>(_action); }

	template <typename CallbackT>
	void set_action(CallbackT&& action)
	{
		_action = std::forward<CallbackT>(action);
	}

	template <typename... Args>
	ValueT perform_action(Args&&... args) const
	{
		return _action(std::forward<Args>(args)...);
	}

private:
	std::uint32_t _index;
	std::string _pattern;
	const SymbolType* _symbol;
	std::unique_ptr<re2::RE2> _regexp;
	CallbackType _action;
};

} // namespace pog
