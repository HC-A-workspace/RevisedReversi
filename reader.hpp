#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Game.hpp"
#include "Feature.hpp"

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

std::string read_game(const std::vector<Byte>& bytes, const size_t game_id) {
	const size_t offset = OFFSET_BYTES + ONE_GAME_BYTES * game_id;
	auto it = bytes.begin() + offset;

	it = it + 8;

	std::vector<std::string> features_list;

	Game game;

	Board prev = game.get_board().pass();
	Board current = game.get_board();

	bool is_myturn = true;

	int turn = 0;

	while (!game.is_game_over()) {
		if (turn > 6) {
			auto features = get_feature_params(current, prev, is_myturn);
			if (!features.empty()) {
				std::string str_features = "";
				for (auto& f : features) {
					str_features += std::to_string(f) + ", ";

				}
				features_list.push_back(str_features);
			}
		}
	
		prev = current;

		if (game.has_valid_move()) {
			auto move = byte_to_cell(*it++);
			game.is_valid_move(move);
			game.play(move);
		}
		else {
			game.pass();
		}
		current = game.get_board();
		is_myturn = !is_myturn;
		turn++;
	}
	return "";
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

	read_game(bytes, 1);
	//for (size_t id = 0; id < num_games; ++id) {
	//	read_game(bytes, id);
	//}
}