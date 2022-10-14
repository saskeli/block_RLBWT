#pragma once

#include "block_rlbwt.hpp"
#include "super_block.hpp"
#include "delta_alphabet.hpp"
#include "two_byte_block.hpp"
#include "block_rlbwt_builder.hpp"

namespace bbwt {

template <uint32_t block_size>
using rlbwt = block_rlbwt<super_block<two_byte_block<block_size, delta_alphabet<uint32_t, '\n', '_'>>>, delta_alphabet<uint64_t, '\n', '_'>>;

} // namespace bbwt
