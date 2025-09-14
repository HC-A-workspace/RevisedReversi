#include <iostream>

#include "AI.hpp"
#include "MemorizedAlphaBetaAI.hpp"
#include "NegaAlphaAI.hpp"
#include "AlphaBetaAI.hpp"
#include "DLAlphaBetaAI.hpp"
#include "Game.hpp"
#include "reader.hpp"

#define GAME true

int main()
{
#if GAME
	Game game;
	std::unique_ptr<AI> black_ai = std::make_unique<AlphaBetaAI>();
	std::unique_ptr<AI> white_ai = std::make_unique<DLAlphaBetaAI>();
	game.play_on_console(std::move(black_ai), std::move(white_ai));
#else
	WthorTransformer transformer;
	transformer.execute();
#endif
}