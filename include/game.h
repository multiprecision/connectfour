#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "board.h"

namespace con4game
{

/**
 * This class implements the GUI portion of the program
 * which includes rendering routine, user input handler, and the main loop.
 * @author Samuel I. Gunadi
 */
class Game
{
public:
	/**
	 * Default constructor.
	 */
	Game();
	/** Non-copyable. */
	Game(const Game&) = delete;
	/**
	 * Main game loop.
	 */
	void run();
	/**
	 * Render a frame.
	 */
	void render();
	/**
	 * Process an event.
	 */
	void process_event(const sf::Event& event);

private:
	/**
	 * Winning markers (the white circles).
	 */
	std::vector<std::pair<int, int>> markers;
	/**
	 * The rendering window.
	 */
	sf::RenderWindow window;
	/**
	 * A camera with right-handed coordinates (x = right, y = up).
	 */
	sf::View view;
	/**
	 * Font object for the Inconsolata font.
	 */
	sf::Font font;
	/**
	 * The text in the bottom of the screen.
	 */
	sf::Text text;
	/**
	 * Enumeration for the game state.
	 * INIT means game is initialized.
	 * START means game has started.
	 * WORKING means game is finding the best move.
	 * END means game is over.
	 */
	enum class Game_state { INIT, START, WORKING, END };

	/**
	 * State of the game.
	 */
	Game_state state;
	/**
	* The board object.
	*/
	Board board;
	/**
	 * 1 if it's player 1 turn.
	 * 2 if it's player 2 turn.
	 */
	int turn;
	/**
	 * True if the board is full and neither players have won nor lost.
	 */
	bool draw;
};

} // namespace con4game
