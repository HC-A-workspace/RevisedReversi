#include <iostream>

#include "AI.hpp"
#include "MemorizedAlphaBetaAI.hpp"
#include "NegaAlphaAI.hpp"
#include "AlphaBetaAI.hpp"
#include "Game.hpp"

int main()
{
	BitBoard white = from_cells({ Cell("d4"), Cell("e5")});
	BitBoard black = from_cells({ Cell("d5"), Cell("e4") });

	Board board(black, white);

	Game game(black, white);

	std::string black_y_or_n;
	while (black_y_or_n != "y" && black_y_or_n != "n") {
		std::cout << "Do you use AI for black player? (y / n) : ";
		std::cin >> black_y_or_n;
	}

	std::cout << std::endl;

	std::string white_y_or_n;
	while (white_y_or_n != "y" && white_y_or_n != "n") {
		std::cout << "Do you use AI for white player? (y / n) : ";
		std::cin >> white_y_or_n;
	}

	std::cout << std::endl;

	if (black_y_or_n == "y") {
		game.set_black_AI(std::make_unique<AlphaBetaAI>(8));
	}
	if (white_y_or_n == "y") {
		game.set_white_AI(std::make_unique<RandomAI>());
	}

	auto human_play = [](const Board& board) {
		std::string move;
		std::cout << "Your move: ";
		std::cin >> move;
		bool is_valid = board.is_valid_move(from_cell(Cell(move)));
		while (!is_valid) {
			std::cout << "Invalid input. Try again." << std::endl;
			std::cout << "Your move: ";
			std::cin >> move;
			if (!Cell::is_valid(move)) {
				is_valid = false;
			}
			else {
				is_valid = board.is_valid_move(from_cell(Cell(move)));
			}
		}
		return Cell(move);
	};

	game.set_human_play(human_play);
	
	while(!game.is_game_over()) {
		std::cout << game.to_string() << std::endl;
		std::cout << "white: " << game.white_count() << ", black: " << game.black_count() << std::endl;
		std::cout << std::endl;
		std::cout << (game.get_current_player() == BLACK ? "Black" : "White") << "'s turn." << std::endl;

		if (!game.has_valid_move()) {
			std::cout << "No valid move. Passing..." << std::endl;
			std::cout << std::endl;
			std::cout << "-----------------------------" << std::endl;
			std::cout << std::endl;
			game.pass(); // pass
			continue;
		}

		Cell move = game.choose_move();
		if (game.is_AI()) {
			std::cout << "AI's move: " << move.to_string() << std::endl;
		}
		std::cout << std::endl;
		std::cout << "-----------------------------" << std::endl;
		std::cout << std::endl;
		game.play(move);
	}
	int white_count = game.white_count();
	int black_count = game.black_count();
	std::cout << game.to_string() << std::endl;
	std::cout << std::endl;
	std::cout << "Game set. White: " << white_count << ", Black: " << black_count << std::endl;
}