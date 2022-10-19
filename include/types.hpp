#pragma once

#include "block_rlbwt.hpp"
#include "super_block.hpp"
#include "delta_alphabet.hpp"
#include "two_byte_block.hpp"
#include "block_rlbwt_builder.hpp"
#include "custom_alphabet.hpp"

namespace bbwt {

template <uint32_t block_size>
using rlbwt = block_rlbwt<super_block<two_byte_block<block_size, delta_alphabet<uint32_t, '\n', '_'>>>, delta_alphabet<uint64_t, '\n', '_'>>;

template <uint32_t block_size>
using custom_rlbwt = block_rlbwt<super_block<two_byte_block<block_size, custom_alphabet<uint32_t>>>, custom_alphabet<uint64_t>>;

} // namespace bbwt
