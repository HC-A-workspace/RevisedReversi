#include <iostream>

#include "AI.hpp"
#include "Game.hpp"

int main()
{
	BitBoard board = 0x1LL << 63;
	std::cout << Cell(board).get_loc() << std::endl;
	BitBoard white = from_cells({ Cell("d4"), Cell("e5")});
	BitBoard black = from_cells({ Cell("d5"), Cell("e4") });
	
	Game game(black, white);

	RandomAI black_ai(game.get_board());
	RandomAI white_ai(game.get_opponent_board());
	
	while(!game.is_game_over()) {
		std::cout << game.to_string() << std::endl;
		std::cout << "white: " << game.white_count() << ", black: " << game.black_count() << std::endl;
		std::cout << std::endl;
		std::cout << (game.get_current_player() == BLACK ? "Black" : "White") << "'s turn." << std::endl;

		if (!game.has_valid_move()) {
			std::cout << "No valid move. Passing..." << std::endl;
			game.pass(); // pass
			if (game.get_current_player() == BLACK) {
				white_ai.opponent_move(Cell::Pass());
				black_ai.pass();
			}
			else {
				black_ai.opponent_move(Cell::Pass());
				white_ai.pass();
			}
			continue;
		}
		std::cout << "Your move: ";
		std::string move;
		//std::cin >> move;
		if (game.get_current_player() == BLACK) {
			Cell ai_move = black_ai.choose_move();
			std::cout << ai_move.to_string() << std::endl;
			white_ai.opponent_move(ai_move);
		}
		else {
			Cell ai_move = white_ai.choose_move();
			std::cout << ai_move.to_string() << std::endl;
			black_ai.opponent_move(ai_move);
		}
		bool is_valid = game.is_valid_move(move);
		while(!is_valid) {
			std::cout << "Invalid input. Try again." << std::endl;
			std::cout << "Your move: ";
			std::cin >> move;
			if (!Cell::is_valid(move)) {
				is_valid = false;
			}
			else {
				is_valid = game.is_valid_move(move);
			}
		}
		std::cout << std::endl;
		game.play(move);
	}
	int white_count = game.white_count();
	int black_count = game.black_count();
	std::cout << game.to_string() << std::endl;
	std::cout << "Game over. White: " << white_count << ", Black: " << black_count << std::endl;
}