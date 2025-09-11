#pragma once

#include "Board.hpp"

class AI {
protected:
	Board board;
public:
	AI(const Board& init) : board(init) {};
	virtual void opponent_move(const Cell& move) = 0;
	virtual Cell choose_move() = 0;
	void pass() {
		board = board.pass();
	}
};

class RandomAI : public AI {
public:
	RandomAI(const Board& init) : AI(init) {};
	void opponent_move(const Cell& move) override {
		if (move.is_pass()) {
			board = board.pass();
		}
		else {
			board = board.play(from_cell(move));
		}
	};
	Cell choose_move() override {
		if (!board.has_candidate()) {
			board = board.pass();
			return Cell::Pass();
		}
		BitBoard candidate_board = board.get_candidates();
		std::vector<Cell> candidates = all_cells(candidate_board);
		int idx = rand() % candidates.size();
		Cell move = candidates[idx];
		board = board.play(from_cell(move));
		return move;
	}
}