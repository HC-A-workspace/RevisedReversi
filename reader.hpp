#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Game.hpp"

constexpr int OFFSET_BYTES = 16;
constexpr int ONE_GAME_BYTES = 68;

using Byte = unsigned char;

std::string byte_to_string(const Byte& byte) {
	std::string out = "";
	out += 'a' + ((int)(byte / 10) - 1);
	out += '0' + (byte % 10);
	return out;
}

Cell byte_to_cell(const Byte& byte) {
	if (byte == 0) return Cell::Pass();
	return Cell((int)(byte / 10) - 1, byte % 10 - 1);
}

void read_game(const std::vector<Byte>& bytes, const size_t game_id) {
	const size_t offset = OFFSET_BYTES + ONE_GAME_BYTES * game_id;
	auto it = bytes.begin() + offset;

	it = it + 8;

	Game game;
	std::cout << "\n-----------------------------\n-----------------------------\n";

	while (!game.is_game_over()) {
		std::cout << game.to_string() << std::endl;
		if (game.has_valid_move()) {
			auto move = byte_to_cell(*it++);
			game.is_valid_move(move);
			game.play(move);
			std::cout << "move: " << move.to_string() << std::endl;
		}
		else {
			game.pass();
			std::cout << "move: pass" << std::endl;
		}
		std::cout << "\n-----------------------------\n";
	}
	std::cout << game.to_string() << std::endl;
};

void wthor_reader(std::string wthor_name) {

	std::ifstream file(wthor_name, std::ios::binary);

	if (!file) {
		std::cout << "invalid file" << std::endl;
		return;
	}

	std::vector<Byte> bytes;
	Byte byte = 0;
	while (file.read(reinterpret_cast<char*>(&byte), 1)) {
		bytes.push_back(byte);
	}

	size_t num_games = (bytes.size() - OFFSET_BYTES) / ONE_GAME_BYTES;

	for (size_t id = 0; id < num_games; ++id) {
		read_game(bytes, id);
	}
}