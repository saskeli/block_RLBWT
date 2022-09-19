CFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

HEADERS = include/reader.hpp include/block_rlbwt_builder.hpp include/simple_rlbwt.hpp include/byte_block.hpp

COVERAGE = -g

.PHONY: clean update_git debug

.DEFAULT: make_bwt

all: make_bwt

make_bwt: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o make_bwt make_bwt.cpp

debug: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DDEBUG -g -o make_bwt make_bwt.cpp

inclue/%.hpp:

clean:
	rm -f make_bwt

update_git:
	git submodule update