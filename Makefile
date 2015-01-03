# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g
YACC = bison
LEX = flex

# Link the object files together into the final executable.

v9: v9-lexer.o v9-parser.tab.o ast.o type_info.o
	$(GCC) v9-parser.tab.o v9-lexer.o ast.o type_info.o -o v9 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

v9-lexer.o: v9-lexer.cc v9.lex symbol_table.h table_entry.h
	$(GCC) $(CFLAGS) -c v9-lexer.cc

v9-parser.tab.o: v9-parser.tab.cc v9.y symbol_table.h table_entry.h
	$(GCC) $(CFLAGS) -c v9-parser.tab.cc


# Compile the individual code files into object files.

v9-lexer.cc: v9.lex v9-parser.tab.cc symbol_table.h table_entry.h
	$(LEX) -o v9-lexer.cc v9.lex

v9-parser.tab.cc: v9.y symbol_table.h
	$(YACC) -v -o v9-parser.tab.cc -d v9.y

ast.o: ast.cc ast.h symbol_table.h table_entry.h
	$(GCC) $(CFLAGS) -c ast.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f v9 *.o v9-lexer.cc *.tab.cc *.tab.hh *~
