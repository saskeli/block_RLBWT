CFLAGS = -std=c++2a -Wall -Wextra -Wshadow -pedantic -march=native

CLANGFLAGS = -std=c++2b -Wall -Wextra -Wshadow -pedantic -march=native

HEADERS = include/reader.hpp include/block_rlbwt_builder.hpp include/block_rlbwt.hpp \
          include/byte_block.hpp include/byte_alphabet.hpp include/super_block.hpp \
		  include/types.hpp include/two_byte_block.hpp include/custom_alphabet.hpp \
		  include/one_byte_block.hpp include/d_block.hpp include/acgtn_alphabet.hpp

COVERAGE = -g

.PHONY: clean update_git debug all clang cdebug

.DEFAULT: all

%/%.hpp:

all: gpp make_alphabet_header clang

gpp: make_bwt bench_bwt

clang: cmake_bwt cbench_bwt

cmake_bwt: make_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DNDEBUG -O3 -o cmake_bwt make_bwt.cpp

cbench_bwt: bench_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DNDEBUG -O3 -o cbench_bwt bench_bwt.cpp

make_bwt: make_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o make_bwt make_bwt.cpp

bench_bwt: bench_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DNDEBUG -Ofast -o bench_bwt bench_bwt.cpp

make_alphabet_header: make_alphabet_header.cpp include/reader.hpp
	g++ $(CFLAGS) -DNDEBUG -Ofast -o make_alphabet_header make_alphabet_header.cpp

cdebug: make_bwt.cpp bench_bwt.cpp $(HEADERS)
	clang++ $(CLANGFLAGS) -DDEBUG -g -O0 -o cmake_bwt make_bwt.cpp
	clang++ $(CLANGFLAGS) -DDEBUG -g -O0 -o cbench_bwt bench_bwt.cpp

debug: make_bwt.cpp bench_bwt.cpp $(HEADERS)
	g++ $(CFLAGS) -DDEBUG -g -o make_bwt make_bwt.cpp
	g++ $(CFLAGS) -DDEBUG -g -o bench_bwt bench_bwt.cpp

clean:
	rm -f make_bwt

update_git:
	git submodule update