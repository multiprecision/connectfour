#pragma once

#include "global.h"

#include <array>
#include <stack>
#include <vector>
#include <utility>
#include <regex>

namespace con4game
{
/**
 * This class defines the board object for the Connect Four game.
 * @author Samuel I. Gunadi
 */
class Board
{
public:
	/** Default constructor. */
	Board();
	/**
	 * Get current board state.
	 * @return pointer to the board container.
	 */
	const uint64_t * const get_board() const;
	
	/**
	 * Look up at specified row and column.
	 * @param row  the row
	 * @param col  the column
	 * @return Return 0 if empty empty,
	 *                1 if occupied by player 1 counter.
	 *                2 if occupied by player 2 counter.
	 */
	int at(int row, int col) const;

	/**
	 * Get positions of the counters which lead to that player winning the game.
	 * @return the positions as pairs (row, column).
	 */
	std::vector<std::pair<int, int>> get_markers() const;

	/**
	 * Drop a counter in the specified column.
	 * @param col     the column
	 */
	void place(int col);

	/**
	 * Reset the board to empty state.
	 */
	void reset();

	/**
	* Check whether a player has won the game.
	* @return non-zero if has won
	*/
	uint64_t has_won(uint64_t newboard);

	/**
	 * Check whether a player has won the game.
	 * @return 0 if the condition for winning the game have not been met, 
	 *         1 if player 1 win,
	 *         2 if player 2 win,
	 *         3 if draw.
	 */
	int test_win();

	/**
	* Check if a column has room, i.e., a counter can be dropped to the specified column.
	* @param col   the column number
	* @return true if a counter can be dropped to the specified column.
	*/
	bool is_playable(int col) const;

	/**
	* Check if newboard lacks overflowing column
	* @param newboard   the bitboard
	* @return true if newboard has overflowing column
	*/
	bool is_legal(uint64_t newboard) const;

	/**
	 * Undo last move.
	 * @return true if successful
	 */
	bool undo_last_move();

	/*
	 * Find the best move for the specified player.
	 * @param player  the player
	 * @return the best column for that player
	 */
	int find_best_move(int player);

protected:
	/**
	 * The recursive Negamax algorithm.
	 * @return the best column and its score.
	 */
	std::pair<int, int> negamax_alpha_beta_pruning(int depth, int alpha, int beta, int player, int sign);

	/** The evaluation function. */
	int evaluate(int player);

	/** Helper function for the evaluation function. */
	int evaluate(int row, int column, int player);

private:
	/**
	 * The container for storing each player counters.
	 * 
	 * Connect Four is played on a vertical board with seven columns and six rows.
	 * That makes 42 slots. The board with these 42 slots is shown in the diagram below.
	 * We add an additional row on top for convenience purposes.
	 * This additional row on top is for computational reasons only.
	 * And so are the bits numbered 49 to 63, adding two more columns and a bit.
	 * The bits of the top row (6, 13, 20, etc.) and the bits on the right (49 - 63)
	 * are seemingly unused but nonetheless important and not to forget in their
	 * role when it comes to manipulating the bits.
	 *
	 * The numbers indicate the position in the bit representation of a long integer.
	 *
	 *   6 13 20 27 34 41 48   55 62     Additional row 
	 * +---------------------+ 
	 * | 5 12 19 26 33 40 47 | 54 61     top row
	 * | 4 11 18 25 32 39 46 | 53 60
	 * | 3 10 17 24 31 38 45 | 52 59
	 * | 2  9 16 23 30 37 44 | 51 58
	 * | 1  8 15 22 29 36 43 | 50 57
	 * | 0  7 14 21 28 35 42 | 49 56 63  bottom row
	 * +---------------------+
	 */
	uint64_t bitboard[2];

	/**
	 * A container that records history of each player's moves.
	 */
	std::array<int, SIZE> moves;

	/**
	* The number of moves done (number of plies).
	*/
	int plies_num;

	/**
	 * A container that stores the height of each column,
	 * holds bit index of lowest free square.
	 */
	std::array<int, BOARD_WIDTH> height;

	/**
	 * How many board states that have been evaluated.
	 */
	int iterations;
};

} // namespace con4game
