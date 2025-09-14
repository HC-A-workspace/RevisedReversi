#pragma once

#include <thread>
#include <mutex>
#include <algorithm>

#include "AI.hpp"

class NegaAlphaAI : public AI {
private:
	double depth;
	int evaluation;
	std::mutex mtx;
	Cell move;

	using BoardInfo = std::pair<Board, Cell>;
	
	static int evaluate(const BoardInfo& board_info, const Board& prev) {
		const Board board = board_info.first;

		const BitBoard& self_board = board.get_self();
		const BitBoard& opponent_board = board.get_opponent();

		const int n_self = count_stones(self_board);
		const int n_opposite = count_stones(opponent_board);

		if (board.finished()) {
			const int score_stone = n_self - n_opposite;
			if (n_self > n_opposite) return score_stone + 100000;
			else if (n_self < n_opposite) return score_stone - 100000;
			else return 0;
		}

		const BitBoard& self_candidates = board.get_candidates();
		const BitBoard& opponent_candidates = prev.get_candidates();


		const int n_self_candidates = count_stones(self_candidates);
		const int n_opponent_candidates = count_stones(opponent_candidates);

		BitBoard self_fixed = 0x0LL, opponent_fixed = 0x0LL;
		calculate_fixed_stones(self_board, opponent_board, self_fixed, opponent_fixed);

		const int n_self_fixed = count_stones(self_fixed);
		const int n_opponent_fixed = count_stones(opponent_fixed);


		const BitBoard corner = 0x8100000000000081LL;
		const int n_self_corner_cands = count_stones(self_candidates & corner);
		const int n_opposite_corner_cands = count_stones(opponent_candidates & corner);

		const int score_candidates = n_self_candidates - n_opponent_candidates;
		const int score_fixed = n_self_fixed - n_opponent_fixed;
		const int score_corner_cands = n_self_corner_cands - n_opposite_corner_cands;

		int score = score_candidates + 5 * score_fixed + 5 * score_corner_cands;

		if (n_opposite_corner_cands > 0) {
			score -= 100;
		}
		if (n_self_corner_cands > 0) {
			score += 100;
		}
		if (n_opponent_fixed > 0) {
			score -= 100;
		}
		if (n_self_fixed > 0) {
			score += 100;
		}

		if (n_self_fixed > BOARD_SIZE * BOARD_SIZE / 2) {
			score += 50000;
		}

		if (n_opponent_fixed > BOARD_SIZE * BOARD_SIZE / 2) {
			score -= 50000;
		}

		return score;
	}

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

	static int nega_alpha(const BoardInfo& board_info, const Board& prev, const double depth, int alpha, int beta) {
		if (depth <= 0 || board_info.first.finished()) {
			return -evaluate(board_info, prev);
		}

		std::vector<BoardInfo> children = sorted_children(board_info.first);

		if (children.size() == 1) {
			return std::max(alpha, -nega_alpha(children.at(0), board_info.first, depth - 0.5, -beta, -alpha));
		}

		int idx = 0;
		for (auto& child : children) {
			if (idx < 2) {
				alpha = std::max(alpha, -nega_alpha(child, board_info.first, depth - 0.7, -beta, -alpha));
			}
			else if (idx < 5) {
				alpha = std::max(alpha, -nega_alpha(child, board_info.first, depth - 1.0, -beta, -alpha));
			}
			else if (idx < 8) {
				alpha = std::max(alpha, -nega_alpha(child, board_info.first, depth - 1.7, -beta, -alpha));
			}
			else {
				alpha = std::max(alpha, -nega_alpha(child, board_info.first, depth - 2.3, -beta, -alpha));
			}
			idx++;
			if (alpha >= beta) return alpha;
		}
		return alpha;
	}

public:
	NegaAlphaAI(const int depth_) : AI(), depth(depth_) {};

	double eval() const override {
		return evaluation;
	}

	void worker(const BoardInfo& child)
	{
		int tmp = nega_alpha(child, board, depth, evaluation, INT_MAX - 1);

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

		std::vector<std::thread> threads;

		evaluation = INT_MIN + 1;

		for (auto& child : children) {
			threads.push_back(std::thread(&NegaAlphaAI::worker, this, child));
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