#pragma once

#include "Board.hpp"

class Game {
private:
	Board board;
	Player current_player = BLACK;
public:
	Game() = default;

	Game(const Board& board_) : board(board_) {};

	Game(const BitBoard& self, const BitBoard& opponent, const Player& player = BLACK)
		: board(self, opponent), current_player(player) {};

	Board get_board() const { return board; }

	Board get_opponent_board() const { return Board(board.get_opponent(), board.get_self()); }

	Player get_current_player() const { return current_player; }

	bool is_game_over() const { return board.finished(); }

	bool has_valid_move() const { return board.has_candidate(); }

	bool is_valid_move(const BitBoard& move) const { return board.is_valid_move(move); }

	bool is_valid_move(const Cell& move) const { return board.is_valid_move(from_cell(move)); }

	bool is_valid_move(const std::string& move) const { return board.is_valid_move(from_cell(Cell(move))); }

	void play(const BitBoard& move) {
		board = board.play(move);
		current_player = opponent(current_player);
	}

	void play(const Cell& move) {
		play(from_cell(move));
	}

	void play(const std::string& move) {
		play(from_cell(Cell(move)));
	}

	void pass() {
		board = board.pass();
		current_player = opponent(current_player);
	}

	std::string to_string() const {
		return board.to_string(current_player);
	}

	int white_count() const {
		if (current_player == WHITE) {
			return count_stones(board.get_self());
		}
		else {
			return count_stones(board.get_opponent());
		}
	}

	int black_count() const {
		if (current_player == BLACK) {
			return count_stones(board.get_self());
		}
		else {
			return count_stones(board.get_opponent());
		}
	}
};

