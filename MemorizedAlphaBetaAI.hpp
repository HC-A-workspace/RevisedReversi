#pragma once

#include <thread>
#include <mutex>
#include <algorithm>

#include "AI.hpp"

class Node {
private:
	int evaluation = 0;
	Cell prev_move;
	bool is_evaluated = false;
	Board board;
	std::vector<std::shared_ptr<Node>> children;

	static int nearby_empty(const BitBoard& stones, const BitBoard& empty) {
		BitBoard near = 0x0LL;

		for (auto& dir : ALL_DIRS) {
			near |= translate(stones, dir) & empty;
		}

		return count_stones(near);
	}

	static int evaluate_child(const Board& board, const Board& prev, const bool is_myturn) {
		const BitBoard& self_board = is_myturn ? board.get_self() : board.get_opponent();
		const BitBoard& opponent_board = is_myturn ? board.get_opponent() : board.get_self();

		const BitBoard empty = ~(self_board | opponent_board);

		const BitBoard& self_candidates = is_myturn ? board.get_candidates() : prev.get_candidates();
		const BitBoard& opponent_candidates = is_myturn ? prev.get_candidates() : board.get_candidates();

		const BitBoard diff = board.get_opponent() ^ prev.get_self();

		const int n_diff_open = nearby_empty(diff, empty);

		const BitBoard corner = 0x8100000000000081LL;

		const int num_cand = count_stones(opponent_candidates);
		const int num_cornoer = count_stones(opponent_candidates & corner);

		const int open = is_myturn ? n_diff_open : -n_diff_open;

		return -num_cand - 3 * num_cornoer + 4 * open;
	}

	void add_child(const Board& board, const Board& prev, const Cell& move, const bool is_myturn) {
		auto child_ptr = std::make_shared<Node>(board);
		child_ptr->prev_move = move;
		child_ptr->evaluation = evaluate_child(board, prev, is_myturn);
		children.push_back(std::move(child_ptr));
	}

public:
	Node() = default;

	Node(const Board& board_) : board(board_) {};

	void sort_children() {
		std::sort(children.begin(), children.end(),
			[](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
				return a->evaluation > b->evaluation;
			});
	}

	~Node() = default;

	bool is_leaf() const {
		return children.empty();
	}

	int get_eval() const { return evaluation; }

	void set_eval(const int& eval) {
		evaluation = eval;
		is_evaluated = true;
	}

	bool evaluated() const { return is_evaluated;  }

	const Board& get_board() const { return board; }

	Cell get_prev_move() const { return prev_move; }

	const std::vector<std::shared_ptr<Node>>& get_children() const { return children; }

	void create_children(const bool is_myturn) {
		std::vector<Cell> candidates = board.get_candidate_list();
		if (candidates.empty()) {
			add_child(board.pass(), board, Cell::Pass(), !is_myturn);
			return;
		}
		for (auto& cell : candidates) {
			add_child(board.play(cell), board, cell, !is_myturn);
		}
	}

	std::shared_ptr<Node> play(const Cell& move) {
		for (auto& child : children) {
			if (child->prev_move == move) {
				return child;
			}
		}
		auto next = std::make_shared<Node>(board.play(move));
		next->prev_move = move;
		return next;
	}
};

class MemorizedAlphaBetaAI : public AI {
private:
	double depth;
	int evaluation;
	std::mutex mtx;
	Cell move;
	int cnt = 0;
	std::shared_ptr<Node> root;

	static int nearby_empty(const BitBoard& stones, const BitBoard& empty) {
		BitBoard near = 0x0LL;

		for (auto& dir : ALL_DIRS) {
			near |= translate(stones, dir) & empty;
		}

		return count_stones(near);
	}

