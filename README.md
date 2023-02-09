# FM-index implementation that uses blocks with a constant number of runs or symbols

If you find this repository useful, please cite

```bib
@inproceedings{ddps2023sea,
  title     = {Simple Runs-bounded FM-index Designs are Fast},
  author    = {Díaz-Domínguez, Diego and Dönges, Saska and Puglisi, Simon and Salmela, Leena},
  booktitle = {TBD},
  year      = {2023},
  pages     = {},
  ee        = {doi.org}
}
```

## TL/DR of what this is

BWT indexing by splitting into blocks of $b$ symbols or $b'$ runs. Block-based structure enables fast `rank` queries while not sacrificing too much space.

Supports access, rank and pattern counting.

Mochrobenchmarks for predecessor search support structure related to blocks of $b'$ runs are available at:
* https://github.com/saskeli/binary_search_patterns and
* https://github.com/saskeli/search_microbench


## Building indexes

Given a plain text BWT file `/path/to/bwt.txt`, to make the index `bwt.rlbwt` with associated data `bwt_data.rlbwt` run the following in the repository root

```bash
$ make make_alphabet_header
$ ./make_alphabet_header -i /path/to/bwt.txt > include/custom_alphabet.hpp
$ make make_bwt
$ ./make_bwt -i /path/to/bwt.txt bwt.rlbwt
```

This will create and index with block size $2^{11}$ and runs endcoded by splitting runs as necessary to store runs in two bytes per run. Run `./make_bwt` for more information on how to generate different versions of the indexes.

## Benchmarking indexes

Given a default index `bwt.rlbwt` and a pattern file `patterns.txt` containing one pattern per line do:

```bash
$ make count_matches
$ ./count_matches bwt.rlbwt patterns.txt
```

To count the number of matches for each pattern in `bwt.rlbwt`. Results for each query will be output to standard out, and summary statistics to std::cerr. Run `./count_matches` for information on how to benchmark other index variants.

## Using the indexes

Include the headers in the `include` directory in your project. `types.hpp` contains default index types that can be used. Given a default index `bwt.rlbwt` the following can be used to load and query the index.

```c++
#include <iostream>
#include "include/types.hpp"

main() {
    bbwt::two_byte<> bwt("bwt.rlbwt");
    std::cout << bwt.count("Einstein") << std::endl;
}
```

Other hopefully useful defualt index variants are `bbwt::runs<>´ and ´bbwt::vbyte<>´. Different blocks sizes can be entered as template parameters.

## Requirements

Compilation and execution has been tested on multiple modern x86-64 systems and GCC supporting `-std=c++2a`. Code should be compatible with other compilers, but is not expected to compile correctly on compilers not supporting C++ 20. The project should also work on new ARM based apple systems.

## Todo

* Include functionality of `make_alphabe_header` in `make_bwt` to eliminate additional compilations steps in index construction
* See what effect eliminating super blocks would have. Probably almost nothing but less code is better.
* Make namespaces clearer to make the project more usefull as a library header for end users.
* See about entropy encoding run heads. Should be able to implement without significant performance hit, and could save significant space.
* Extract rare symbols?
* Separate block encodings for blocks with different properties?
