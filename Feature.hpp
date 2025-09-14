#pragma once

#include "Board.hpp"

/**
Feature parameters

0:  is my turn
1:  the number of my total stone
2:  the number of my corner stone
3:  the number of my C stone
4:  the number of my X stone
5:  the number of my outer edge stone
6:  the number of my inner edge stone
7:  the number of opponent's total stone
8:  the number of opponent's corner stone
9:  the number of opponent's C stone
10: the number of opponent's X stone
11: the number of opponent's outer edge stone
12: the number of opponent's inner edge stone
13: the openness of my total stone
14: the openness of my corner stone
15: the openness of my C stone
16: the openness of my X stone
17: the openness of my outer edge stone
18: the openness of my inner edge stone
19: the openness of opponent's total stone
20: the openness of opponent's corner stone
21: the openness of opponent's C stone
22: the openness of opponent's X stone
23: the openness of opponent's outer edge stone
24: the openness of opponent's inner edge stone
25: the number of my total candidates
26: the number of my total fixed stone
27: the number of my corner candidates
28: the number of my corner fixed stone
29: the number of my C candidates
30: the number of my C fixed stone
31: the number of my X candidates
32: the number of my X fixed stone
33: the number of opponent's total candidates
34: the number of opponent's total fixed stone
35: the number of opponent's corner candidates
36: the number of opponent's corner fixed stone
37: the number of opponent's C candidates
38: the number of opponent's C fixed stone
39: the number of opponent's X candidates
40: the number of opponent's X fixed stone
41: the number of flip stone
42: the openness of flip stone
43: board data
.
.   my fixed stone: 1, my stone: 0.5, my candidate: 0.25, empty: 0, ...
.
106 : board data


total 107 parameters

output: #my stone - #opponent's stone when game is finished
*/

constexpr BitBoard CORNER_MASK = 0x8100000000000081LL;

constexpr BitBoard C_MASK = 0x4281000000004281LL;

constexpr BitBoard X_MASK = 0x0042000000004200LL;

constexpr BitBoard OUTER_EDGE_MASK = 0x3c0081818181003cLL;

constexpr BitBoard INNER_EDGE_MASK = 0x003c424242423c00LL;

constexpr size_t NUM_OF_FEATURES = 107;

inline double safe_div(const double a, const double b) {
	return (a == 0) ? 0.0 : a / b;
}

