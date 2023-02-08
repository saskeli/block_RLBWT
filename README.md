# FM-index implementation that uses blocks with a constant number of runs or symbols

If you find this repository useful, please cite the following:

```bib
@inproceedings{ddps2023sea,
  title     = {Simple Runs-bounded FM-index Designs are Fast},
  author    = {Diaz, Diego and Dönges, Saska and Puglisi, Simon and Salmela, Leena},
  booktitle = {TBD},
  year      = {2023},
  pages     = {},
  ee        = {doi.org}
}
```

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
