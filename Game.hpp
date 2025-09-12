#pragma once

#include <functional>
#include "Board.hpp"
#include "AI.hpp"

class Game {
private:
	Board board;
	Player current_player = BLACK;
	std::unique_ptr<AI> black_ai = nullptr;
	std::unique_ptr<AI> white_ai = nullptr;
	std::function<Cell(const Board&)> human_play;
	Cell move = Cell::Pass();
public:
	Game() = default;

	Game(const Board& board_) : board(board_) {};

	Game(const BitBoard& self, const BitBoard& opponent, const Player& player = BLACK)
		: board(self, opponent), current_player(player) {};

	Board get_board() const { return board; }

	Board get_opponent_board() const { return Board(board.get_opponent(), board.get_self()); }

	Player get_current_player() const { return current_player; }

	void set_white_AI(std::unique_ptr<AI> ai) {
		white_ai = std::move(ai);
		white_ai->load_board(board);
	}

	void set_black_AI(std::unique_ptr<AI> ai) {
		black_ai = std::move(ai);
		black_ai->load_board(board);
	}

	void set_human_play(std::function<Cell(const Board&)> human_play_) {
		human_play = human_play_;
	}

	bool is_game_over() const { return board.finished(); }

	bool has_valid_move() const { return board.has_candidate(); }

	bool is_valid_move(const BitBoard& move) const { return board.is_valid_move(move); }

	bool is_valid_move(const Cell& move) const { return board.is_valid_move(from_cell(move)); }

	bool is_valid_move(const std::string& move) const { return board.is_valid_move(from_cell(Cell(move))); }

	inline AI* current_AI() const {
		return (current_player == BLACK) ? black_ai.get() : white_ai.get();
	}

	Cell choose_move() {
		AI* ai = current_AI();
		if (ai != nullptr) {
			move = ai->choose_move();
			std::cout << "evaluation: " << ai->eval() << std::endl;
			std::cout << "time: " << ai->elapsed << " ms" << std::endl;
		}
		else {
			move = human_play(board);
		}
		return move;
	}

	void play(const BitBoard& move) {
		if (black_ai != nullptr) black_ai->play(move);
		if (white_ai != nullptr) white_ai->play(move);
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
		if (black_ai != nullptr) black_ai->pass();
		if (white_ai != nullptr) white_ai->pass();
		board = board.pass();
		current_player = opponent(current_player);
	}

	std::string to_string() const {
		return board.to_string(current_player, true, move);
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

	bool is_AI() const {
		return current_AI() != nullptr;
	}
};

