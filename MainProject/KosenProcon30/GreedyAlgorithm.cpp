#include "Algorithm.hpp"

Procon30::SearchResult Procon30::GreedyAlgorithm::execute(const Game& game)
{
	agentsOrder order;

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else if(game.field.m_board.at(p).color == TeamColor::None)
			return Action::Move;
		else {
			if (game.field.m_board.at(p).score < 0) {
				return Action::Remove;
			}
			else {
				return Action::Move;
			}
		}
	};

	//位置がかぶった対策
	std::set<int32> s;

	for (int32 i = 0; i < game.teams.first.agentNum; i++) {
		int32 score = -10000;
		int32 state = 4;

		order.at(i).dir = Point(0,0);
		order.at(i).action = Action::Stay;

		//for文まわして、あれする
		s3d::Point dirs[8] = { {1,0},{-1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1} };
		//空マスへのMove > 相手マスへのRemove > 自陣マスへのRemove > 自陣マスへの移動
		for (int32 dirNum = 0; dirNum < 8; dirNum++) {

			if (InRange(game.teams.first.agents.at(i).nowPosition + dirs[dirNum])) {

				const Tile nextTile = game.field.m_board.at(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);

				if (s.find((game.teams.first.agents.at(i).nowPosition + dirs[dirNum]).x * 50 + (game.teams.first.agents.at(i).nowPosition + dirs[dirNum]).y)
					!= s.end())
					continue;

				switch (nextTile.color)
				{
				case TeamColor::None:

					if (state > 0) {
						state = 0;
						score = nextTile.score;
						order.at(i).dir = dirs[dirNum];
						order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
					}
					else if (state == 0) {
						if (score < nextTile.score) {
							score = nextTile.score;
							order.at(i).dir = dirs[dirNum];
							order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
						}
					}

					break;
				case TeamColor::Red:

					if (state > 1) {
						state = 1;
						score = nextTile.score;
						order.at(i).dir = dirs[dirNum];
						order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
					}
					else if (state == 1) {
						if (score < nextTile.score) {
							score = nextTile.score;
							order.at(i).dir = dirs[dirNum];
							order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
						}
					}

					break;
				case TeamColor::Blue:

					if (nextTile.score < 0) {

						if (state > 2) {
							state = 2;
							score = nextTile.score;
							order.at(i).dir = dirs[dirNum];
							order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
						}
						else if (state == 2) {
							if (score < nextTile.score) {
								score = nextTile.score;
								order.at(i).dir = dirs[dirNum];
								order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
							}
						}
					}
					else {
						if (state > 3) {
							state = 3;
							score = nextTile.score;
							order.at(i).dir = dirs[dirNum];
							order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
						}
						else if (state == 3) {
							if (score < nextTile.score) {
								score = nextTile.score;
								order.at(i).dir = dirs[dirNum];
								order.at(i).action = getAction(game.teams.first.agents.at(i).nowPosition + dirs[dirNum]);
							}
						}
					}

					break;
				default:
					break;
				}
			}
		}
		s.emplace((game.teams.first.agents.at(i).nowPosition + order.at(i).dir).x * 50 + (game.teams.first.agents.at(i).nowPosition + order.at(i).dir).y);
	}

	SearchResult result;

	result.code = AlgorithmStateCode::None;

	result.orders << order;

	return result;

}

void Procon30::GreedyAlgorithm::initilize([[maybe_unused]] const Game& game)
{
}