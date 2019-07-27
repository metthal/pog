# Pog

[![Build Status](https://travis-ci.org/metthal/pog.svg?branch=master)](https://travis-ci.org/metthal/pog)
[![codecov](https://codecov.io/gh/metthal/pog/branch/master/graph/badge.svg)](https://codecov.io/gh/metthal/pog)


Pog is C++17 library for generating LALR(1) parsers. It splits definitions of parser into:

1. Declaration of tokens (regular expressions describing how the input should be tokenized)
2. Grammar rules over tokens from tokenization phase

If you are familiar with tools like yacc + lex or bison + flex then this should be already known concept for you.

## Why make another parser generator?

I had idea for project like this for a few years already, back when I used bison + flex for a school project. The advantage of bison + flex is that it generates LALR(1) parser. These parsers are very good for parsing usual programming languages constructs without any transformations of the grammar (such as removal of left-recursion, making sure that no 2 rules have same prefix). Their approach of splitting the process into tokenization and parsing makes it much easier to write the actual grammar without having it cluttered with things like whitespaces, comments and other things that can be ignored and don't have to be present in the grammar itself. The disadvantage of bison + flex is that you have to have these installed on your system because they are standalone tools which will generate you C/C++ code. Maintaining build system which uses them and works on Linux, Windows and macOS is not an easy task. For a long time, bison was also not able to work with modern C++ features such as move semantics. It should be supported as of now (3.4) but a lot of Linux distributions still don't have this version and some stable distros won't have for a very long time. There are also other options than bison + flex in C++ world such as Boost.Spirit or PEGTL which are all amazing but they all have some drawbacks (LL parsers, cluttered and hard to maintain grammars, inability to specify operator precedence, ...). This library aims to provide what was missing out there -- taking philosophy of bison + flex and putting it into pure C++ while still generating LALR(1) parser.

## Roadmap

Things to do before 1.0.0

- [ ] Tokenizer action on end of input
- [ ] Support for states in tokenizer (`BEGIN`, `END` like behavior in flex)
- [ ] Generate debugging report (text file parsing table, states, lookahead sets, graphviz LALR autmaton, ...)
- [ ] Windows Build
- [ ] Tests
- [ ] Code Coverage
- [ ] CI pipeline (Travis CI, AppVeyor)
- [ ] Packaging (ZIP, RPM, DEB, ... + getting package into repositories)
- [ ] Sphinx Docs (+ Read The Docs integration)

Things to do after 1.0.0

- [ ] Error Recovery
- [ ] Code Cleanup :)
- [ ] Own implementation for tokenizer to be header-only (DFA)
- [ ] Lightweight iterator ranges
