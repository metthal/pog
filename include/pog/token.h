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

	Token(std::uint32_t index, const std::string& pattern) : Token(index, nullptr, pattern) {}
	Token(std::uint32_t index, SymbolType* symbol, const std::string& pattern)
		: _index(index), _symbol(symbol), _pattern(pattern), _regexp(std::make_unique<re2::RE2>(_pattern)), _action()
	{
		if (_symbol)
			_symbol->set_kind(SymbolKind::Terminal);
	}

	template <typename CallbackT>
	Token(std::uint32_t index, const std::string& pattern, CallbackT&& action) : Token(index, nullptr, pattern, std::forward<CallbackT>(action)) {}

	template <typename CallbackT>
	Token(std::uint32_t index, SymbolType* symbol, const std::string& pattern, CallbackT&& action)
		: _index(index), _symbol(symbol), _pattern(pattern), _regexp(std::make_unique<re2::RE2>(_pattern)), _action(std::forward<CallbackT>(action))
	{
		if (_symbol)
			_symbol->set_kind(SymbolKind::Terminal);
	}

	const SymbolType* get_symbol() const { return _symbol; }
	const std::string& get_pattern() const { return _pattern; }
	const re2::RE2* get_regexp() const { return _regexp.get(); }

	bool has_symbol() const { return _symbol != nullptr; }
	bool has_action() const { return static_cast<bool>(_action); }

	template <typename... Args>
	ValueT perform_action(Args&&... args) const
	{
		return _action(std::forward<Args>(args)...);
	}

private:
	std::uint32_t _index;
	SymbolType* _symbol;
	std::string _pattern;
	std::unique_ptr<re2::RE2> _regexp;
	std::function<ValueT(std::string_view)> _action;
};

} // namespace pog
