#pragma once
#include "../KosenProcon30.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30 {

	class AlternatelyBeamSearchAlgorithm : public Algorithm {
	private:
		int32 beamWidth;
	public:
		AlternatelyBeamSearchAlgorithm(int32 beamWidth);
		SearchResult execute(const Game& game);
	};

}