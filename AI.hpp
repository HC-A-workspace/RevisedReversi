#pragma once

#include <chrono>

#include "Board.hpp"

class AI {
protected:
	Board board;
public:
	double elapsed = 0;
	AI() = default;

	AI(const Board& init) : board(init) {};

	virtual void load_board(const Board& board_) {
		board = board_;
	}

	virtual int eval() const = 0;
	
	virtual void play(const Cell& move) {
		if (move.is_pass()) {
			board = board.pass();
		}
		else {
			board = board.play(from_cell(move));
		}
	};
	
	virtual Cell choose_move() = 0;
	
	virtual void pass() {
		board = board.pass();
	}
};

#include <stdlib.h>
#include <time.h>

class RandomAI : public AI {
public:
	RandomAI() : AI() {
		srand((unsigned int)time(NULL));
	};
	
	Cell choose_move() override {
		std::chrono::system_clock::time_point  start, end;
		start = std::chrono::system_clock::now();

		if (!board.has_candidate()) {
			return Cell::Pass();
		}
		BitBoard candidate_board = board.get_candidates();
		std::vector<Cell> candidates = all_cells(candidate_board);
		int idx = rand() % candidates.size();
		Cell move = candidates[idx];

		end = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		return move;
	}

	int eval() const override {
		return 0;
	}
};