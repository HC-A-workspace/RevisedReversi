#pragma once

#include "Board.hpp"

class AI {
protected:
	Board board;
public:
	AI() = default;

	AI(const Board& init) : board(init) {};

	virtual void load_board(const Board& board_) {
		board = board_;
	}

	virtual int eval() const = 0;
	
	virtual void play(const Cell& move) = 0;
	
	virtual Cell choose_move() = 0;
	
	void pass() {
		board = board.pass();
	}
};

class RandomAI : public AI {
public:
	RandomAI() : AI() {};
	void play(const Cell& move) override {
		if (move.is_pass()) {
			board = board.pass();
		}
		else {
			board = board.play(from_cell(move));
		}
	};
	
	Cell choose_move() override {
		if (!board.has_candidate()) {
			return Cell::Pass();
		}
		BitBoard candidate_board = board.get_candidates();
		std::vector<Cell> candidates = all_cells(candidate_board);
		int idx = rand() % candidates.size();
		Cell move = candidates[idx];
		return move;
	}

	int eval() const override {
		return 0;
	}
};