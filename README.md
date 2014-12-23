# V9 JavaScript Engine

V9 is a basic JavaScript engine written in C++ using flex and bison.

## Installing

Download or clone the source and run `make` within. `flex` and `bison` are required.

An executable named `v9` will be created.

## Running

An REPL is not available yet, so V9 must be run with a JavaScript source file as
an argument.

For example:

    $ v9 hello_world.js
    Hello World!
