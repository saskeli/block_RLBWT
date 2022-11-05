#pragma once

#include "block_rlbwt.hpp"
#include "block_rlbwt_builder.hpp"
#include "byte_alphabet.hpp"
#include "byte_block.hpp"
#include "custom_alphabet.hpp"
#include "d_block.hpp"
#include "delta_alphabet.hpp"
#include "one_byte_block.hpp"
#include "super_block.hpp"
#include "two_byte_block.hpp"
#include "genomics_alphabet.hpp"
#include "acgtn_alphabet.hpp"

namespace bbwt {

template <uint32_t block_size = 32768>
using byte_rlbwt =
    block_rlbwt<super_block<byte_block<block_size, byte_alphabet<u_int32_t>>>,
                byte_alphabet<uint64_t>>;

template <uint32_t block_size = 4096>
using genomics_rlbwt = block_rlbwt<
    super_block<two_byte_block<block_size, genomics_alphabet<uint32_t>>>,
    genomics_alphabet<uint64_t>>;

template <uint32_t block_size = 4096>
using acgt_rlbwt = block_rlbwt<
    super_block<two_byte_block<block_size, acgt_alphabet<uint32_t>>>,
    acgt_alphabet<uint64_t>>;

using rlbwt = block_rlbwt<
    super_block<two_byte_block<4096, delta_alphabet<uint32_t, 10, 89>>>,
    delta_alphabet<uint64_t, 10, 89>>;

using custom_rlbwt = block_rlbwt<
    super_block<two_byte_block<4096, custom_alphabet<uint32_t>>>,
    custom_alphabet<uint64_t>>;

//using custom_rlbwt = block_rlbwt<
//    super_block<byte_block<131072, genomics_alphabet<uint32_t>>>,
//    genomics_alphabet<uint64_t>>;

template <uint32_t block_size = 4096>
using dyn_rlbwt = block_rlbwt<
    super_block<
        d_block<two_byte_block<block_size, custom_alphabet<uint32_t>>,
                one_byte_block<block_size, custom_alphabet<uint32_t>, true>>>,
    custom_alphabet<uint64_t>>;

}  // namespace bbwt
