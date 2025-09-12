#pragma once

#include <thread>
#include <mutex>

#include "AI.hpp"

class AlphaBetaAI : public AI {
private:
	double depth;
	int evaluation;
	std::mutex mtx;
	Cell move;

	using BoardInfo = std::pair<Board, Cell>;

	static int openness(const BitBoard& empty, const Cell& cell) {
		BitBoard stone = from_cell(cell);
		int cnt = 0;
		for (auto& dir : ALL_DIRS) {
			if (!is_empty(translate(stone, dir) & empty)) {
				cnt++;
			}
		}
		return cnt;
	}

	static int evaluate_boardInfo(const BoardInfo& board_info) {
		const Board& board = board_info.first;
		const BitBoard oppo_candidates = board.get_candidates();
		const BitBoard corner = 0x8100000000000081LL;

		const int num_cand = count_stones(oppo_candidates);
		const int num_cornoer = count_stones(oppo_candidates & corner);

		const Cell& move = board_info.second;
		const int open = openness(~(board.get_self() | board.get_opponent()), move);

		return -num_cand - 3 * num_cornoer - 4 * open;
	}

	static std::vector<BoardInfo> sorted_children(const Board& board) {
		auto candidates = board.get_candidate_list();

		if (candidates.empty()) {
			return std::vector<BoardInfo>({ std::make_pair(board.pass(), Cell::Pass()) });
		}
		else {
			std::vector<BoardInfo> children(candidates.size());
			size_t idx = 0;
			for (auto& cell : candidates) {
				children[idx++] = std::make_pair(board.play(cell), cell);
			}
			std::sort(children.begin(), children.end(),
				[](const BoardInfo& a, const BoardInfo& b) {
					return evaluate_boardInfo(a) > evaluate_boardInfo(b);
				});
			return children;
		}
	}

	static int near_empty(const BitBoard& stones, const BitBoard& empty) {
		BitBoard near = 0x0LL;

		for (auto& dir : ALL_DIRS) {
			near |= translate(stones, dir) & empty;
		}

		return count_stones(near);
	}

	static int evaluate(const BoardInfo& board_info, const Board& prev, const bool is_myturn) {
		const Board board = board_info.first;

		const BitBoard& self_board = is_myturn ? board.get_self() : board.get_opponent();
		const BitBoard& opponent_board = is_myturn ? board.get_opponent() : board.get_self();

		const BitBoard empty = ~(self_board | opponent_board);

		const int n_self = count_stones(self_board);
		const int n_opposite = count_stones(opponent_board);

		if (board.finished()) {
			const int score_stone = n_self - n_opposite;
			if (n_self > n_opposite) return score_stone + 1000000;
			else if (n_self < n_opposite) return score_stone - 1000000;
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

		const int n_diff_open = near_empty(diff ^ (is_myturn ? opponent_fixed : self_fixed), empty);
		const int n_self_open = near_empty(self_board ^ self_fixed, empty);
		//const int n_opponent_open = near_empty(opponent_board ^ opponent_fixed, ~(self_board | opponent_board));

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
			score += 500000;
		}

		if (n_opponent_fixed > BOARD_SIZE * BOARD_SIZE / 2) {
			score -= 500000;
		}

		return score;
	}

	static int alpha_beta(const BoardInfo& board_info, const Board& prev, const double depth, const bool is_myturn, int alpha, int beta) {
		if (depth <= 0 || board_info.first.finished()) {
			return evaluate(board_info, prev, is_myturn);
		}

		std::vector<BoardInfo> children = sorted_children(board_info.first);

		if (is_myturn) {
			if (children.size() == 1) {
				return std::max(alpha, alpha_beta(children.at(0), board_info.first, depth, !is_myturn, alpha, beta));
			}

			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					alpha = std::max(alpha, alpha_beta(child, board_info.first, depth - 0.7, !is_myturn, alpha, beta));
				}
				else if (idx < 5) {
					alpha = std::max(alpha, alpha_beta(child, board_info.first, depth - 1.0, !is_myturn, alpha, beta));
				}
				else if (idx < 8) {
					alpha = std::max(alpha, alpha_beta(child, board_info.first, depth - 1.7, !is_myturn, alpha, beta));
				}
				else if (idx < 10) {
					alpha = std::max(alpha, alpha_beta(child, board_info.first, depth - 2.3, !is_myturn, alpha, beta));
				}
				else {
					alpha = std::max(alpha, alpha_beta(child, board_info.first, depth - 3.0, !is_myturn, alpha, beta));
				}
				idx++;
				if (alpha >= beta) return alpha;
			}
			return alpha;
		}
		else {
			if (children.size() == 1) {
				return std::min(beta, alpha_beta(children.at(0), board_info.first, depth, !is_myturn, alpha, beta));
			}

			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					beta = std::min(beta, alpha_beta(child, board_info.first, depth - 0.7, !is_myturn, alpha, beta));
				}
				else if (idx < 5) {
					beta = std::min(beta, alpha_beta(child, board_info.first, depth - 1.0, !is_myturn, alpha, beta));
				}
				else if (idx < 8) {
					beta = std::min(beta, alpha_beta(child, board_info.first, depth - 1.7, !is_myturn, alpha, beta));
				}
				else if (idx < 10) {
					beta = std::min(beta, alpha_beta(child, board_info.first, depth - 2.3, !is_myturn, alpha, beta));
				}
				else {
					beta = std::min(beta, alpha_beta(child, board_info.first, depth - 3.0, !is_myturn, alpha, beta));
				}
				idx++;
				if (alpha >= beta) return beta;
			}
			return beta;
		}

	}

public:
	AlphaBetaAI(const double depth_ = 8.0) : AI(), depth(depth_) {};

	int eval() const override {
		return evaluation;
	}

	void worker(const BoardInfo& child, const double depth)
	{
		int tmp = alpha_beta(child, board, depth, false, evaluation, INT_MAX - 1);

		std::lock_guard<std::mutex> lock(mtx);
		if (evaluation < tmp) {
			evaluation = tmp;
			move = child.second;
		}
	}

	Cell choose_move() override {
		std::chrono::system_clock::time_point  start, end;
		start = std::chrono::system_clock::now();

		auto children = sorted_children(board);

		const int rest_turn = count_stones(~(board.get_opponent() | board.get_self()));

		std::vector<std::thread> threads;

		evaluation = INT_MIN + 1;

		if (rest_turn < 13) {
			int cnt = 0;
			for (auto& child : children) {
				if (cnt < 2) {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth + 3.5));
				}
				else if (cnt < 6) {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth + 2.0));
				}
				else {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth));
				}
				cnt++;
			}
		}
		else {
			int cnt = 0;
			for (auto& child : children) {
				if (cnt < 2) {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth + 0.5));
				}
				else if (cnt < 6) {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth));
				}
				else {
					threads.push_back(std::thread(&AlphaBetaAI::worker, this, child, depth - 0.5));
				}
				cnt++;
			}
		}

		for (auto& thd : threads)
		{
			thd.join();
		}


		//evaluation = alpha;

		end = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		return move;
	}
};