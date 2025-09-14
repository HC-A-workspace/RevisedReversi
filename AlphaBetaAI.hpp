#pragma once

#include <thread>
#include <mutex>
#include <algorithm>

#include "AI.hpp"

class AlphaBetaAI : public AI {
protected:
	double depth;
	double evaluation = 0;
	std::mutex mtx;
	Cell move = Cell::Pass();
	double depth_offset = 0.0;

	static int evaluate_child(const Board& board, const Board& prev, const bool is_myturn) {
		const BitBoard& self_board = is_myturn ? board.get_self() : board.get_opponent();
		const BitBoard& opponent_board = is_myturn ? board.get_opponent() : board.get_self();

		const BitBoard empty = ~(self_board | opponent_board);

		const BitBoard& self_candidates = is_myturn ? board.get_candidates() : prev.get_candidates();
		const BitBoard& opponent_candidates = is_myturn ? prev.get_candidates() : board.get_candidates();

		const BitBoard diff = board.get_opponent() ^ prev.get_self();

		const int n_diff_open = openness(diff, empty);

		const BitBoard corner = 0x8100000000000081LL;

		const int num_cand = count_stones(opponent_candidates);
		const int num_cornoer = count_stones(opponent_candidates & corner);

		const int open = is_myturn ? n_diff_open : -n_diff_open;

		return -num_cand - 3 * num_cornoer + 4 * open;
	}

	static std::vector<Board> sorted_children(const Board& board, const bool is_myturn) {
		auto candidates = board.get_candidate_list();

		if (candidates.empty()) {
			return std::vector<Board>({ board.pass() });
		}
		else {
			std::vector<Board> children(candidates.size());
			size_t idx = 0;
			for (auto& cell : candidates) {
				children[idx++] = board.play(cell);
			}
			std::sort(children.begin(), children.end(),
				[board, is_myturn](const Board& a, const Board& b) {
					return evaluate_child(a, board, !is_myturn) > evaluate_child(b, board, !is_myturn);
				});
			return children;
		}
	}

	virtual double evaluate(const Board& board, const Board& prev, const bool is_myturn) {

		const BitBoard& self_board = is_myturn ? board.get_self() : board.get_opponent();
		const BitBoard& opponent_board = is_myturn ? board.get_opponent() : board.get_self();

		const BitBoard empty = ~(self_board | opponent_board);

		const int n_self = count_stones(self_board);
		const int n_opposite = count_stones(opponent_board);

		if (board.finished()) {
			const int score_stone = n_self - n_opposite;
			if (n_self > n_opposite) return score_stone + 100000;
			else if (n_self < n_opposite) return score_stone - 100000;
			else return 0;
		}

		const BitBoard& self_candidates = is_myturn ? board.get_candidates() : prev.get_candidates();
		const BitBoard& opponent_candidates = is_myturn ? prev.get_candidates() : board.get_candidates();

		const int n_self_candidates = count_stones(self_candidates);
		const int n_opponent_candidates = count_stones(opponent_candidates);

		BitBoard self_fixed = 0x0LL, opponent_fixed = 0x0LL;
		calculate_fixed_stones(self_board, opponent_board, self_fixed, opponent_fixed);

		const int n_self_fixed = count_stones(self_fixed);
		const int n_opponent_fixed = count_stones(opponent_fixed);


		const BitBoard corner = 0x8100000000000081LL;
		const int n_self_corner_cands = count_stones(self_candidates & corner);
		const int n_opposite_corner_cands = count_stones(opponent_candidates & corner);

		const BitBoard diff = board.get_opponent() ^ prev.get_self();

		const int n_diff_open = openness(diff ^ (is_myturn ? opponent_fixed : self_fixed), empty);
		const int n_self_open = openness(self_board ^ self_fixed, empty);
		//const int n_opponent_open = openness(opponent_board ^ opponent_fixed, ~(self_board | opponent_board));

		const int score_open = - n_self_open + 2 * (is_myturn ? n_diff_open : -n_diff_open);
		const int score_candidates = - n_opponent_candidates;
		const int score_fixed = n_self_fixed - n_opponent_fixed * n_opponent_fixed;
		const int score_corner_cands = - n_opposite_corner_cands * n_opposite_corner_cands;

		int score = 3 * score_candidates + 5 * score_fixed + 5 * score_corner_cands + score_open;

		if (n_opposite_corner_cands > 0) {
			score -= 1000;
		}
		if (n_opponent_fixed > 0) {
			score -= 1000;
		}
		if (n_self_corner_cands > 0) {
			score += 500;
		}
		if (n_self_fixed > 0) {
			score += 500;
		}
		if (n_self_fixed > BOARD_SIZE * BOARD_SIZE / 2) {
			score += 50000;
		}

		if (n_opponent_fixed > BOARD_SIZE * BOARD_SIZE / 2) {
			score -= 50000;
		}

		return score;
	}

