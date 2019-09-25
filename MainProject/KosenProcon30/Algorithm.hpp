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
	private:
		int q_front = 0;
		int q_end = 0;
		std::array<std::array<bool, 22>, 22> visit = {};
		s3d::Point q[2000] = {};
	public:
		int32 calculateScore(Field& field, TeamColor teamColor);
		virtual SearchResult execute(const Game& game) = 0;
	};

	class RandAlgorithm : public Algorithm {
		SearchResult execute(const Game& game);
	};

	class BeamSearchAlgorithm : public Algorithm {
	private:
		int32 beamWidth;
	public:
		struct BeamSearchData {
			double evaluatedScore;
			Point first_dir[8] = {};
			Action first_act[8] = {};
			std::pair<Team, Team> teams;
			Field field;
		};
		BeamSearchAlgorithm(int32 beamWidth);
		SearchResult execute(const Game& game);
	};

}