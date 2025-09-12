#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>
#include <string>

using BitBoard = unsigned long long;
constexpr int BOARD_SIZE = 8;

inline bool is_empty(const BitBoard& board) {
	return board == 0x0LL;
}

inline int msb_loc(const BitBoard& stone) {
	BitBoard tmp = stone;
	int ans = 0;
	if (!is_empty(tmp >> 32)) {
		tmp >>= 32;
		ans += 32;
	}
	if (!is_empty(tmp >> 16)) {
		tmp >>= 16;
		ans += 16;
	}
	if (!is_empty(tmp >> 8)) {
		tmp >>= 8;
		ans += 8;
	}
	if (!is_empty(tmp >> 4)) {
		tmp >>= 4;
		ans += 4;
	}
	if (!is_empty(tmp >> 2)) {
		tmp >>= 2;
		ans += 2;
	}
	if (!is_empty(tmp >> 1)) {
		tmp >>= 1;
		ans += 1;
	}
	return ans;
}

class Cell {
private:
	int loc = -1;
public:
	Cell() = default;

	Cell(const int& x, const int& y) : loc(x + BOARD_SIZE * y) {};

	Cell(const std::string str) {
		char column = str[0];
		char row = str[1];
		int x = column - 'a';
		int y = row - '1';
		loc = x + BOARD_SIZE * y;
	};

	Cell(const BitBoard& stone) : loc(msb_loc(stone)) { };

	int X() const { return loc % BOARD_SIZE; };

	int Y() const { return loc / BOARD_SIZE; };

	int get_loc() const { return loc; };

	static bool is_valid(const std::string str) {
		if (str.length() != 2) return false;
		char column = str[0];
		char row = str[1];
		int x = column - 'a';
		int y = row - '1';
		if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
			return false;
		}
		return true;
	};

	static Cell Pass() {
		return Cell();
	};

	bool is_pass() const {
		return loc == -1;
	}

	std::string to_string() const {
		if (is_pass()) return "pass";
		std::string out = "";
		out += ('a' + X());
		out += ('1' + Y());
		return out;
	}
};

bool operator==(const Cell& lhs, const Cell& rhs) {
	return lhs.get_loc() == rhs.get_loc();
}

enum Player {
	WHITE, BLACK
};

Player opponent(const Player& player) {
	return player == WHITE ? BLACK : WHITE;
}

enum Direction {
	UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT
};

constexpr Direction ALL_DIRS[8] = {
	UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT
};

std::string to_string(const BitBoard& board, const char& mark = 'o') {
	std::string out = " abcdefgh";
	BitBoard stone = 0x1LL;
	for (int n = 0; n < BOARD_SIZE * BOARD_SIZE; ++n) {
		if (n % BOARD_SIZE == 0) {
			out += std::to_string(n / BOARD_SIZE + 1);
		}
		if (is_empty(stone & board)) {
			out += '.';
		}
		else {
			out += mark;
		}
		if ((n + 1) % BOARD_SIZE == 0) {
			out += '\n';
		}
		stone <<= 1;
	}
	return out;
}

BitBoard from_table(const std::vector<std::vector<int>>& table) {
	int cnt = 0;
	BitBoard board = 0x0LL;
	BitBoard stone = 0x1LL;
	for (auto& v : table) {
		for (auto& i : v) {
			if (i != 0) board |= stone;
			cnt++;
			stone <<= 1;
		}
	}
	if (cnt != BOARD_SIZE * BOARD_SIZE) {
		throw std::length_error("table size is invalid");
	}
	return board;
};

inline BitBoard from_XY(const int& X, const int& Y) {
	return 0x1LL << (X + BOARD_SIZE * Y);
}

inline BitBoard from_cell(const Cell& cell) {
	return 0x1LL << cell.get_loc();
}

BitBoard from_cells(const std::vector<Cell>& cells) {
	BitBoard board = 0x0LL;
	for (auto& cell : cells) {
		board |= from_cell(cell);
	}
	return board;
}

static std::vector<Cell> all_cells(const BitBoard& board) {
	std::vector<Cell> out;
	auto tmp = board;
	while (!is_empty(tmp)) {
		Cell cell(tmp);
		out.push_back(cell);
		tmp ^= from_cell(cell);
	}
	return out;
}

inline int count_stones(const BitBoard& board) {
	auto bmap = board;
	bmap = (bmap & 0x5555555555555555LL) + ((bmap >> 1) & 0x5555555555555555LL);
	bmap = (bmap & 0x3333333333333333LL) + ((bmap >> 2) & 0x3333333333333333LL);
	bmap = (bmap & 0x0f0f0f0f0f0f0f0fLL) + ((bmap >> 4) & 0x0f0f0f0f0f0f0f0fLL);
	bmap = (bmap & 0x00ff00ff00ff00ffLL) + ((bmap >> 8) & 0x00ff00ff00ff00ffLL);
	bmap = (bmap & 0x0000ffff0000ffffLL) + ((bmap >> 16) & 0x0000ffff0000ffffLL);
	return (bmap & 0x00000000ffffffffLL) + ((bmap >> 32) & 0x00000000ffffffffLL);
}

#define REPEAT_BITS(b) 0x ## b ## b ## b ## b ## b ## b ## b ## b ## LL

constexpr BitBoard LEFT_MASK[BOARD_SIZE + 1] = {
	REPEAT_BITS(00),
	REPEAT_BITS(01),
	REPEAT_BITS(03),
	REPEAT_BITS(07),
	REPEAT_BITS(0f),
	REPEAT_BITS(1f),
	REPEAT_BITS(3f),
	REPEAT_BITS(7f),
	REPEAT_BITS(ff)
};

