
#include <iostream>

#include "Board.hpp"

int main()
{
	BitBoard white = from_cells({ Cell("d4"), Cell("e5") });
	BitBoard black = from_cells({ Cell("d5"), Cell("e4") });
	Board board(white, black, BLACK);
	std::cout << board << std::endl;
}