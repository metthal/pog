=========
Debugging
=========

Debugging errors in parser can be a hard process since its whole complexity and the amount of data and code you need to go through to even reach the source of your problems. This sections describes
some options you have to debug the problems in your parser.

HTML report
===========

You are able to generate HTML report out of your parser. The output is HTML file which contains full parsing table, contains information about LR automaton and it even includes Graphviz
representation of LR automaton. In order to generate HTML report, initialize it with your ``Parser`` after you've prepared it.

.. code-block:: cpp

  Parser<Value> parser;

  // tokens & rules

  // Don't forget to first call prepare()
  parser.prepare();

  HtmlReport html(parser);
  html.save("parser.html");

You can now open ``parser.html`` in your current working directory and inspect the inner structure of your parser.

LR automaton
============

Generated HTML report will contain `Graphviz <https://www.graphviz.org/>`_ representation of LR automaton at the very bottom. Copy it over to some other file and run following command to turn it
into PNG image.

.. code-block:: bash

  dot -Tpng -o automaton.png <INPUT_FILE>

Runtime debugging
=================

Since parsers are rather complex, it is hard to debug them line by line because there is too much going on and we are only mostly interested in what is parser doing and not what is the whole framework doing.
Therefore we provide an option to compile your project so that *pog* prints out additional debug messages on standard debug output. In order to use them, define preprocessor macro ``POG_DEBUG``
before including the very first *pog* related header file. That will cause parser and tokenizer to print out debugging messages. If you want only parser or only tokenizer messages, define
``POG_DEBUG_PARSER`` or ``POG_DEBUG_TOKENIZER`` instead of ``POG_DEBUG``. These messages have no overhead if they are not requested to be printed so you don't have to worry about that.
