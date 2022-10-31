#include <iostream>
#include <vector>
#include <random>
#include <chrono>

#include "include/reader.hpp"
#include "include/types.hpp"

void help() {
    std::cout << "Benchmark RLBWT data structure.\n\n";
    std::cout << "Usage: bench_bwt file_name_a file_name_b [-n num] [-a alphabet]\n";
    std::cout << "   file_name_a    Path to first block rlbwt root element.\n";
    std::cout << "   file_name_b    Path to second block rlbwt root element.\n";
    std::cout << "   -n num         Number of tests to run. Defaults to 1000000.\n";
    std::cout << "   -s seed        Seed to use for random number generation. Defaults to 1337.\n";
    std::cout << "   -a alphabet    Characters to run rank on. defaults to \"ACGT\".\n\n";
    std::cout << "Input file is required.\n"
              << "Will run num rank(c, i) and access(i) querises where i and c are are picked at random (almost uniformly).\n"
              << "Statistics will be output to std::cerr and rank query results sto std::cout.\n\n";
    std::cout << "Example: bench_bwt bwt.bin -n 100000 -a ACGTN >> /dev/null" << std::endl;
    exit(0);
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        std::cerr << "Input files are required\n" << std::endl;
        help();
    }
    uint64_t seed = 1337;
    std::string alphabet = "ACGT";
    uint64_t n = 1000000;
    std::string in_file_path_a = "";
    std::string in_file_path_b = "";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            std::sscanf(argv[++i], "%lu", &n);
        } else if (strcmp(argv[i], "-s") == 0) {
            std::sscanf(argv[++i], "%lu", &seed);
        } else if (strcmp(argv[i], "-a") == 0) {
            alphabet = argv[++i];
        } else if (in_file_path_a.size() == 0) {
            in_file_path_a = argv[i];
        } else {
            in_file_path_b = argv[i];
        }
    }
    if (in_file_path_a.size() == 0 || in_file_path_b.size() == 0) {
        std::cerr << "Input files are required\n" << std::endl;
        help();
    }
    bbwt::rlbwt bwt_a(in_file_path_a);
    bbwt::custom_rlbwt bwt_b(in_file_path_b);
    if (bwt_a.size() != bwt_b.size()) {
        std::cerr << "Indexes must have the same size!" << std::endl;
        exit(1);
    }
    std::cerr << "Testing " << in_file_path_a << " vs. " << in_file_path_b << "\n"
              << "with " << n << " queries in [0.." << bwt_a.size() << "]\n"
              << "from alphabet " << alphabet << "\n"
              << "using seed " << seed << std::endl;

    std::mt19937 mt(seed);
    std::uniform_int_distribution<unsigned long long> gen(
        std::numeric_limits<std::uint64_t>::min(),
        std::numeric_limits<std::uint64_t>::max());

    std::vector<std::pair<char, uint64_t>> queries;

    for (uint64_t i = 0; i < n; i++) {
        queries.push_back({alphabet[gen(mt) % alphabet.size()], gen(mt) % bwt_a.size()});
    }

    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;
    std::cout << "c\ti\trank_a\taccess_a\trank_b\taccess_b\tr_ok\ta_ok" << std::endl;
    double r_nanos_a = 0;
    double a_nanos_a = 0;
    double r_nanos_b = 0;
    double a_nanos_b = 0;
    uint64_t r_fails = 0;
    uint64_t a_fails = 0;
    for (auto q : queries) {
        std::cout << q.first << "\t" << q.second << "\t";
        auto start = high_resolution_clock::now();
        uint64_t r_a = bwt_a.rank(q.first, q.second);
        auto end = high_resolution_clock::now();
        double time = duration_cast<nanoseconds>(end - start).count();
        std::cout << time << "\t";
        r_nanos_a += time;

        start = high_resolution_clock::now();
        uint64_t a_a = bwt_a.at(q.second);
        end = high_resolution_clock::now();
        time = duration_cast<nanoseconds>(end - start).count();
        std::cout << time << "\t";
        a_nanos_a += time;

        start = high_resolution_clock::now();
        uint64_t r_b = bwt_b.rank(q.first, q.second);
        end = high_resolution_clock::now();
        time = duration_cast<nanoseconds>(end - start).count();
        std::cout << time << "\t" << std::endl;
        r_nanos_b += time;

        start = high_resolution_clock::now();
        uint64_t a_b = bwt_a.at(q.second);
        end = high_resolution_clock::now();
        time = duration_cast<nanoseconds>(end - start).count();
        std::cout << time << "\t";
        a_nanos_b += time;

        if (r_a != r_b) {
            r_fails++;
            std::cout << "n\t";
        } else {
            std::cout << "y\t";
        }
        if (a_a != a_b) {
            a_fails++;
            std::cout << "n" << std::endl;
        } else {
            std::cout << "y" << std::endl;
        }
    }
    if (r_fails || a_fails) {
        std::cerr << r_fails << " / " << n << " = " << double(r_fails) / n << " rank fails" << std::endl;
        std::cerr << a_fails << " / " << n << " = " << double(a_fails) / n << " access fails" << std::endl;
    }
    std::cerr << "     \trank\tacces\tbps\n" 
              << "bwt_a\t" << r_nanos_a / n << "\t" << a_nanos_a / n << "\t" << double(bwt_a.bytes() * 8) / bwt_a.size() << "\n"
              << "bwt_b\t" << r_nanos_b / n << "\t" << a_nanos_b / n << "\t" << double(bwt_b.bytes() * 8) / bwt_b.size() << std::endl;
}