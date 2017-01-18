#include "board.h"
#include "game.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace con4game
{

Board::Board()
{
	reset();
}

const uint64_t * const Board::get_board() const
{
	return bitboard;
}
int Board::at(int row, int col) const
{
	// .  .  .  .  .  .  .  TOP
	// 5 12 19 26 33 40 47
	// 4 11 18 25 32 39 46
	// 3 10 17 24 31 38 45
	// 2  9 16 23 30 37 44
	// 1  8 15 22 29 36 43
	// 0  7 14 21 28 35 42  BOTTOM
	int bit_location = row + col * H1;
	if (((bitboard[0] >> bit_location) & 1) == 1)
	{
		return 1;
	}
	else if (((bitboard[1] >> bit_location) & 1) == 1)
	{
		return 2;
	}
	return 0;
}

void Board::place(int col)
{
	if (is_playable(col))
	{
		bitboard[plies_num & 1] ^= 1ULL << height[col]++;
		moves[plies_num++] = col;
	}
}

void Board::reset()
{
	plies_num = 0;
	bitboard[0] = bitboard[1] = 0;
	for (int i = 0; i < BOARD_WIDTH; i++)
		height[i] = H1 * i;
}


uint64_t Board::has_won(uint64_t bitboard)
{
	uint64_t diag1 = bitboard & (bitboard >> BOARD_HEIGHT);
	uint64_t hori = bitboard & (bitboard >> H1);
	uint64_t diag2 = bitboard & (bitboard >> H2);
	uint64_t vert = bitboard & (bitboard >> 1);
	return ((diag1 & (diag1 >> 2 * BOARD_HEIGHT)) |
		(hori & (hori >> 2 * H1)) |
		(diag2 & (diag2 >> 2 * H2)) |
		(vert & (vert >> 2)));
}


int Board::test_win()
{
	if (has_won(bitboard[0]))
	{
		return 1;
	}
	else if (has_won(bitboard[1]))
	{
		return 2;
	}
	else if (plies_num >= SIZE)
	{
		return 3;
	}
	return 0;
}



bool Board::is_playable(int col) const
{
	return is_legal(bitboard[plies_num & 1] | (1ULL << height[col]));
}

bool Board::is_legal(uint64_t newboard) const
{
	return (newboard & TOP) == 0;
}

bool Board::undo_last_move()
{
	if (plies_num == 0)
	{
		return false;
	}
	int col = moves[--plies_num];
	bitboard[plies_num & 1] ^= 1ULL << --height[col];
	return true;
}

int Board::find_best_move(int player)
{
	iterations = 0;
	int opponent = 3 - player;
	// measure time
	std::chrono::time_point<std::chrono::steady_clock> start_clock = std::chrono::steady_clock::now();

	// Rule #1. If player can win in 1 turn, do it.
	for (int col_index = 0; col_index < BOARD_WIDTH; col_index++)
	{
		if (!is_playable(col_index))
		{
			continue;
		}
		uint64_t player_board = bitboard[player - 1] ^ (1ULL << height[col_index]);
		if (has_won(player_board))
		{
			std::cout << std::endl << "[DEBUG] Player can win in 1 turn. Take it." << std::endl << "[DEBUG] column: " << col_index << std::endl;
			return col_index;
		}
	}

	// Rule #2. If opponent player can win in 1 turn, prevent it.
	for (int col_index = 0; col_index < BOARD_WIDTH; col_index++)
	{
		if (!is_playable(col_index))
		{
			continue;
		}
		uint64_t opponent_board = bitboard[opponent - 1] ^ (1ULL << height[col_index]);
		if (has_won(opponent_board))
		{
			std::cout << std::endl << "[DEBUG] Opponent player can win in 1 turn. Prevent it." << std::endl << "[DEBUG] column: " << col_index << std::endl;
			return col_index;
		}

	}

	std::cout << std::endl << "[DEBUG] Finding the best move using Negamax algorithm..." << std::endl;

	std::pair<int, int> result = negamax_alpha_beta_pruning(MAX_SEARCH_DEPTH, INT_MIN, INT_MAX, player, 1);

	std::cout << std::endl << "[DEBUG] Finished finding best move." << std::endl << "[DEBUG] iterations: " << iterations << std::endl << "[DEBUG] column: " << result.first << std::endl << "[DEBUG] score: " << result.second  << std::endl;

	std::chrono::time_point<std::chrono::steady_clock> end_clock = std::chrono::steady_clock::now();
	std::chrono::duration<long long, std::nano> clock_diff = end_clock - start_clock;

	std::cout << "[DEBUG] time taken: " << (1e-9 * clock_diff.count()) << " s" << std::endl;
	return result.first;
}

std::pair<int, int> Board::negamax_alpha_beta_pruning(int depth, int alpha, int beta, int player, int sign)
{
	// stop if maximum search depth has been reached, or if the game is over
	if (depth <= 0 || test_win())
	{
		int score = evaluate(player);
		return std::pair<int, int>(-1, sign * score);
	}

	iterations++;

	int best_column = -1;
	int best_value = INT_MIN;
	for (int col_index = 0; col_index < BOARD_WIDTH; col_index++)
	{
		// full
		if (!is_playable(col_index))
		{
			continue;
		}
		place(col_index);
		int value = -negamax_alpha_beta_pruning(depth - 1, -beta, -alpha, player, -sign).second;

		undo_last_move();

		if (value > best_value)
		{
			best_value = value;
			best_column = col_index;
		}

		alpha = std::max(value, alpha);

		// beta cut-off
		if (alpha >= beta)
		{
			break;
		}

	}
	return std::pair<int, int>(best_column, best_value);
}

int Board::evaluate(int row, int column, int player)
{
	int score = 0;
	bool unblocked = true;
	int tally = 0;
	int opponent = 3 - player;
	if (row < BOARD_HEIGHT - 3)
	{
		//check up
		unblocked = true;
		tally = 0;

		for (int row_index = row; row_index < row + 4; row_index++)
		{
			if (at(row_index, column) == opponent)
			{
				unblocked = false;
			}
			if (at(row_index, column) == player)
			{
				tally++;
			}
		}
		if (unblocked == true)
		{
			score = score + (tally * tally * tally * tally);
		}
		if (column < BOARD_WIDTH - 3)
		{
			// check up and to the right
			unblocked = true;
			tally = 0;
			for (int row_index = row, column_index = column; row_index < row + 4; row_index++, column_index++)
			{
				if (at(row_index, column_index) == opponent)
				{
					unblocked = false;
				}
				if (at(row_index, column_index) == player)
				{
					tally++;
				}
			}
			if (unblocked == true)
			{
				score = score + (tally * tally * tally * tally);
			}
		}
	}
	if (column < BOARD_WIDTH - 3)
	{
		// check right
		unblocked = true;
		tally = 0;
		for (int column_index = column; column_index < column + 4; column_index++)
		{
			if (at(row, column_index) == opponent)
			{
				unblocked = false;
			}
			if (at(row, column_index) == player)
			{
				tally++;
			}
		}
		if (unblocked == true)
		{
			score = score + (tally * tally * tally * tally);
		}
		if (row > 2)
		{
			// check down and to the right
			unblocked = true;
			tally = 0;
			for (int row_index = row, column_index = column; column_index < column + 4; row_index--, column_index++)
			{
				if (at(row_index, column_index) == opponent)
				{
					unblocked = false;
				}
				if (at(row_index, column_index) == player)
				{
					tally++;
				}
			}
			if (unblocked == true)
			{
				score = score + (tally * tally * tally * tally);
			}
		}
	}
	return score;
}

int Board::evaluate(int player)
{
	int score = 0;
	int opponent = 3 - player;
	for (int row_index = 0; row_index < BOARD_HEIGHT; row_index++)
	{
		if (row_index <= BOARD_HEIGHT - 4)
		{
			for (int column_index = 0; column_index < BOARD_WIDTH; column_index++)
			{
				score += evaluate(row_index, column_index, player);
			}
		}
		else
		{
			for (int column_index = 0; column_index <= BOARD_WIDTH - 4; column_index++)
			{
				score += evaluate(row_index, column_index, player);
			}
		}
	}
	return score;
}


std::vector<std::pair<int, int>> Board::get_markers() const
{
	// .  .  .  .  .  .  .  TOP
	// 5 12 19 26 33 40 47
	// 4 11 18 25 32 39 46
	// 3 10 17 24 31 38 45
	// 2  9 16 23 30 37 44
	// 1  8 15 22 29 36 43
	// 0  7 14 21 28 35 42  BOTTOM
	std::vector<std::pair<int, int>> markers;
	int x = moves[plies_num - 1];
	int y = height[x] - H1 * x - 1;
	// vertical
	if (y >= 3 && at(y - 3, x) == at(y, x) && at(y - 2, x) == at(y, x) && at(y - 1, x) == at(y, x))
	{
		for (int i = 0; i < 4; i++)
		{
			markers.push_back(std::pair<int, int>(x, y + i * -1));
		}
	}
	// horizontal and diagonal
	for (int dy = -1; dy <= 1; dy++)
	{
		int dx = 0;
		int nb = 0;
		for (dx = 1; x + dx < BOARD_WIDTH && y + dx * dy < BOARD_HEIGHT && y + dx * dy >= 0; dx++)
		{
			if (at(y + dx * dy, x + dx) == at(y, x))
			{
				nb++;
			}
			else
			{
				break;
			}
		}
		for (dx = -1; x + dx >= 0 && y + dx * dy < BOARD_HEIGHT && y + dx * dy >= 0; dx--)
		{
			if (at(y + dx * dy, x + dx) == at(y, x))
			{
				nb++;
			}
			else
			{
				break;
			}
		}
		if (nb >= 3)
		{
			for (int i = 0; i < 4; i++)
			{
				markers.push_back(std::pair<int, int>(x + dx + 1 + i, (y + (dx + 1) * dy) + i * dy));
			}
		}
	}
	return markers;
}

} // namespace con4game
