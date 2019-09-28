#include "Algorithm.hpp"


//enumerateDirは使いまわしているのでゴミ情報が入っている。
bool Procon30::PruneBranchesAlgorithm::pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams)
{
	//とりまその方向のタイルの値の合計、自分の色だと、0.3倍、相手の色だと1.1倍、マイナス値だと0.7倍、エージェントがいると0、
	//フィールド用の配列準備して、累積和して、区間取得すればいいか。

	constexpr double my_ratio = 0.3;
	constexpr double enemy_ratio = 1.1;
	constexpr double minus_ratio = 0.7;
	constexpr double agent_ratio = 0;

	std::array<std::array<double, 22>, 22> memo;
	std::array<std::array<double, 22>, 22> ruiseki;

	//そのエージェントから、2離れたところ中心の5*5四角形

	int32 fieldSizeY = field.boardSize.y;
	int32 fieldSizeX = field.boardSize.x;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {

			double score = field.m_board.at(y, x).score;

			if (field.m_board.at(y, x).color == teams.first.color) {
				score *= my_ratio;
			}
			else if (field.m_board.at(y, x).color == teams.second.color) {
				score *= enemy_ratio;
			}

			if (score < 0) {
				score *= -1 * minus_ratio;
			}
		}
	}

	for (auto y : step(fieldSizeY)) {
		ruiseki[y][0] = memo[y][0];
		for (auto x : step(fieldSizeX - 1)) {
			ruiseki[y][x + 1] = memo[y][x + 1] + ruiseki[y][x];
		}
	}

	for (auto x : step(fieldSizeX)) {
		for (auto y : step(fieldSizeY)) {
			ruiseki[y + 1][x] = ruiseki[y][x];
		}
	}

	//TODO:ここにコードを挿入してください。


	return false;
}
