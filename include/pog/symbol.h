#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <pog/precedence.h>

namespace pog {

enum class SymbolKind
{
	Start,
	End,
	Nonterminal,
	Terminal
};

template <typename ValueT>
class Symbol
{
public:
	Symbol(std::uint32_t index, SymbolKind kind) : _index(index), _kind(kind), _name() {}
	Symbol(std::uint32_t index, SymbolKind kind, const std::string& name) : _index(index), _kind(kind), _name(name) {}

	std::uint32_t get_index() const { return _index; }
	const Precedence& get_precedence() const { return _precedence.value(); }

	void set_kind(SymbolKind kind) { _kind = kind; }
	void set_precendence(std::uint32_t level, Associativity assoc) { _precedence = Precedence{level, assoc}; }

	bool has_precedence() const { return static_cast<bool>(_precedence); }
	bool is_start() const { return _kind == SymbolKind::Start; }
	bool is_end() const { return _kind == SymbolKind::End; }
	bool is_nonterminal() const { return _kind == SymbolKind::Nonterminal || _kind == SymbolKind::Start; } // TODO: maybe delete start here
	bool is_terminal() const { return _kind == SymbolKind::Terminal; }

	const std::string& get_name() const
	{
		static const std::unordered_map<SymbolKind, std::string> kind_to_name = {
			{ SymbolKind::Start, "<start>" },
			{ SymbolKind::End, "<end>" }
		};

		if (auto itr = kind_to_name.find(_kind); itr != kind_to_name.end())
			return itr->second;

		return _name;
	}

private:
	std::uint32_t _index;
	SymbolKind _kind;
	std::string _name;
	std::optional<Precedence> _precedence;
};

} // namespace pog
