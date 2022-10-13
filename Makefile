CFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

HEADERS = include/reader.hpp include/block_rlbwt_builder.hpp include/block_rlbwt.hpp \
          include/byte_block.hpp include/byte_alphabet.hpp include/super_block.hpp \
		  include/types.hpp include/two_byte_block.hpp

COVERAGE = -g

.PHONY: clean update_git debug

.DEFAULT: all

%/%.hpp:

all: make_bwt read_bwt

make_bwt: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o make_bwt make_bwt.cpp

read_bwt: read_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o read_bwt read_bwt.cpp

debug: make_bwt.cpp make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DDEBUG -g -o make_bwt make_bwt.cpp
	g++ $(CFLAGS) -DDEBUG -g -o read_bwt read_bwt.cpp

clean:
	rm -f make_bwt

update_git:
	git submodule update