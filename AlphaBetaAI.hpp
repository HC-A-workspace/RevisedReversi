#pragma once

#include <memory>
#include <algorithm>

#include "AI.hpp"

class Node {
private:
	Board board;
	Node* parent = nullptr;
	Cell prev_move;
	int evaluation = 0;
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
				return a->evaluation > b->evaluation;
			});
	}

	~Node() = default;

	bool is_leaf() const {
		return children.empty();
	}

	int get_eval() const { return evaluation; }

	void set_eval(const int& eval) { evaluation = eval; }

	Board get_board() const { return board; }

	Node* get_parent() const { return parent; }

	Cell get_prev_move() const { return prev_move; }

	std::vector<std::shared_ptr<Node>> get_children() const { return children; }

	void create_children() {
		std::vector<Cell> candidates = board.get_candidate_list();
		if (candidates.empty()) {
			add_child(board.pass(), Cell::Pass());
			return;
		}
		for (auto& cell : candidates) {
			add_child(board.play(cell), cell);
		}
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
		str += std::to_string(evaluation);
		str += '\n';
		for (auto& child : children) {
			str += child->to_string(i + 1);
		}
		return str;
	}
};

class AlphaBetaAI : public AI {
private:
	int depth = 0;
	Node root;

	static int eval_leaf(const Board& board) {
		return count_stones(board.get_self()) - count_stones(board.get_opponent());
	}

	static void alpha_beta(Node& node, const int depth, int alpha, int beta) {
		const Board board = node.get_board();
		if (depth == 0) {
			node.set_eval(eval_leaf(board));
			return;
		}
		else if (board.finished()) {
			return;
		}
		else if (node.is_leaf()) {
			node.create_children();
		}

		std::vector<std::shared_ptr<Node>> children = node.get_children();

		int idx = 0;
		for (auto& child : node.get_children()) {
			alpha_beta(*child, depth - 1, -beta, -alpha);
			alpha = std::max(alpha, -child->get_eval());
			idx++;
			if (alpha >= beta) {
				break;
			}
		}
		node.remove_children(idx);

		node.sort_children();

		//std::cout << depth << " " << alpha << std::endl;

		node.set_eval(alpha);
	}
	
public:
	AlphaBetaAI(const int depth = 3) : AI(), depth(depth) {};

	void load_board(const Board& board_) override {
		board = board_;
		root = Node(board);
	}

	int eval() const override {
		auto child = root.get_children();
		auto& next = child.at(0);
		return next->get_eval();
	}

	void play(const Cell& move) override {
		root = *root.play(move).get();
		std::cout << root.to_string() << std::endl;
	}

	Cell choose_move() override {
		std::cout << "eval: " << root.get_eval() << std::endl;
		alpha_beta(root, depth, INT_MIN, INT_MAX);
		std::cout << root.to_string() << std::endl;
		auto child = root.get_children();
		auto& next = child.at(0);
		return next->get_prev_move();
	}
};