#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Game.hpp"
#include "Feature.hpp"

constexpr int OFFSET_BYTES = 16;
constexpr int ONE_GAME_BYTES = 68;


struct WthorTransformer {
private:
	using Byte = unsigned char;

	std::string input_path = "data\\original\\";
	std::string output_path = "data\\formatted\\";
	std::string header;

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

	void transform_game(const std::vector<Byte>& bytes, const size_t game_id, const Player evaluator, const std::string& output_file) {
		const size_t offset = OFFSET_BYTES + ONE_GAME_BYTES * game_id;
		auto it = bytes.begin() + offset;

		it = it + 8;

		std::vector<std::string> features_list;

		Game game;

		Board prev = game.get_board().pass();
		Board current = game.get_board();

		bool is_myturn = (evaluator == BLACK);

		int turn = 0;

		while (!game.is_game_over()) {
			if (turn > 6) {
				auto features = get_feature_params(current, prev, is_myturn);
				if (features.size() == NUM_OF_FEATURES) {
					std::string str_features = "";
					for (auto& f : features) {
						str_features += std::to_string(f) + ",";

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
		const Board& final_board = game.get_board();
		int my_stones = count_stones(is_myturn ? final_board.get_self() : final_board.get_opponent());
		int opponent_stones = count_stones(is_myturn ? final_board.get_opponent() : final_board.get_self());
		double score = result_evaluation(my_stones, opponent_stones);

		std::ofstream file(output_file, std::ios::app);

		if (file.is_open()) {
			std::string result = std::to_string(score);
			for (auto& f : features_list) {
				file << f + result + "\n";
			}
			file.close();
		}
	};

	void transform_single_file(const std::string& name) {
		std::string input_file = input_path + name + ".wtb";
		std::string output_file = output_path + name + ".csv";

		std::ifstream ifile(input_file, std::ios::binary);

		if (!ifile) {
			std::cout << "invalid file" << std::endl;
			return;
		}

		std::vector<Byte> bytes;
		Byte byte = 0;
		while (ifile.read(reinterpret_cast<char*>(&byte), 1)) {
			bytes.push_back(byte);
		}

		size_t num_games = (bytes.size() - OFFSET_BYTES) / ONE_GAME_BYTES;

		Player evaluator = BLACK;

		std::ofstream ofile(output_file);
		if (ofile.is_open()) {
			ofile << header;
			ofile.close();
		}

		for (size_t id = 0; id < num_games; ++id) {
			transform_game(bytes, id, evaluator, output_file);
			evaluator = opponent(evaluator);
		}
	}

public:
	WthorTransformer() {
		header = "";
		for (size_t i = 0; i < NUM_OF_FEATURES; ++i) {
			header += "phi_" + std::to_string(i) + ",";
		}
		header += "result\n";
	};

	void execute() {
		std::vector<std::string> file_names;

		int begin = 2003;
		int end = 2023;

		for (int year = begin; year <= end; year++) {
			file_names.push_back("WTH_" + std::to_string(year));
		}

		for (auto& name : file_names) {
			transform_single_file(name);
			std::cout << name << " is finished\n";
		}
	}
};
