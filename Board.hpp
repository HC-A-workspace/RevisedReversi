#pragma once

#include "BitBoard.hpp"

class Board {
private:
	BitBoard self = 0x0LL;
	BitBoard opponent = 0x0LL;
	BitBoard candidates = 0x0LL;

	void update_candidates() {
		candidates = 0x0LL;

		BitBoard empty = ~(self | opponent);

		for (auto dir : ALL_DIRS) {
			BitBoard shifted = self;
			while(!is_empty(shifted)) {
				shifted = translate(shifted, dir) & opponent;
				candidates |= translate(shifted, dir) & empty;
			}
		}
	}

public:
	Board() = default;
	
	Board(const BitBoard& self_, const BitBoard& opponent_)
		: self(self_), opponent(opponent_) {
		update_candidates();
	};
	
	Board(const std::vector<std::vector<int>>& self_table, const std::vector<std::vector<int>>& opponent_table)
		: Board(from_table(self_table), from_table(opponent_table)) {};
	
	Board(const std::vector<Cell>& self_cells, const std::vector<Cell>& opponent_cells)
		: Board(from_cells(self_cells), from_cells(opponent_cells)) {};

	std::string to_string(const Player& player = BLACK) const {
		char self_mark = (player == BLACK) ? 'x' : 'o';
		char opponent_mark = (player == BLACK) ? 'o' : 'x';

		std::string out = " abcdefgh\n";
		BitBoard stone = 0x1LL;
		for (int n = 0; n < BOARD_SIZE * BOARD_SIZE; ++n) {
			if (n % BOARD_SIZE == 0) {
				out += std::to_string(n / BOARD_SIZE + 1);
			}
			if (!is_empty(stone & self)) {
				out += self_mark;
			}
			else if (!is_empty(stone & opponent)) {
				out += opponent_mark;
			}
			else if (!is_empty(stone & candidates)) {
				out += '_';
			}
			else {
				out += '.';
			}
			if ((n + 1) % BOARD_SIZE == 0) {
				out += '\n';
			}
			stone <<= 1;
		}
		return out;
	}

	BitBoard get_self() const { return self; }
	
	BitBoard get_opponent() const { return opponent; }
	
	BitBoard get_candidates() const { return candidates; }

	std::vector<Cell> get_candidate_list() const { return all_cells(candidates); }
	
	bool has_candidate() const { return !is_empty(candidates); }
	
	bool is_valid_move(const BitBoard& move) const {
		return !is_empty(move & candidates);
	}

	Board play(const BitBoard& move) const {
		BitBoard flipped = 0x0LL;
		for (auto dir : ALL_DIRS) {
			BitBoard captured = 0x0LL;
			BitBoard shifted = translate(move, dir);
			while (!is_empty(shifted & opponent)) {
				shifted &= opponent;
				captured |= shifted;
				shifted = translate(shifted, dir);
			}
			if (!is_empty(shifted & self)) {
				flipped |= captured;
			}
		}
		return Board(opponent ^ flipped, self | move | flipped);
	}

	Board play(const Cell& move) const {
		return play(from_cell(move));
	}

	Board pass() const {
		return Board(opponent, self);
	}

	bool finished() const {
		if (!is_empty(candidates)) return false;
		auto next_board = pass();
		return is_empty(next_board.candidates);
	}
};

std::ostream& operator<<(std::ostream& os, const Board board) {
	os << board.to_string(BLACK);
	return os;
};