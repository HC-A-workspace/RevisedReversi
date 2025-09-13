#pragma once

#include "Board.hpp"

/**
Feature parameters

1 parameter
is my turn? {-1, 1}

2 * 2 * 6 = 24 parameters
(the number | the openness) of (my | opponent's)
	(total | corner | c | x | outer egde | inner edge) stones

2 * 4 * 2 = 16 parameters
the number of (my | opponent's)
	(total | corner | c | x) (candidates | fixed stone)

2 parameters
(the number | the openness) of flipped stones

4 parameters
has (my | opponent's) (corner candidates | fixed stones)? {0, 1}

2 parameters
(my | opponent's) definitely lose or win {-1, 0, 1}

total 39 parameters

output: #my stone - #opponent's stone when game is finished
*/

constexpr BitBoard CORNER_MASK = 0x8100000000000081LL;

constexpr BitBoard C_MASK = 0x4281000000004281LL;

constexpr BitBoard X_MASK = 0x0042000000004200LL;

constexpr BitBoard OUTER_EDGE_MASK = 0x3c0081818181003cLL;

constexpr BitBoard INNER_EDGE_MASK = 0x003c424242423c00LL;

constexpr size_t NUM_OF_FEATURES = 81;


