#pragma once

#include <fstream>

#include <pog/grammar.h>
#include <pog/parsing_table.h>

namespace pog {

template <typename ValueT>
class HtmlReport
{
public:
	using AutomatonType = Automaton<ValueT>;
	using GrammarType = Grammar<ValueT>;
	using ParsingTableType = ParsingTable<ValueT>;

	using ShiftActionType = Shift<ValueT>;
	using ReduceActionType = Reduce<ValueT>;

	HtmlReport(const GrammarType* grammar, const AutomatonType* automaton, const ParsingTableType* parsing_table)
		: _grammar(grammar), _automaton(automaton), _parsing_table(parsing_table) {}

	void save(const std::string& file_path)
	{
		static const std::string html_page_template = R"(<!doctype html>
<html lang="en">
	<head>
		<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
		<link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">
		<link href="https://stackpath.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css" rel="stylesheet" integrity="sha384-wvfXpqpZZVQGK6TAh5PVlGOfQNHSoD2xbE+QkPxCAFlNEevoEH3Sl0sibVcOQVnN" crossorigin="anonymous">
		<style>
			#parsing_table {{
				width: auto;
			}}
			#parsing_table td, #parsing_table th {{
				min-width: 2em;
				max-width: 6em;
				word-wrap: break-word;
				text-align: center;
			}}
		</style>
	</head>
	<body>
		<nav class="navbar navbar-dark bg-dark">
			<span class="navbar-brand">pog report</span>
		</nav>
		<div class="container">
			<div class="pt-3 row justify-content-md-center">
				<table id="parsing_table" class="table table-bordered">
					<thead class="thead-dark">
						<tr>
							<th style="text-align: center" rowspan="2">State</th>
							<th style="text-align: center" colspan="{number_of_terminals}">Action</th>
							<th style="text-align: center" colspan="{number_of_nonterminals}">Goto</th>
						</tr>
						<tr>
							{symbols}
						</tr>
					</thead>
					<tbody>
						{rows}
					</tbody>
				</table>
			</div>
			<div class="pt-3 row justify-content-md-center">
				{states}
			</div>
			<div class="pt-3 row justify-content-md-center">
				<small>Generated: {generated_at}</small>
			</div>
		</div>
		<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
		<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
		<script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>
		<script type="text/javascript">
$(document).ready(function() {{
	$('[data-toggle="tooltip"]').tooltip();
}});
		</script>
	</body>
</html>)";

		using namespace fmt;

		auto terminal_symbols = _grammar->get_terminal_symbols();
		auto nonterminal_symbols = _grammar->get_nonterminal_symbols();

		std::vector<std::string> symbol_headers(terminal_symbols.size() + nonterminal_symbols.size());
		std::transform(terminal_symbols.begin(), terminal_symbols.end(), symbol_headers.begin(), [](const auto& s) {
			return fmt::format("<th>{}</th>", s->get_name());
		});
		std::transform(nonterminal_symbols.begin(), nonterminal_symbols.end(), symbol_headers.begin() + terminal_symbols.size(), [](const auto& s) {
			return fmt::format("<th>{}</th>", s->get_name());
		});

		std::vector<std::string> rows(_automaton->get_states().size());
		for (const auto& state : _automaton->get_states())
		{
			std::vector<std::string> row;
			row.push_back(fmt::format(
				"<tr><td data-toggle=\"tooltip\" data-placement=\"bottom\" title=\"{}\" data-html=\"true\">{}</td>",
				state->to_string("→", "ε", "•", "<br/>"),
				state->get_index()
			));
			for (const auto& sym : terminal_symbols)
			{
				auto action = _parsing_table->get_action(state.get(), sym);
				if (!action)
				{
					row.push_back("<td></td>");
					continue;
				}

				row.push_back(visit_with(action.value(),
					[](const ShiftActionType& shift) {
						return fmt::format(
							"<td data-toggle=\"tooltip\" data-placement=\"bottom\" title=\"{}\" data-html=\"true\">s{}</td>",
							shift.state->to_string("→", "ε", "•", "<br/>"),
							shift.state->get_index()
						);
					},
					[](const ReduceActionType& reduce) {
						return fmt::format(
							"<td data-toggle=\"tooltip\" data-placement=\"bottom\" title=\"{}\">r{}</td>",
							reduce.rule->to_string("→", "ε"),
							reduce.rule->get_index()
						);
					},
					[](const Accept&) -> std::string { return "<td><span class=\"fa fa-check-square\" style=\"color: green\"/></td>"; }
				));
			}

			for (const auto& sym : nonterminal_symbols)
			{
				auto go_to = _parsing_table->get_transition(state.get(), sym);
				if (!go_to)
				{
					row.push_back("<td></td>");
					continue;
				}

				row.push_back(fmt::format(
					"<td data-toggle=\"tooltip\" data-placement=\"bottom\" title=\"{}\" data-html=\"true\">{}</td>",
					go_to.value()->to_string("→", "ε", "•", "<br/>"),
					go_to.value()->get_index()
				));
			}

			row.push_back("</tr>");
			rows.push_back(fmt::format("{}", fmt::join(row.begin(), row.end(), "")));
		}

		std::ofstream file(file_path, std::ios::out | std::ios::trunc);
		if (file.is_open())
			file << fmt::format(html_page_template,
				"number_of_terminals"_a = fmt::format("{}", terminal_symbols.size()),
				"number_of_nonterminals"_a = fmt::format("{}", nonterminal_symbols.size()),
				"symbols"_a = fmt::format("{}", fmt::join(symbol_headers.begin(), symbol_headers.end(), "")),
				"rows"_a = fmt::format("{}", fmt::join(rows.begin(), rows.end(), "\n")),
				"states"_a = "HERE COME STATES",
				"generated_at"_a = current_time("%Y-%m-%d %H:%M:%S %Z")
			);
		file.close();
	}

private:
	const GrammarType* _grammar;
	const AutomatonType* _automaton;
	const ParsingTableType* _parsing_table;
};

} // namespace pog
