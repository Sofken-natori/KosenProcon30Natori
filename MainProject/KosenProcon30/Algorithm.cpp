#include "Algorithm.hpp"

Procon30::SearchResult Procon30::RandAlgorithm::execute(const Game& game)
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

int32 Procon30::Algorithm::calculateScore(Field& field, TeamColor teamColor)
{
	int q_front = 0;
	int q_end = 0;

	const int& fieldSizeX = field.boardSize.x;
	const int& fieldSizeY = field.boardSize.y;

	assert(fieldSizeX > 0);
	assert(fieldSizeY > 0);

	for (auto y : step(fieldSizeY)) {
		q[q_end++] = Point{ 0, y };
		q[q_end++] = Point{ fieldSizeX - 1,y };
	}
	for (auto x : step(fieldSizeX)) {
		q[q_end++] = Point{ x , 0 };
		q[q_end++] = Point{ x , fieldSizeY - 1 };
	}

	while (q_front <= q_end) {

		auto now = q[q_front++];

		if (0 <= now.y && now.y < fieldSizeY && 0 <= now.x && now.x < fieldSizeX) {

			if (visit.at(now.y).at(now.x))
				continue;
			visit.at(now.y).at(now.x) = true;

			if (field.m_board.at(now.y, now.x).color != teamColor) {
				q[q_end++] = Point{ now.x - 1,now.y };
				q[q_end++] = Point{ now.x,now.y + 1 };
				q[q_end++] = Point{ now.x + 1,now.y };
				q[q_end++] = Point{ now.x,now.y - 1 };
			}

		}

	}


	int32 result = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (teamColor != field.m_board.at(y, x).color && visit.at(y).at(x) == false) {
				result += abs(field.m_board.at(y, x).score);
			}
			if (field.m_board.at(y, x).color == teamColor) {
				result += field.m_board.at(y, x).score;
			}
			visit.at(y).at(x) = false;
		}
	}

	return result;
}
