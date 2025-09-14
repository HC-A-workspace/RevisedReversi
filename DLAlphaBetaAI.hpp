#pragma once

#include <fstream>

#include "AlphaBetaAI.hpp"
#include "Feature.hpp"

class DLAlphaBetaAI : public AlphaBetaAI {
private:
	std::vector<double> W1;
	std::vector<double> b1;
	std::vector<double> W2;
	std::vector<double> b2;
	std::vector<double> Wo;
	std::vector<double> bo;

	int D, H1, H2;

	int cnt_leaf = 0;
	int cnt_definite_leaf = 0;
	bool depth_updated = false;

	std::string data_path = "data\\weight\\data.txt";

	static void read_vector(std::istream& is, std::vector<double>& dst, size_t n) {
		dst.resize(n);
		for (size_t idx = 0; idx < n; ++idx) {
			is >> dst[idx];
		}
	}

	inline static double ReLU(double x) { return (x > 0.5) ? 0.5 : 0.0; }

	inline double dot_row(const std::vector<double>& W, size_t row, size_t cols, const std::vector<double>& x) const {
		const double* w = &W[row * cols];
		double s = 0.0;
		for (size_t j = 0;j < cols;++j) s += w[j] * x[j];
		return s;
	}

	inline double predict(const std::vector<double>& features) const {
		std::vector<double> h1(H1);
		for (int k = 0;k < H1;++k) {
			double z = dot_row(W1, k, D, features) + b1[k];
			h1[k] = ReLU(z);
		}

		std::vector<double> h2(H2);
		for (int k = 0;k < H2;++k) {
			double z = dot_row(W2, k, H1, h1) + b2[k];
			h2[k] = ReLU(z);
		}

		return dot_row(Wo, 0, H2, h2) + bo[0];
	}

	double evaluate(const Board& board, const Board& prev, const bool is_myturn) override {
		cnt_leaf++;
		auto features = get_feature_params(board, prev, is_myturn);
		if (features.size() == NUM_OF_FEATURES) {
			return predict(features);
		}
		cnt_definite_leaf++;
		//feature = { num_my_stone, num_opponent_stone, num_my_cand, num_opponent_cand, num_my_fixed, num_opponent_fixed }
		if (board.finished()) {
			const int stone_diff = (int)features[0] - (int)features[1];
			if (stone_diff > 0.0) {
				return stone_diff + 1.0e5 - (features[1] > 0.0 ? 1.0e4 : 0.0);
			}
			else if (stone_diff < 0.0) {
				return stone_diff - 1.0e5;
			}
			else {
				const int num_my_stones = (int)features[0];
				return result_evaluation(num_my_stones, num_my_stones);
			}
		}
		else if (features[4] > features[5]) {
			return features[4] * features[4] - 2 * features[5] - features[3] + 5.0e4 - (features[5] > 0.0 ? 1.0e4 : 0.0);
		}
		else {
			return features[4] * features[4] - 2 * features[5] - features[3] - 5.0e4;
		}
	}

public:
	DLAlphaBetaAI(const double depth_ = 6.0) : AlphaBetaAI(depth_) {
		std::ifstream file(data_path);

		if (!file) throw std::runtime_error("cannot open data file");

		file >> D >> H1 >> H2;
		read_vector(file, W1, D * H1);
		read_vector(file, b1, H1);
		read_vector(file, W2, H1 * H2);
		read_vector(file, b2, H2);
		read_vector(file, Wo, H2);
		read_vector(file, bo, 1);
	}

	Cell choose_move() override {
		cnt_definite_leaf = 0;
		cnt_leaf = 0;

		Cell out = AlphaBetaAI::choose_move();

		if (!depth_updated && (double)cnt_definite_leaf / (double)cnt_leaf > 0.01) {
			depth_offset += 2.0;
			depth_updated = true;
		}
		std::cout << (double)cnt_definite_leaf / (double)cnt_leaf << std::endl;
		return out;
	}

};