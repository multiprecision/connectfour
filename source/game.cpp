#include "global.h"
#include "game.h"
#include "board.h"
#include "asset.h"

#include <SFML/Graphics.hpp>

#include <numeric>
#include <thread>
#include <sstream>
#include <iomanip>

namespace con4game
{

Game::Game()
: window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Connect Four Game", sf::Style::Close, sf::ContextSettings(0, 0, 16))
, view()
, font()
, text()
, state(Game_state::INIT)
, turn(1)
, draw(false)
{
	window.setVerticalSyncEnabled(true);
	// right-handed coordinates (x = right, y = up).
	view = window.getDefaultView();
	view.setSize((float) WINDOW_WIDTH, -1.f * WINDOW_HEIGHT);
	// font is in asset.h
	font.loadFromMemory(inconsolata_font, 58464);
	text.setFont(font);
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(TEXT_SIZE);
	text.setPosition(0, WINDOW_HEIGHT - TEXT_SIZE * 3); // 2 lines of text ++ offset
	state = Game_state::START;
}

void Game::run()
{
	const long long update_interval_in_ns = 1000000000;  // update fps counter after 1 second
	double average_ns_per_frame;
	std::chrono::time_point<std::chrono::steady_clock> start_clock;
	std::chrono::time_point<std::chrono::steady_clock> end_clock;
	std::chrono::duration<long long, std::nano> clock_diff;
	std::deque<long long> samples;
	std::chrono::time_point<std::chrono::steady_clock> next_update = std::chrono::steady_clock::now() + std::chrono::duration<long long, std::nano>(update_interval_in_ns);
	while (window.isOpen())
	{
		start_clock = std::chrono::steady_clock::now();
		// Process events.
		sf::Event event;
		while (window.pollEvent(event))
		{
			process_event(event);
		}
		// Then render.
		render();
		end_clock = std::chrono::steady_clock::now();
		clock_diff = end_clock - start_clock;
		samples.push_back(clock_diff.count());
		if (end_clock >= next_update)
		{
			if (!samples.empty())
			{
				average_ns_per_frame = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
				std::stringstream ss;
				ss.precision(3);
				ss.setf(std::ios_base::fixed, std::ios_base::floatfield);
				ss << "Connect Four Game " << 1. / (1e-9 * average_ns_per_frame) << " FPS " << average_ns_per_frame * 1e-6 << " ms";
				window.setTitle(ss.str());
				next_update = std::chrono::steady_clock::now() + std::chrono::duration<long long, std::nano>(update_interval_in_ns);
				samples.clear();
			}
		}
	}
}

void Game::render()
{
	// clear the window with yellow color
	window.clear(sf::Color::Yellow);
	// draw board
	for (int row = 0; row < BOARD_HEIGHT; row++)
	{
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			sf::CircleShape stone((float) STONE_SIZE / 2);
			stone.setScale(0.9f, 0.9f);
			stone.setPosition((float) col * STONE_SIZE, (float) row * STONE_SIZE + TEXT_SIZE * 3); // 2 lines of text + offset
			stone.move(sf::Vector2f((float) STONE_SIZE * 0.05f, (float) STONE_SIZE * 0.05f));
			switch (board.at(row, col))
			{
			case 0:
				stone.setFillColor(sf::Color::White); // white
				break;
			case 1:
				stone.setFillColor(sf::Color::Red); // player 1
				break;
			case 2:
				stone.setFillColor(sf::Color::Black); // player 2
				break;
			}
			window.setView(view);
			window.draw(stone);
		}
	}
	// draw marker
	if (state == Game_state::END)
	{
		for (auto it : markers)
		{

			sf::CircleShape marker((float)STONE_SIZE / 2);
			marker.setScale(0.5f, 0.5f);
			marker.setPosition((float)it.first * STONE_SIZE, (float)it.second * STONE_SIZE + TEXT_SIZE * 3); // 2 lines of text + offset
			marker.move(sf::Vector2f((float)STONE_SIZE * 0.25f, (float)STONE_SIZE * 0.25f));
			marker.setFillColor(sf::Color::White);
			window.setView(view);
			window.draw(marker);
		}
	}
	// draw text
	if (state == Game_state::END)
	{
		if (draw)
		{
			text.setString("Draw.\r\nPress X to restart.");
		}
		else
		{
			text.setString("Player " + std::to_string(turn) + " won.\r\nPress Z to undo, X to restart.");
		}
	}
	else if (state == Game_state::START)
	{
		text.setString("Player " + std::to_string(turn) + " turn.\r\nPress Z to undo, X to solve.");
	}
	else if (state == Game_state::WORKING)
	{
		text.setString("Finding the best move for player " + std::to_string(turn) + "...");
	}
	window.setView(window.getDefaultView());
	window.draw(text);

	window.display();
}

void Game::process_event(const sf::Event & event)
{
	if (event.type == sf::Event::Closed)
	{
		window.close();
		return;
	}
	else if(event.type == sf::Event::KeyPressed)
	{
		if (state == Game_state::WORKING)
		{
			return;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		{
			window.close();
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
		{
			if (state != Game_state::END)
			{
				if (board.undo_last_move())
				{
					turn = 3 - turn;
				}
			}
			else if (state == Game_state::END)
			{
				if (board.undo_last_move())
				{
					draw = false;
					state = Game_state::START;
				}
			}
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
			if (state != Game_state::END)
			{
				state = Game_state::WORKING;
				std::thread(
					[this]
					{
						board.place(board.find_best_move(turn));
						int test = board.test_win();
						if (test != 0)
						{
							markers = board.get_markers();
							state = Game_state::END;
							if (test == 3)
							{
								draw = true;
							}
							return;
						}
						turn = 3 - turn;
						state = Game_state::START;
					}
				).detach();
			}
			else if (state == Game_state::END)
			{
				board.reset();
				turn = 1;
				draw = false;
				state = Game_state::START;
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonPressed)
	{
		if (state == Game_state::WORKING)
		{
			return;
		}
		if (state != Game_state::END)
		{
			int selected_column = event.mouseButton.x / STONE_SIZE;
			if (!board.is_playable(selected_column))
			{
				return;
			}
			board.place(selected_column);
			int test = board.test_win();
			if (test != 0)
			{
				markers = board.get_markers();
				state = Game_state::END;
				if (test == 3)
				{
					draw = true;
				}
				return;
			}
			turn = 3 - turn;
		}
	}
}

}  // namespace con4game

