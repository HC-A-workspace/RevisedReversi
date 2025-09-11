#pragma once

#include <vector>
#include <stdexcept>
#include <iostream>
#include <string>

using BitBoard = unsigned long long;
constexpr int BOARD_SIZE = 8;

struct Cell {
	int X, Y;
	Cell(const int& x, const int& y) : X(x), Y(y) {};
	Cell(const std::string loc) {
		char column = loc[0];
		char row = loc[1];
		X = column - 'a';
		Y = row - '1';
		if (X < 0 || X >= BOARD_SIZE || Y < 0 || Y >= BOARD_SIZE) {
			throw std::out_of_range("invalid cell");
		}
	};
};

inline bool is_empty(const BitBoard& board) {
	return board == 0x0LL;
}

std::string to_string(const BitBoard& board, const char& mark = 'o') {
	std::string out = "";
	BitBoard stone = 0x1LL;
	for (int n = 0; n < BOARD_SIZE * BOARD_SIZE; ++n) {
		if (is_empty(stone & board)) {
			out += '.';
		} else {
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
			stone << 1;
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

BitBoard from_cell(const Cell& cell) {
	return 0x1LL << (cell.X + BOARD_SIZE * cell.Y);
}

BitBoard from_cells(const std::vector<Cell>& cells) {
	BitBoard board = 0x0LL;
	for (auto& cell : cells) {
		board |= from_cell(cell);
	}
	return board;
}

enum Player {
	WHITE, BLACK
};

class Board {
private:
	BitBoard white = 0x0LL;
	BitBoard black = 0x0LL;
	BitBoard candidates = 0x0LL;
	Player player = BLACK;
public:
	Board() = default;
	Board(const BitBoard& white_, const BitBoard& black_, const Player player_)
		: white(white_), black(black_), player(player_) {};
	Board(const std::vector<std::vector<int>>& white_table, const std::vector<std::vector<int>>& black_table, const Player& player)
		: Board(from_table(white_table), from_table(black_table), player) {};
	Board(const std::vector<Cell>& white_cells, const std::vector<Cell>& black_cells, const Player& player)
		: Board(from_cells(white_cells), from_cells(black_cells), player) {};

	BitBoard get_white() const { return white; }
	BitBoard get_black() const { return black; }
	Player get_player() const { return player; }
};

std::ostream& operator<<(std::ostream& os, const Board board) {
	BitBoard stone = 0x1LL;
	for (int n = 0; n < BOARD_SIZE * BOARD_SIZE; ++n) {
		if (!is_empty(stone & board.get_white())) {
			os << 'o';
		} else if (!is_empty(stone & board.get_black())){
			os << 'x';
		} else {
			os << '.';
		}
		if ((n + 1) % BOARD_SIZE == 0) {
			os << '\n';
		}
		stone <<= 1;
	}
	return os;
}