add_subdirectory(googletest)

set(TEST_FILES
	pog_tests.cpp
	test_automaton.cpp
	test_grammar.cpp
	test_item.cpp
	test_parser.cpp
	test_parsing_table.cpp
	test_precedence.cpp
	test_rule.cpp
	test_state.cpp
	test_symbol.cpp
	test_token.cpp
	test_utils.cpp
)

add_executable(pog_tests ${TEST_FILES})
target_link_libraries(pog_tests pog googletest)
