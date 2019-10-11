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
}

bool Procon30::YASAI::CompressBranch::pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir,const Field& field,const std::pair<Team, Team>& teams) const
{
#define TILE(p) field.m_board[(p)]
	const Array<s3d::Point> dirs = { {0,0},{1,0},{-1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1} };

	std::array<std::pair<double, s3d::Point>, 9> sortedDirs;
	//これらはabs(score)*AreaWeight*distweight
	const double myAreaWeight = 0.6;
	const double enemyAreaWeight = 1.4;
	//Tileがminusの時(TileWeight - minusWeightDiff)にweightを変更するための調整変数
	const double minusWeightDiff = 1.3;
	//これらはscore*TileWeight* distweight
	const double myTileWeight = 0.5;
	const double enemyTileWeight = 1.2;
	//0点マスに対してネガティブな補正をかける
	//zeroPenarty * distWeight;
	const double zeroPenarty = -1.2;

	//0:none
	//1:tile
	//2:area
	const FieldFlag myTeamFlag = innerCalculateScoreFast(field, teams.first.color);
	const FieldFlag enemyTeamFlag = innerCalculateScoreFast(field, teams.second.color);

	auto evaluate = [&](const Agent& agent, const Point& pos, const size_t& index) {
		const size_t dist = Max(Abs((agent.nowPosition.x + sortedDirs[1].second.x) - pos.x), Abs((agent.nowPosition.y + sortedDirs[1].second.y) - pos.y));
		if (TILE(pos).score == 0) {
			sortedDirs[index].first += zeroPenarty * distanceWeight[dist];
			return;
		}
		//A==0 && B==0とどっちが速いのか...?
		if ((myTeamFlag[pos] * enemyTeamFlag[pos]) == 0) {
			sortedDirs[index].first += (double)(TILE(pos).score) * distanceWeight[dist];
		}
		if (myTeamFlag[pos] == 1) {
			if (TILE(pos).score < 0) {
				sortedDirs[index].first += (double)(TILE(pos).score) * distanceWeight[dist] * (myTileWeight - minusWeightDiff);
			}
			else {
				sortedDirs[index].first += (double)(TILE(pos).score) * distanceWeight[dist] * (myTileWeight);
			}
		}
		if (enemyTeamFlag[pos] == 1) {
			if (TILE(pos).score < 0) {
				sortedDirs[index].first += (double)(TILE(pos).score) * distanceWeight[dist] * (enemyTileWeight);
			}
			else {
				sortedDirs[index].first += (double)(TILE(pos).score) * distanceWeight[dist] * (enemyTileWeight);
			}
		}
		if (myTeamFlag[pos] == 2) {
			sortedDirs[index].first += (double)(Abs(TILE(pos).score)) * distanceWeight[dist] * (myAreaWeight);
		}
		if (enemyTeamFlag[pos] == 2) {
			sortedDirs[index].first += (double)(Abs(TILE(pos).score)) * distanceWeight[dist] * (enemyAreaWeight);
		}
		return;
	};

	for (int agent_num = 0; agent_num < teams.first.agentNum; agent_num++) {
		const auto& agent = teams.first.agents[agent_num];
		//stay
		sortedDirs[0].first = std::numeric_limits<double>::min();
		sortedDirs[0].second = { 0,0 };

		//右
		sortedDirs[1].first = 0;
		sortedDirs[1].second = { 1,0 };
		for (int32 x = 1; x < (fieldSize.x - agent.nowPosition.x); x++) {
			for (int32 y = Max(agent.nowPosition.y - (x - 1), 0); y < Min(agent.nowPosition.y + x, fieldSize.y); y++) {
				const Point nowPos = { x + agent.nowPosition.x,y };
				evaluate(agent, nowPos, 1);
			}
		}

		//左
		sortedDirs[2].first = 0;
		sortedDirs[2].second = { -1,0 };
		for (int32 x = -1; (agent.nowPosition.x + x) >= 0; x--) {
			for (int32 y = Max(agent.nowPosition.y + (x + 1), 0); y < Min(agent.nowPosition.y - x, fieldSize.y); y++) {
				const Point nowPos = { x + agent.nowPosition.x,y };
				evaluate(agent, nowPos, 2);
			}
		}

		//上
		sortedDirs[3].first = 0;
		sortedDirs[3].second = { 0,-1 };
		for (int32 y = -1; (agent.nowPosition.y + y) >= 0; y--) {
			for (int32 x = Max(agent.nowPosition.x + (y + 1), 0); x < Min(agent.nowPosition.x - y, fieldSize.y); x++) {
				const Point nowPos = { x,y + agent.nowPosition.y };
				evaluate(agent, nowPos, 3);
			}
		}

		//下
		sortedDirs[4].first = 0;
		sortedDirs[4].second = { 0,1 };
		for (int32 y = 1; y < (fieldSize.y - agent.nowPosition.y); y++) {
			for (int32 x = Max(agent.nowPosition.x - (y - 1), 0); x < Min(agent.nowPosition.x + y, fieldSize.y); x++) {
				const Point nowPos = { x,y + agent.nowPosition.y };
				evaluate(agent, nowPos, 4);
			}
		}

		//左上
		sortedDirs[5].first = 0;
		sortedDirs[5].second = { -1,-1 };
		for (int32 x = 0; x < agent.nowPosition.x; x++) {
			for (int32 y = 0; y < agent.nowPosition.y; y++) {
				evaluate(agent, Point(x, y), 5);
			}
		}

		//右上
		sortedDirs[6].first = 0;
		sortedDirs[6].second = { 1,-1 };
		for (int32 x = agent.nowPosition.x + 1; x < fieldSize.x; x++) {
			for (int32 y = 0; y < agent.nowPosition.y; y++) {
				evaluate(agent, Point(x, y), 6);
			}
		}

		//左下
		sortedDirs[7].first = 0;
		sortedDirs[7].second = { -1,1 };
		for (int32 x = 0; x < agent.nowPosition.x; x++) {
			for (int32 y = agent.nowPosition.y + 1; y < fieldSize.y; y++) {
				evaluate(agent, Point(x, y), 7);
			}
		}

		//右下
		sortedDirs[8].first = 0;
		sortedDirs[8].second = { 1,1 };
		for (int32 x = agent.nowPosition.x+1; x < fieldSize.x; x++) {
			for (int32 y = agent.nowPosition.y+1; y < fieldSize.y; y++) {
				evaluate(agent, Point(x, y), 8);
			}
		}
		sortedDirs[5].first *= 1.15;
		sortedDirs[6].first *= 1.15;
		sortedDirs[7].first *= 1.15;
		sortedDirs[8].first *= 1.15;

		sort(sortedDirs.begin(), sortedDirs.end(),
			[](const std::pair<double, s3d::Point> left, const std::pair<double, s3d::Point> right) {return left.first > right.first; });

		for (int i = 0; i < canSimulateNum; i++) {
			enumerateDir[agent_num][i] = sortedDirs[i].second;
		}
		enumerateDir[agent_num][canSimulateNum] = { -2,-2 };
	}
	return false;
}

