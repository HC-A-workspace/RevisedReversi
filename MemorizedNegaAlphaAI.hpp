#pragma once

#include <memory>
#include <algorithm>

#include "AI.hpp"

class Node {
private:
	bool is_evaluated = false;
	int evaluation = 0;
	Cell prev_move;
	Node* parent = nullptr;
	Board board;
	std::vector<std::shared_ptr<Node>> children;
public:
	Node() = default;

	Node(const Board& board_) : board(board_) {};

	void add_child(const Board& child, const Cell& move) {
		auto child_ptr = std::make_shared<Node>(child);
		child_ptr->parent = this;
		child_ptr->prev_move = move;
		children.push_back(std::move(child_ptr));
	}

	void remove_children(const int idx) {
		children.resize(idx);
	}

	void sort_children() {
		std::sort(children.begin(), children.end(),
			[](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
				if (a->is_evaluated != b->is_evaluated) {
					if (a->is_evaluated) return a->evaluation > 0;
					else return b->evaluation <= 0;
				}

				if (!a->is_evaluated) return a.get() < b.get();

				if (a->evaluation != b->evaluation) return a->evaluation > b->evaluation;

				return a.get() < b.get();
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

	Board get_board() const { return board; }

	Node* get_parent() const { return parent; }

	Cell get_prev_move() const { return prev_move; }

	bool evaluated() const { return is_evaluated;  }

	const std::vector<std::shared_ptr<Node>>& get_children() const { return children; }

	void create_children() {
		std::vector<Cell> candidates = board.get_candidate_list();
		if (candidates.empty()) {
			add_child(board.pass(), Cell::Pass());
			return;
		}
		for (auto& cell : candidates) {
			add_child(board.play(cell), cell);
		}
		std::sort(children.begin(), children.end(),
			[](const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) {
				const int a_cand = count_stones(a->board.get_candidates());
				const int b_cand = count_stones(a->board.get_candidates());
				return a_cand < b_cand;
			});
	}

	std::shared_ptr<Node> play(const Cell& move) {
		for (auto& child : children) {
			if (child->prev_move == move) {
				return child;
			}
		}
		auto next = std::make_shared<Node>(board.play(move));
		next->parent = this;
		next->prev_move = move;
		return next;
	}

	std::string to_string(int i = 0) {
		std::string str = "";
		str += std::to_string(i);
		str += ' ';
		if (is_evaluated) {
			str += std::to_string(evaluation);
		}
		else {
			str += "NaN";
		}
		str += " ";
		str += prev_move.to_string();
		str += '\n';
		for (auto& child : children) {
			str += child->to_string(i + 1);
		}
		return str;
	}
};

#include <cmath>
#include <thread>
#include <mutex>

class MemorizedNegaAlphaAI : public AI {
private:
	double depth = 0;
	std::shared_ptr<Node> root;
	int cnt = 0;


	void alpha_beta(Node& node, const double depth_, int alpha, int beta) {
		cnt++;
		if (depth_ <= 0) {
			node.set_eval(-eval_leaf(node));
			return;
		}
		else if (node.get_board().finished()) {
			if (!node.evaluated()) {
				node.set_eval(-eval_leaf(node));
			}
			return;
		}
		else if (node.is_leaf()) {
			node.create_children();
		}

		auto& ch = node.get_children();

		if (ch.size() == 1) {
			auto& child = ch.at(0);
			alpha_beta(*child, depth_ - 0.5, -beta, -alpha);
			alpha = std::max(alpha, -child->get_eval());
			node.set_eval(alpha);
		}
		else if(ch.size() > 6) {
			int idx = 0;
			for (auto& child : ch) {
				if (child->get_eval() < 0) {
					alpha_beta(*child, depth_ - 3, -beta, -alpha);
				}
				else if (idx < 4) {
					alpha_beta(*child, depth_ - 0.7, -beta, -alpha);
				}
				else if (idx < 6) {
					alpha_beta(*child, depth_ - 1, -beta, -alpha);
				}
				else if (idx < 10) {
					alpha_beta(*child, depth_ - 2.2, -beta, -alpha);
				}
				else {
					alpha_beta(*child, depth_ - 3, -beta, -alpha);
				}

				alpha = std::max(alpha, -child->get_eval());
				idx++;
				if (alpha >= beta) {
					break;
				}
			}

			node.sort_children();


			node.set_eval(alpha);
		}
		else {
			int idx = 0;
			for (auto& child : ch) {
				if (child->get_eval() < 0) {
					alpha_beta(*child, depth_ - 2, -beta, -alpha);
				}
				else {
					alpha_beta(*child, depth_ - 1, -beta, -alpha);
				}
				alpha = std::max(alpha, -child->get_eval());
				idx++;
				if (alpha >= beta) {
					break;
				}
			}

			node.sort_children();


			node.set_eval(alpha);
		}

	}
	
public:
	static int eval_leaf(const Node& node) {
		const Board board = node.get_board();

		const BitBoard& self_board = board.get_self();
		const BitBoard& opponent_board = board.get_opponent();

		const int n_self = count_stones(self_board);
		const int n_opposite = count_stones(opponent_board);

		if (board.finished()) {
			const int score_stone = n_self - n_opposite;
			if (n_self > n_opposite) return score_stone + 5000;
			else if (n_self < n_opposite) return score_stone - 5000;
			else return 0;
		}
		const BitBoard& self_candidates = board.get_candidates();
		Node* p = node.get_parent();
		if (!p) {
			auto b = p->get_board();
		}
		else {
			auto b = p->get_board();
		}
		const BitBoard& opponent_candidates = node.get_parent()->get_board().get_candidates();


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

		int score = score_candidates + 9 * score_fixed + 9 * score_corner_cands;

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

	MemorizedNegaAlphaAI(const double depth_ = 3) : AI(), depth(depth_) {};

	void load_board(const Board& board_) override {
		AI::load_board(board_);
		root = std::make_shared<Node>(board);
	}

	double eval() const override {
		return -root->get_eval();
	}

	void play(const Cell& move) override {
		AI::play(move);
		root = root->play(move);
	}

	Cell choose_move() override {
		std::chrono::system_clock::time_point  start, end;
		start = std::chrono::system_clock::now();

		if (root->get_children().empty()) {
			root->create_children();
		}

		//std::vector<std::thread> threads;

		cnt = 0;


		for (auto& child : root->get_children()) {
			//threads.emplace_back(std::thread(&MemorizedNegaAlphaAI::alpha_beta, std::ref(*child), depth, INT_MIN + 1, INT_MAX - 1));
			alpha_beta(*child, depth, INT_MIN + 1, INT_MAX - 1);
		}
		//for (auto& thd : threads)
		//{
		//	thd.join();
		//}

		std::cout << "count: " << cnt << std::endl;

		root->sort_children();
		const auto& children = root->get_children();
		for (auto& s : children) {
			std::cout << s->get_prev_move().to_string() << ": " << s->get_eval() << ", ";
		}
		std::cout << std::endl;
		auto& next = children.at(0);
		root->set_eval(-next->get_eval());

		end = std::chrono::system_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		return next->get_prev_move();
	}

	void pass() override {
		AI::pass();
		root = root->play(Cell::Pass());
	}
};