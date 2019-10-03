#include "TakahashiAlgorithm.hpp"

Procon30::YASAI::CompressBranch::CompressBranch(double weight)
	: weight(weight)
{
	distanceWeight[0] = 1;
	for (size_t i = 1; i < 21; i++) {
		distanceWeight[i] = distanceWeight[i - 1] / weight;
	}
}

void Procon30::YASAI::CompressBranch::initilize(const Game& game)
{
	fieldSize = game.field.boardSize;
	for (Point setPos : step(fieldSize)) {
		weightSum[setPos.y][setPos.x] = 0;
		for (Point readPos : step(fieldSize)) {
			size_t distance = Max(Abs(setPos.x - readPos.x), Abs(setPos.y - readPos.y));
			weightSum[setPos.y][setPos.x] += distanceWeight[distance];
		}
	}
}

bool Procon30::YASAI::CompressBranch::pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams)
{

	s3d::Point dirs[9] = { {0,0},{1,0},{-1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1} };

	std::array<std::pair<double, s3d::Point>, 9> sortedDirs;

	for (int agent_num = 0; agent_num < teams.first.agentNum; agent_num++) {
		const auto& agent = teams.first.agents[agent_num];
		for (int i = 0; i < 9; i++) {
			sortedDirs[i].first = weightSum[agent.nowPosition.y + dirs[i].y][agent.nowPosition.x + dirs[i].x];
			sortedDirs[i].second = dirs[i];
		}
		sort(sortedDirs.begin(), sortedDirs.end(),
			[](const std::pair<double, s3d::Point> left, const std::pair<double, s3d::Point> right) {return left.first < right.first; });

		for (int i = 8; i > 8 - canSimulateNum; i--) {
			enumerateDir[agent_num][8 - i] = sortedDirs[i].second;
		}
		enumerateDir[agent_num][canSimulateNum] = { -2,-2 };
	}
	return false;
}
