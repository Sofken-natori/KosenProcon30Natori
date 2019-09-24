#include "Algorithm.hpp"

Procon30::SearchResult Procon30::BeamSearchAlgorithm::execute(const Game& game)
{
	agentsOrder order;

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else
			return Action::Move;
	};


	for (int32 i = 0; i < game.teams.first.agentNum; i++) {
		int count = 0;
		while (true) {
			Point rand = RandomPoint(s3d::Rect(3, 3));
			rand -= Point(1, 1);
			if (InRange(game.teams.first.agents.at(i).nowPosition + rand) && rand != Point(0, 0)) {
				order.at(i).dir = rand;
				order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + rand);
				break;
			}
			count++;
			if (count > 100) {
				order.at(i).dir = Point(0, 0);
				order.at(i).action = Action::Stay;
				break;
			}
		}
	}

	SearchResult result;

	result.code = AlgorithmStateCode::None;

	result.orders << order;

	return result;
}