	double alpha_beta(const Board& board, const Board& prev, const double depth, const bool is_myturn, double alpha, double beta) {
		if (depth <= 0 || board.finished()) {
			return evaluate(board, prev, is_myturn);
		}

		std::vector<Board> children = sorted_children(board, is_myturn);

		if (is_myturn) {
			if (children.size() == 1) {
				return std::max(alpha, alpha_beta(children.at(0), board, depth, !is_myturn, alpha, beta));
			}

			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					alpha = std::max(alpha, alpha_beta(child, board, depth - 0.7, !is_myturn, alpha, beta));
				}
				else if (idx < 5) {
					alpha = std::max(alpha, alpha_beta(child, board, depth - 1.0, !is_myturn, alpha, beta));
				}
				else if (idx < 8) {
					alpha = std::max(alpha, alpha_beta(child, board, depth - 1.7, !is_myturn, alpha, beta));
				}
				else if (idx < 10) {
					alpha = std::max(alpha, alpha_beta(child, board, depth - 2.3, !is_myturn, alpha, beta));
				}
				else {
					alpha = std::max(alpha, alpha_beta(child, board, depth - 3.0, !is_myturn, alpha, beta));
				}
				idx++;
				if (alpha >= beta) return alpha;
			}
			return alpha;
		}
		else {
			if (children.size() == 1) {
				return std::min(beta, alpha_beta(children.at(0), board, depth, !is_myturn, alpha, beta));
			}
			std::reverse(children.begin(), children.end());

			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					beta = std::min(beta, alpha_beta(child, board, depth - 0.7, !is_myturn, alpha, beta));
				}
				else if (idx < 5) {
					beta = std::min(beta, alpha_beta(child, board, depth - 1.0, !is_myturn, alpha, beta));
				}
				else if (idx < 8) {
					beta = std::min(beta, alpha_beta(child, board, depth - 1.7, !is_myturn, alpha, beta));
				}
				else if (idx < 10) {
					beta = std::min(beta, alpha_beta(child, board, depth - 2.3, !is_myturn, alpha, beta));
				}
				else {
					beta = std::min(beta, alpha_beta(child, board, depth - 3.0, !is_myturn, alpha, beta));
				}
				idx++;
				if (alpha >= beta) return beta;
			}
			return beta;
		}

	}

public:
	AlphaBetaAI(const double depth_ = 8.0) : AI(), depth(depth_) {};

	double eval() const override {
		return evaluation;
	}

	void worker(const Board& child, const Board& board, const double depth)
	{
		double tmp = alpha_beta(child, board, depth, false, evaluation, INT_MAX - 1);

		std::lock_guard<std::mutex> lock(mtx);
		if (evaluation < tmp) {
			evaluation = tmp;
			move = Cell((child.get_opponent() | child.get_self()) ^ (board.get_opponent() | board.get_self()));
		}
	}

	Cell choose_move() override {
		std::chrono::system_clock::time_point start, end;
		start = std::chrono::system_clock::now();

		auto children = sorted_children(board, true);

		const int rest_turn = count_stones(~(board.get_opponent() | board.get_self()));

		std::vector<std::thread> threads;

		evaluation = INT_MIN + 1;

		if (rest_turn == 12) depth_offset += 2.0;

		int cnt = 0;
		for (auto& child : children) {
			if (cnt < 2) {
				threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, board, depth + depth_offset + 0.5));
			}
			else if (cnt < 6) {
				threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, board, depth + depth_offset));
			}
			else {
				threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, board, depth + depth_offset - 0.5));
			}
			cnt++;
		}

		for (auto& thd : threads)
		{
			thd.join();
		}


		end = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		return move;
	}

	void clear() override {
		evaluation = 0;
		move = Cell::Pass();
	}
};