Procon30::YASAI::CompressBranch::FieldFlag Procon30::YASAI::CompressBranch::innerCalculateScoreFast(const Procon30::Field& field, Procon30::TeamColor teamColor) const
{
	unsigned short qFast[2000];
	std::bitset<1023> visitFast, isTeamColorFast;
	FieldFlag flagGrid(fieldSize, 0);
	//short is 16bit and 2byte.
#define XY_TO_SHORT(x,y) ((((x) & 31) << 5) | ((y) & 31))
#define SHORT_TO_X(c) (((c) >> 5) & 31)
#define SHORT_TO_Y(c) ((c) & 31)

	int32 q_front = 0;
	int32 q_end = 0;

	const int& fieldSizeX = field.boardSize.x;
	const int& fieldSizeY = field.boardSize.y;

	assert(fieldSizeX > 0);
	assert(fieldSizeY > 0);

	for (auto y : step(fieldSizeY)) {
		qFast[q_end++] = XY_TO_SHORT(0, y);
		qFast[q_end++] = XY_TO_SHORT(fieldSizeX - 1, y);
	}
	for (auto x : step(fieldSizeX)) {
		qFast[q_end++] = XY_TO_SHORT(x, 0);
		qFast[q_end++] = XY_TO_SHORT(x, fieldSizeY - 1);
	}

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			isTeamColorFast[XY_TO_SHORT(x, y)] = field.m_board.at(y, x).color != teamColor;
		}
	}

	while (q_front < q_end) {

		const auto& now = qFast[q_front++];

		if (!visitFast[now]) {

			visitFast[now] = true;

			if (isTeamColorFast[now]) {
				if (SHORT_TO_X(now) != 0)
					qFast[q_end++] = now - (1 << 5);
				//qFast[q_end++] = XY_TO_SHORT(SHORT_TO_X(now) - 1, SHORT_TO_Y(now));
				if (SHORT_TO_Y(now) + 1 < fieldSizeY)
					qFast[q_end++] = now + 1;
				//qFast[q_end++] = XY_TO_SHORT(SHORT_TO_X(now), SHORT_TO_Y(now) + 1);
				if (SHORT_TO_X(now) + 1 < fieldSizeX)
					qFast[q_end++] = now + (1 << 5);
				//qFast[q_end++] = XY_TO_SHORT(SHORT_TO_X(now) + 1, SHORT_TO_Y(now));
				if (SHORT_TO_Y(now) != 0)
					qFast[q_end++] = now - 1;
				//qFast[q_end++] = XY_TO_SHORT(SHORT_TO_X(now), SHORT_TO_Y(now) - 1);
			}

			assert(q_end <= 2000);
		}

	}
	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (field.m_board.at(y, x).color == teamColor) {
				flagGrid[y][x] = 1U;
			}
			else if (visitFast[XY_TO_SHORT(x, y)] == false) {
				flagGrid[y][x] = 2U;
			}
		}
	}

	visitFast.reset();

	return flagGrid;
}