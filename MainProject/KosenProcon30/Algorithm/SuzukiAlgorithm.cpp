#include "SuzukiAlgorithm.hpp"

Procon30::AlternatelyBeamSearchAlgorithm::AlternatelyBeamSearchAlgorithm(int32 beamWidth)
{
}

Procon30::SearchResult Procon30::AlternatelyBeamSearchAlgorithm::execute(const Game& game)
{
	return SearchResult();
}

/*
	memo

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else
			return Action::Move;
	};

	auto convXY = [](const s3d::Point& p) {
		return p.x + p.y * 20;//ç≈ëÂÉ}ÉbÉvëÂÇ´Ç≥20
	};

*/