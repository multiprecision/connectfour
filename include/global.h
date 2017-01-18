/**
 * Defines global constants.
 * @author Samuel I. Gunadi
 */

#pragma once

#include <cstdint>
#include <random>

namespace con4game
{
	const uint64_t BOARD_WIDTH = 7;
	const uint64_t BOARD_HEIGHT = 6;
	const uint64_t H1 = BOARD_HEIGHT + 1;
	const uint64_t H2 = BOARD_HEIGHT + 2;
	const uint64_t COL1 = (1ULL << H1) - 1ULL;
	const uint64_t SIZE = BOARD_HEIGHT * BOARD_WIDTH;
	const uint64_t SIZE1 = H1 * BOARD_WIDTH;
	const uint64_t ALL1 = (1ULL << SIZE1) - 1ULL;
	const uint64_t BOTTOM = ALL1 / COL1; // has bits i*H1 set
	const uint64_t TOP = BOTTOM << BOARD_HEIGHT;

	const int TEXT_SIZE = 16;
	const int STONE_SIZE = 64;
	const int WINDOW_WIDTH = STONE_SIZE * BOARD_WIDTH;
	const int WINDOW_HEIGHT = STONE_SIZE * BOARD_HEIGHT + TEXT_SIZE * 3;

	const int MAX_SEARCH_DEPTH = 8;

} // namespace con4game
