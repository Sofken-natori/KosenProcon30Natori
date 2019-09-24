#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"

namespace Procon30 {

	enum class AlgorithmStateCode : uint32 {
		None,
		TimeOver,
		UnknownError
	};

	struct Order {
		Action action;
		s3d::Point dir;
	};

	using agentsOrder = std::array<Order, 8>;

	struct SearchResult {
		AlgorithmStateCode code;
		Array<agentsOrder> orders;
	};

	class Algorithm
	{
	public:
		virtual SearchResult execute(const Game& game) = 0;
	};

	class RandAlgorithm : public Algorithm {
		SearchResult execute(const Game& game);
	};

	class BeamSearchAlgorithm : public Algorithm {
		SearchResult execute(const Game& game);
	};

}