constexpr BitBoard RIGHT_MASK[BOARD_SIZE + 1] = {
	REPEAT_BITS(00),
	REPEAT_BITS(80),
	REPEAT_BITS(c0),
	REPEAT_BITS(e0),
	REPEAT_BITS(f0),
	REPEAT_BITS(f8),
	REPEAT_BITS(fc),
	REPEAT_BITS(fe),
	REPEAT_BITS(ff)
};

inline void translate_self(BitBoard& board, const Direction& dir, const int& shift = 1) {
	if (dir == UP) {
		board >>= BOARD_SIZE * shift;
	}
	else if (dir == DOWN) {
		board <<= BOARD_SIZE * shift;
	}
	else if (dir == LEFT) {
		board >>= shift;
		board &= LEFT_MASK[BOARD_SIZE - shift];
	}
	else if (dir == RIGHT) {
		board <<= shift;
		board &= RIGHT_MASK[BOARD_SIZE - shift];
	}
	else if (dir == UP_LEFT) {
		board >>= (BOARD_SIZE + 1) * shift;
		board &= LEFT_MASK[BOARD_SIZE - shift];
	}
	else if (dir == UP_RIGHT) {
		board >>= (BOARD_SIZE - 1) * shift;
		board &= RIGHT_MASK[BOARD_SIZE - shift];
	}
	else if (dir == DOWN_LEFT) {
		board <<= (BOARD_SIZE - 1) * shift;
		board &= LEFT_MASK[BOARD_SIZE - shift];
	}
	else if (dir == DOWN_RIGHT) {
		board <<= (BOARD_SIZE + 1) * shift;
		board &= RIGHT_MASK[BOARD_SIZE - shift];
	}
	else {
		throw std::invalid_argument("invalid direction");
	}
}

inline BitBoard translate(const BitBoard& board, const Direction& dir, const int& shift=1) {
	auto out = board;
	translate_self(out, dir, shift);
	return out;
}

BitBoard calculate_candidates(const BitBoard& self, const BitBoard& opponent) {
	BitBoard candidates = 0x0LL;

	BitBoard empty = ~(self | opponent);

	for (auto dir : ALL_DIRS) {
		BitBoard shifted = self;
		while (!is_empty(shifted)) {
			shifted = translate(shifted, dir) & opponent;
			candidates |= translate(shifted, dir) & empty;
		}
	}

	return candidates;
}

inline void calculate_fixed_stones_helper(const BitBoard& self, const BitBoard& opponent, const BitBoard& empty, BitBoard& self_fixed, BitBoard& opponent_fixed,
	const Direction& dir_1, const Direction& dir_2) {

	BitBoard fixed_1 = translate(empty, dir_1, 1) | empty;
	fixed_1 |= translate(fixed_1, dir_1, 2);
	fixed_1 |= translate(fixed_1, dir_1, 4);

	BitBoard fixed_2 = translate(empty, dir_2, 1) | empty;
	fixed_2 |= translate(fixed_2, dir_2, 2);
	fixed_2 |= translate(fixed_2, dir_2, 4);

	BitBoard fixed_self_1 = translate(~self, dir_1, 1) | ~self;
	fixed_self_1 |= translate(fixed_self_1, dir_1, 2);
	fixed_self_1 |= translate(fixed_self_1, dir_1, 4);

	BitBoard fixed_self_2 = translate(~self, dir_2, 1) | ~self;
	fixed_self_2 |= translate(fixed_self_2, dir_2, 2);
	fixed_self_2 |= translate(fixed_self_2, dir_2, 4);

	BitBoard fixed_opponent_1 = translate(~opponent, dir_1, 1) | ~opponent;
	fixed_opponent_1 |= translate(fixed_opponent_1, dir_1, 2);
	fixed_opponent_1 |= translate(fixed_opponent_1, dir_1, 4);

	BitBoard fixed_opponent_2 = translate(~opponent, dir_2, 1) | ~opponent;
	fixed_opponent_2 |= translate(fixed_opponent_2, dir_2, 2);
	fixed_opponent_2 |= translate(fixed_opponent_2, dir_2, 4);

	self_fixed &= (~(fixed_1 | fixed_2)) | (~(fixed_self_1 & fixed_self_2));
	opponent_fixed &= (~(fixed_1 | fixed_2)) | (~(fixed_opponent_1 & fixed_opponent_2));
}

void calculate_fixed_stones(const BitBoard& self, const BitBoard& opponent, BitBoard& self_fixed, BitBoard& opponent_fixed) {
	BitBoard empty = ~(self | opponent);
	self_fixed = self;
	opponent_fixed = opponent;

	//Fixed for LR
	calculate_fixed_stones_helper(self, opponent, empty, self_fixed, opponent_fixed, LEFT, RIGHT);

	//Fixed for UD
	calculate_fixed_stones_helper(self, opponent, empty, self_fixed, opponent_fixed, UP, DOWN);

	//Fixed for UL-DR
	calculate_fixed_stones_helper(self, opponent, empty, self_fixed, opponent_fixed, UP_LEFT, DOWN_RIGHT);

	//Fixed for UR-DL
	calculate_fixed_stones_helper(self, opponent, empty, self_fixed, opponent_fixed, UP_RIGHT, DOWN_LEFT);
}