	static int evaluate_child(const Board& board, const Board& prev, const bool is_myturn) {
		const BitBoard& self_board = is_myturn ? board.get_self() : board.get_opponent();
		const BitBoard& opponent_board = is_myturn ? board.get_opponent() : board.get_self();

		const BitBoard empty = ~(self_board | opponent_board);

		const BitBoard& self_candidates = is_myturn ? board.get_candidates() : prev.get_candidates();
		const BitBoard& opponent_candidates = is_myturn ? prev.get_candidates() : board.get_candidates();

		const BitBoard diff = board.get_opponent() ^ prev.get_self();

		const int n_diff_open = nearby_empty(diff, empty);

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

	static int evaluate(const Board& board, const Board& prev, const bool is_myturn) {

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

		const int n_diff_open = nearby_empty(diff ^ (is_myturn ? opponent_fixed : self_fixed), empty);
		const int n_self_open = nearby_empty(self_board ^ self_fixed, empty);
		//const int n_opponent_open = nearby_empty(opponent_board ^ opponent_fixed, ~(self_board | opponent_board));

		const int score_open = -n_self_open + 2 * (is_myturn ? n_diff_open : -n_diff_open);
		const int score_candidates = -n_opponent_candidates;
		const int score_fixed = n_self_fixed - n_opponent_fixed * n_opponent_fixed;
		const int score_corner_cands = -n_opposite_corner_cands * n_opposite_corner_cands;

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

	void alpha_beta(const std::shared_ptr<Node>& board_ptr, const Board& prev, const double depth, const bool is_myturn, int alpha, int beta) {
		auto& board = board_ptr->get_board();
		cnt++;
		if (depth <= 0 || board.finished()) {
			if (!board_ptr->evaluated()) board_ptr->set_eval(evaluate(board, prev, is_myturn));
			return;
		}

		if (board_ptr->get_children().empty()) board_ptr->create_children(is_myturn);
		
		board_ptr->sort_children();
		auto children = board_ptr->get_children();

		if (is_myturn) {
			if (children.size() == 1) {
				alpha_beta(children.at(0), board, depth, !is_myturn, alpha, beta);
				board_ptr->set_eval(std::max(alpha, children.at(0)->get_eval()));
			}

			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					alpha_beta(child, board, depth - 0.7, !is_myturn, alpha, beta);
				}
				else if (idx < 5) {
					alpha_beta(child, board, depth - 1.0, !is_myturn, alpha, beta);
				}
				else if (idx < 8) {
					alpha_beta(child, board, depth - 1.7, !is_myturn, alpha, beta);
				}
				else if (idx < 10) {
					alpha_beta(child, board, depth - 2.3, !is_myturn, alpha, beta);
				}
				else {
					alpha_beta(child, board, depth - 3.0, !is_myturn, alpha, beta);
				}
				alpha = std::max(alpha, child->get_eval());
				idx++;
				if (alpha >= beta) break;
			}
			board_ptr->set_eval(alpha);
		}
		else {
			if (children.size() == 1) {
				alpha_beta(children.at(0), board, depth, !is_myturn, alpha, beta);
				board_ptr->set_eval(std::min(beta, children.at(0)->get_eval()));
			}
			std::reverse(children.begin(), children.end());
			int idx = 0;
			for (auto& child : children) {
				if (idx < 2) {
					alpha_beta(child, board, depth - 0.7, !is_myturn, alpha, beta);
				}
				else if (idx < 5) {
					alpha_beta(child, board, depth - 1.0, !is_myturn, alpha, beta);
				}
				else if (idx < 8) {
					alpha_beta(child, board, depth - 1.7, !is_myturn, alpha, beta);
				}
				else if (idx < 10) {
					alpha_beta(child, board, depth - 2.3, !is_myturn, alpha, beta);
				}
				else {
					alpha_beta(child, board, depth - 3.0, !is_myturn, alpha, beta);
				}
				beta = std::min(beta, child->get_eval());
				idx++;
				if (alpha >= beta) break;
			}
			board_ptr->set_eval(beta);
		}

	}

public:
	MemorizedAlphaBetaAI(const double depth_ = 8.0) : AI(), depth(depth_) {};

	int eval() const override {
		return evaluation;
	}

	void worker(const std::shared_ptr<Node>& child, const Board& board, const double depth)
	{
		alpha_beta(child, board, depth, false, evaluation, INT_MAX - 1);

		std::lock_guard<std::mutex> lock(mtx);
		//if (evaluation < tmp) {
		//	evaluation = tmp;
		//	move = Cell((child.get_opponent() | child.get_self()) ^ (board.get_opponent() | board.get_self()));
		//}
	}

	Cell choose_move() override {
		std::chrono::system_clock::time_point start, end;
		start = std::chrono::system_clock::now();

		if (root->get_children().empty()) {
			root->create_children(true);
		}
		root->sort_children();

		const int rest_turn = count_stones(~(board.get_opponent() | board.get_self()));

		std::vector<std::thread> threads;

		evaluation = INT_MIN + 1;
		cnt = 0;

		if (rest_turn < 13) {
			int cnt = 0;
			for (auto& child : root->get_children()) {
				if (cnt < 2) {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth + 3.5));
				}
				else if (cnt < 6) {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth + 2.0));
				}
				else {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth));
				}
				cnt++;
			}
		}
		else {
			int cnt = 0;
			for (auto& child : root->get_children()) {
				if (cnt < 2) {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth + 0.5));
				}
				else if (cnt < 6) {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth));
				}
				else {
					threads.push_back(std::thread(&MemorizedAlphaBetaAI::worker, this, child, board, depth - 0.5));
				}
				cnt++;
			}
		}

		for (auto& thd : threads)
		{
			thd.join();
		}
		std::cout << "count: " << cnt << std::endl;

		root->sort_children();
		auto& front = root->get_children().at(0);
		move = front->get_prev_move();
		evaluation = front->get_eval();

		end = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		return move;
	}

	void load_board(const Board& board_) {
		AI::load_board(board_);
		root = std::make_shared<Node>(board);
	}

	void play(const Cell& move) {
		AI::play(move);
		if (root->get_children().empty()) {
			root = std::make_shared<Node>(board);
		}
		else {
			root = root->play(move);
		}
	}

	void pass() {
		AI::pass();
		if (root->get_children().empty()) {
			root = std::make_shared<Node>(board);
		}
		else {
			root = root->play(Cell::Pass());
		}
	}
};