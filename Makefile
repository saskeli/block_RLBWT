CFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

HEADERS = include/reader.hpp

COVERAGE = -g

.PHONY: clean update_git

.DEFAULT: make_bwt

all: make_bwt

make_bwt: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -o make_bwt make_bwt.cpp

inclue/%.hpp:

clean:
	rm -f make_bwt

update_git:
	git submodule update