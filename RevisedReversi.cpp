#include <iostream>

#include "AI.hpp"
#include "MemorizedAlphaBetaAI.hpp"
#include "NegaAlphaAI.hpp"
#include "AlphaBetaAI.hpp"
#include "Game.hpp"
#include "reader.hpp"

#define GAME true

int main()
{
#if GAME
	Game game;
	std::unique_ptr<AI> black_ai = std::make_unique<AlphaBetaAI>();
	std::unique_ptr<AI> white_ai = std::make_unique<RandomAI>();
	game.play_on_console(std::move(black_ai), std::move(white_ai));
#else
	std::string INPUT_PASS = "C:\\Users\\cANDaYITP\\OneDrive\\ドキュメント\\WTHOR\\original\\";
	std::string OUTPUT_PASS = "C:\\Users\\cANDaYITP\\OneDrive\\ドキュメント\\WTHOR\\original\\";

	wthor_reader(INPUT_PASS + "WTH_2021.wtb");
#endif
}