#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

static const constexpr uint32_t BLOCK_SIZE = 1 << 12;
static const constexpr uint32_t BUFFER_SIZE = 100;

void help() {
    std::cout << "Benchmark RLBWT data structure.\n\n";
    std::cout << "Usage: bench_bwt file_name [-n num] [-a alphabet]\n";
    std::cout << "   file_name      Path to block rlbwt root element.\n";
    std::cout << "   -n num         Number of tests to run. Defaults to 1000000.\n";
    std::cout << "   -s seed        Seed to use for random number generation. Defaults to 1337.\n";
    std::cout << "   -a alphabet    Characters to run rank on. defaults to \"ACGT\".\n\n";
    std::cout << "Input file is required.\n"
              << "Will run num rank(c, i) querises where i and c are are picked at random (almost uniformly).\n"
              << "Statistics will be output to std::cerr and rank query results sto std::cout.\n\n";
    std::cout << "Example: bench_bwt bwt.bin -n 100000 -a ACGTN >> /dev/null" << std::endl;
    exit(0);
}

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        std::cerr << "Input file is required\n" << std::endl;
        help();
    }
    uint64_t seed = 1337;
    std::string alphabet = "ACGT";
    uint64_t n = 1000000;
    std::string in_file_path = "";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            std::sscanf(argv[++i], "%lu", &n);
        } else if (strcmp(argv[i], "-s") == 0) {
            std::sscanf(argv[++i], "%lu", &seed);
        } else if (strcmp(argv[i], "-a") == 0) {
            alphabet = argv[++i];
        } else {
            in_file_path = argv[i];
        }
    }
    if (in_file_path.size() == 0) {
        std::cerr << "Input file is required\n" << std::endl;
        help();
    }
    bbwt::custom_rlbwt<BLOCK_SIZE> bwt(in_file_path);
    std::cerr << "Testing " << in_file_path << "\n"
              << "with " << n << " queries in [0.." << bwt.size() << "]\n"
              << "from alphabet " << alphabet << "\n"
              << "using seed " << seed << std::endl;

    std::mt19937 mt(seed);
    std::uniform_int_distribution<unsigned long long> gen(
        std::numeric_limits<std::uint64_t>::min(),
        std::numeric_limits<std::uint64_t>::max());

    std::vector<std::pair<char, uint64_t>> queries;

    for (uint64_t i = 0; i < n; i++) {
        queries.push_back({alphabet[gen(mt) % alphabet.size()], gen(mt) % bwt.size()});
    }

    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::microseconds;
    std::cout << "c\ti\trank(c, i)\ttime(us)" << std::endl;
    double micros = 0;
    for (auto q : queries) {
        auto start = high_resolution_clock::now();
        uint64_t r = bwt.rank(q.first, q.second);
        auto end = high_resolution_clock::now();
        double time = duration_cast<microseconds>(end - start).count();
        std::cout << q.first << "\t" << q.second << "\t" << r << "\t" << time << std::endl;
        micros += time;
    }
    std::cerr << "Mean query time: " << micros / n << "us" << std::endl;
}