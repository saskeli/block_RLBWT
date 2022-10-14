CFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

CLANGFLAGS = -std=c++2b -Wall -Wextra -Wshadow -pedantic -march=native

HEADERS = include/reader.hpp include/block_rlbwt_builder.hpp include/block_rlbwt.hpp \
          include/byte_block.hpp include/byte_alphabet.hpp include/super_block.hpp \
		  include/types.hpp include/two_byte_block.hpp

COVERAGE = -g

.PHONY: clean update_git debug all clang cdebug

.DEFAULT: all

%/%.hpp:

all: make_bwt read_bwt clang

clang: cmake_bwt cread_bwt

cmake_bwt: make_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DNDEBUG -O3 -o cmake_bwt make_bwt.cpp

cread_bwt: read_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DNDEBUG -O3 -o cread_bwt read_bwt.cpp

make_bwt: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o make_bwt make_bwt.cpp

read_bwt: read_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o read_bwt read_bwt.cpp

cdebug: make_bwt.cpp read_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DDEBUG -g -O0 -o cmake_bwt make_bwt.cpp
	clang++ $(CLANGFLAGS) -DDEBUG -g -O0 -o cread_bwt read_bwt.cpp

debug: make_bwt.cpp read_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DDEBUG -g -o make_bwt make_bwt.cpp
	g++ $(CFLAGS) -DDEBUG -g -o read_bwt read_bwt.cpp

clean:
	rm -f make_bwt

update_git:
	git submodule update