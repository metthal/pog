# dev

* Added support for global tokenizer actions
* Added option to specify symbol description which can provide more human friendly output for the symbol in case of a syntax error
* Improved performance of constructing parser (construction of LR automaton to be precise)
* Debugging traces of parser and tokenizer now have no effect when debugging is not turned on

# v0.4.0 (2019-09-28)

* Fixed calulcation of includes and lookback relations when there are more instances of the same symbol inspected
* Added option to define `POG_DEBUG` to print debugging messages from parser adn tokenizer

# v0.3.0 (2019-09-22)

* Midrule actions and all preceding symbols are now accessible from later actions in that rule
* Explicit switching of tokenizer state with `enter_tokenizer_state` method
* Implicit end of input token now has modifiable states in which it is active

# v0.2.1 (2019-09-13)

* Added option `POG_PIC` to build position-independent code

# v0.2.0 (2019-08-12)

* Added support for tokenizer action when end of input stream is reached
* Token actions are now always called exactly once for each individual token
* Tokenizer now supports states
* Generation of HTML report for parsers
* Added support for build on Windows (using MSVC) and macOS
* Added support for mid-rule actions

# v0.1.0 (2019-07-27)

* Initial release
