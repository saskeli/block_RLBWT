#pragma once

#include "block_rlbwt.hpp"
#include "block_rlbwt_builder.hpp"
#include "custom_alphabet.hpp"
#include "d_block.hpp"
#include "delta_alphabet.hpp"
#include "one_byte_block.hpp"
#include "super_block.hpp"
#include "two_byte_block.hpp"
#include "byte_block.hpp"

namespace bbwt {

using rlbwt = block_rlbwt<
    super_block<two_byte_block<4096, delta_alphabet<uint32_t, 10, 89>>>,
    delta_alphabet<uint64_t, 10, 89>>;

using custom_rlbwt = block_rlbwt<
    super_block<byte_block<4096, custom_alphabet<uint32_t>>>,
    custom_alphabet<uint64_t>>;

template <uint32_t block_size = 4096>
using dyn_rlbwt = block_rlbwt<
    super_block<
        d_block<two_byte_block<block_size, custom_alphabet<uint32_t>>,
                one_byte_block<block_size, custom_alphabet<uint32_t>, true>>>,
    custom_alphabet<uint64_t>>;

}  // namespace bbwt
