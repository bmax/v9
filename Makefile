# Setup some aliases to these can be easily altered in the future.
GCC = g++
CFLAGS = -g
YACC = bison
LEX = flex

# Link the object files together into the final executable.

tube4: tube4-lexer.o tube4-parser.tab.o ast.o ic.o type_info.o
	$(GCC) tube4-parser.tab.o tube4-lexer.o ast.o ic.o type_info.o -o tube4 -ll -ly


# Use the lex and yacc templates to build the C++ code files.

tube4-lexer.o: tube4-lexer.cc tube4.lex symbol_table.h
	$(GCC) $(CFLAGS) -c tube4-lexer.cc

tube4-parser.tab.o: tube4-parser.tab.cc tube4.y symbol_table.h
	$(GCC) $(CFLAGS) -c tube4-parser.tab.cc


# Compile the individual code files into object files.

tube4-lexer.cc: tube4.lex tube4-parser.tab.cc symbol_table.h
	$(LEX) -otube4-lexer.cc tube4.lex

tube4-parser.tab.cc: tube4.y symbol_table.h
	$(YACC) -o tube4-parser.tab.cc -d tube4.y

ast.o: ast.cc ast.h symbol_table.h
	$(GCC) $(CFLAGS) -c ast.cc

ic.o: ic.cc ic.h symbol_table.h
	$(GCC) $(CFLAGS) -c ic.cc

type_info.o: type_info.h type_info.cc
	$(GCC) $(CFLAGS) -c type_info.cc


# Cleanup all auto-generated files

clean:
	rm -f tube4 *.o tube4-lexer.cc *.tab.cc *.tab.hh *~
