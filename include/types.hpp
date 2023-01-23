#pragma once

#include "block_rlbwt.hpp"
//#include "byte_alphabet.hpp"
#include "byte_block.hpp"
#include "custom_alphabet.hpp"
#include "d_block.hpp"
//#include "delta_alphabet.hpp"
#include "one_byte_block.hpp"
#include "super_block.hpp"
#include "two_byte_block.hpp"
//#include "genomics_alphabet.hpp"
//#include "acgtn_alphabet.hpp"
#include "alphabet.hpp"
#include "vbyte_runs.hpp"
#include "run_rlbwt.hpp"

#ifndef RUN_COUNT
#define RUN_COUNT 32
#endif

#ifndef LARGE_BLOCK_SIZE
#define LARGE_BLOCK_SIZE 16384
#endif

#ifndef SMALL_BLOCK_SIZE
#define SMALL_BLOCK_SIZE 4096
#endif

namespace bbwt {

template <uint32_t block_size = SMALL_BLOCK_SIZE>
using two_byte_build = block_rlbwt<
    super_block<two_byte_block<block_size, custom_alphabet<uint32_t>>>,
    custom_alphabet<uint64_t>>;

template <uint32_t block_size = SMALL_BLOCK_SIZE>
using two_byte = block_rlbwt<
    super_block<two_byte_block<block_size, alphabet<uint32_t>>>,
    alphabet<uint64_t>>;

template <uint32_t block_size = LARGE_BLOCK_SIZE>
using vbyte_build = block_rlbwt<
    super_block<byte_block<block_size, custom_alphabet<uint32_t>>>,
    custom_alphabet<uint64_t>>;

template <uint32_t block_size = LARGE_BLOCK_SIZE>
using vbyte = block_rlbwt<
    super_block<byte_block<block_size, alphabet<uint32_t>>>,
    alphabet<uint64_t>>;

template <uint32_t block_size = LARGE_BLOCK_SIZE>
using dyn_build = block_rlbwt<
    super_block<
        d_block<two_byte_block<block_size, custom_alphabet<uint32_t>>,
                one_byte_block<block_size, custom_alphabet<uint32_t>, true>>>,
    custom_alphabet<uint64_t>>;

template <uint32_t block_size = LARGE_BLOCK_SIZE>
using dyn = block_rlbwt<
    super_block<
        d_block<two_byte_block<block_size, alphabet<uint32_t>>,
                one_byte_block<block_size, alphabet<uint32_t>, true>>>,
    alphabet<uint64_t>>;

template <uint32_t n_runs = RUN_COUNT>
using run_build = run_rlbwt<vbyte_runs<n_runs, custom_alphabet<uint64_t>>>;

template <uint32_t n_runs = RUN_COUNT>
using run = run_rlbwt<vbyte_runs<n_runs, alphabet<uint64_t>>, 0>;

}  // namespace bbwt
