#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"

namespace Procon30 {

	enum class Code {
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
		Code code;
		Array<agentsOrder> orders;
	};

	class Algorithm
	{
	public:
		virtual SearchResult execute(const Game& game) = 0;
	};

	class RandAlgorithm : Algorithm {
		SearchResult execute(const Game& game);
	};

}