std::vector<double> get_feature_params(const Board& current, const Board& prev, const bool is_myturn) {

	const BitBoard& my_board = is_myturn ? current.get_self() : current.get_opponent();
	const BitBoard& opponent_board = is_myturn ? current.get_opponent() : current.get_self();

	const BitBoard empty = ~(my_board | opponent_board);

	const BitBoard& my_candidates = is_myturn ? current.get_candidates() : prev.get_candidates();
	const BitBoard& opponent_candidates = is_myturn ? prev.get_candidates() : current.get_candidates();

	const BitBoard flipped = current.get_opponent() ^ prev.get_self();

	BitBoard my_fixed = 0x0LL, opponent_fixed = 0x0LL;
	calculate_fixed_stones(my_board, opponent_board, my_fixed, opponent_fixed);

	const int num_my_fixed = count_stones(my_fixed);
	const int num_opponent_fixed = count_stones(opponent_fixed);
	const int num_my_candidates = count_stones(my_candidates);
	const int num_opponent_candidates = count_stones(opponent_candidates);

	std::vector<double> features;
	if (num_my_fixed > BOARD_AREA / 2 || num_opponent_fixed > BOARD_AREA / 2 ||
		((is_myturn ? num_my_candidates : num_opponent_candidates) == 0 && count_stones(current.pass().get_candidates()) == 0)) {
		features.push_back(count_stones(my_board));
		features.push_back(count_stones(opponent_board));
		features.push_back(num_my_candidates);
		features.push_back(num_opponent_candidates);
		features.push_back(num_my_fixed);
		features.push_back(num_opponent_fixed);
		return features;
	}
	features.reserve(NUM_OF_FEATURES);

	// 0: is my turn
	features.push_back(is_myturn ? 1.0 : -1.0);

	// 1: the number of my total stone
	features.push_back((double)count_stones(my_board) / 64.0);

	// 2: the number of my corner stone
	features.push_back((double)count_stones(my_board & CORNER_MASK) / 4.0);

	// 3: the number of my C stone
	features.push_back((double)count_stones(my_board & C_MASK) / 8.0);

	// 4: the number of my X stone
	features.push_back((double)count_stones(my_board & X_MASK) / 4.0);

	// 5: the number of my outer edge stone
	features.push_back((double)count_stones(my_board & OUTER_EDGE_MASK) / 16.0);

	// 6: the number of my inner edge stone
	features.push_back((double)count_stones(my_board & INNER_EDGE_MASK) / 16.0);

	// 7: the number of opponent's total stone
	features.push_back((double)count_stones(opponent_board) / 64.0);

	// 8: the number of opponent's corner stone
	features.push_back((double)count_stones(opponent_board & CORNER_MASK) / 4.0);

	// 9: the number of opponent's C stone
	features.push_back((double)count_stones(opponent_board & C_MASK) / 8.0);

	// 10: the number of opponent's X stone
	features.push_back((double)count_stones(opponent_board & X_MASK) / 4.0);

	// 11: the number of opponent's outer edge stone
	features.push_back((double)count_stones(opponent_board & OUTER_EDGE_MASK) / 16.0);

	// 12: the number of opponent's inner edge stone
	features.push_back((double)count_stones(opponent_board & INNER_EDGE_MASK) / 16.0);

	// 13: the openness of my total stone
	features.push_back(safe_div(openness(my_board, empty), 64 * (features.at(1) + features.at(7))));

	// 14: the openness of my corner stone
	features.push_back(safe_div(openness(my_board & CORNER_MASK, empty), 4 * (features.at(2) + features.at(8))));

	// 15: the openness of my C stone
	features.push_back(safe_div(openness(my_board & C_MASK, empty), 8 * (features.at(3) + features.at(9))));

	// 16: the openness of my X stone
	features.push_back(safe_div(openness(my_board & X_MASK, empty), 4 * (features.at(4) + features.at(10))));

	// 17: the openness of my outer edge stone
	features.push_back(safe_div(openness(my_board & OUTER_EDGE_MASK, empty), 16 * (features.at(5) + features.at(11))));

	// 18: the openness of my inner edge stone
	features.push_back(safe_div(openness(my_board & INNER_EDGE_MASK, empty), 16 * (features.at(6) + features.at(12))));

	// 19: the openness of opponent's total stone
	features.push_back(safe_div(openness(opponent_board, empty), 64 * (features.at(1) + features.at(7))));

	// 20: the openness of opponent's corner stone
	features.push_back(safe_div(openness(opponent_board & CORNER_MASK, empty), 4 * (features.at(2) + features.at(8))));

	// 21: the openness of opponent's C stone
	features.push_back(safe_div(openness(opponent_board & C_MASK, empty), 8 * (features.at(3) + features.at(9))));

	// 22: the openness of opponent's X stone
	features.push_back(safe_div(openness(opponent_board & X_MASK, empty), 4 * (features.at(4) + features.at(10))));

	// 23: the openness of opponent's outer edge stone
	features.push_back(safe_div(openness(opponent_board & OUTER_EDGE_MASK, empty), 16 * (features.at(5) + features.at(11))));

	// 24: the openness of opponent's inner edge stone
	features.push_back(safe_div(openness(opponent_board & INNER_EDGE_MASK, empty), 16 * (features.at(6) + features.at(12))));

	// 25: the number of my total candidates
	features.push_back((double)num_my_candidates / 64.0);

	// 26: the number of my total fixed stone
	features.push_back((double)num_my_fixed / 64.0);

	// 27: the number of my corner candidates
	features.push_back((double)count_stones(my_candidates & CORNER_MASK) / 4.0);

	// 28: the number of my corner fixed stone
	features.push_back((double)count_stones(my_fixed & CORNER_MASK) / 4.0);

	// 29: the number of my C candidates
	features.push_back((double)count_stones(my_candidates & C_MASK) / 8.0);

	// 30: the number of my C fixed stone
	features.push_back((double)count_stones(my_fixed & C_MASK) / 8.0);

	// 31: the number of my X candidates
	features.push_back((double)count_stones(my_candidates & X_MASK) / 4.0);

	// 32: the number of my X fixed stone
	features.push_back((double)count_stones(my_fixed & X_MASK) / 4.0);

	// 33: the number of opponent's total candidates
	features.push_back((double)num_opponent_candidates / 64.0);

	// 34: the number of opponent's total fixed stone
	features.push_back((double)num_opponent_fixed / 64.0);

	// 35: the number of opponent's corner candidates
	features.push_back((double)count_stones(opponent_candidates & CORNER_MASK) / 4.0);

	// 36: the number of opponent's corner fixed stone
	features.push_back((double)count_stones(opponent_fixed & CORNER_MASK) / 4.0);

	// 37: the number of opponent's C candidates
	features.push_back((double)count_stones(opponent_candidates & C_MASK) / 8.0);

	// 38: the number of opponent's C fixed stone
	features.push_back((double)count_stones(opponent_fixed & C_MASK) / 8.0);

	// 39: the number of opponent's X candidates
	features.push_back((double)count_stones(opponent_candidates & X_MASK) / 4.0);

	// 40: the number of opponent's X fixed stone
	features.push_back((double)count_stones(opponent_fixed & X_MASK) / 4.0);

	// 41: the number of flip stone
	features.push_back(((double)count_stones(flipped) / 64.0) * (is_myturn ? -1.0 : 1.0));

	// 42: the openness of flip stone
	features.push_back(safe_div(openness(flipped, empty), 64 * features.at(41)) * (is_myturn ? -1.0 : 1.0));

	// 43 to 106: board data
	BitBoard loc_mask = 0x1LL;
	for (size_t loc = 0; loc < BOARD_AREA; ++loc) {
		double score = 0.0;
		if (!is_empty(my_candidates & loc_mask)) {
			score += 0.25;
		}
		if (!is_empty(opponent_candidates & loc_mask)) {
			score += -0.25;
		}
		if (!is_empty(my_fixed & loc_mask)) {
			score += 1.0;
		}
		else if (!is_empty(my_board & loc_mask)) {
			score += 0.5;
		}
		else if (!is_empty(opponent_fixed & loc_mask)) {
			score -= 1.0;
		}
		else if (!is_empty(opponent_board & loc_mask)) {
			score -= 0.5;
		}
		features.push_back(score);
		loc_mask <<= 1;
	}

	return features;
}

inline double result_evaluation(const int my_stones, const int opponent_stones) {
	return 2.0 * (double)(my_stones * my_stones + (BOARD_AREA - opponent_stones - my_stones)) / (double)(BOARD_AREA * BOARD_AREA) - 1.0;
}