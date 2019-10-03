#include "Algorithm.hpp"


//enumerateDir�͎g���܂킵�Ă���̂ŃS�~��񂪓����Ă���B
bool Procon30::PruneBranchesAlgorithm::pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams)
{
	//�Ƃ�܂��̕����̃^�C���̒l�̍��v�A�����̐F���ƁA0.3�{�A����̐F����1.1�{�A�}�C�i�X�l����0.7�{�A�G�[�W�F���g�������0�A
	//�t�B�[���h�p�̔z�񏀔����āA�ݐϘa���āA��Ԏ擾����΂������B

	constexpr double my_ratio = 0.3;
	constexpr double enemy_ratio = 1.1;
	constexpr double minus_ratio = 0.8;
	constexpr double agent_ratio = 0;
	const int section_size = 2;

	std::array<std::array<double, 22>, 22> memo;
	std::array<std::array<double, 22>, 22> ruiseki;

	//���̃G�[�W�F���g����A2���ꂽ�Ƃ��뒆�S��5*5�l�p�`

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

			memo[y][x] = score;

		}
	}

	for (auto& agent : teams.first.agents) {
		memo[agent.nowPosition.y][agent.nowPosition.x] = 0;
	}
	for (auto& agent : teams.second.agents) {
		memo[agent.nowPosition.y][agent.nowPosition.x] = 0;
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


	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < field.boardSize.x && 0 <= p.y && p.y < field.boardSize.y;
	};

	//���S�̍��W��^����ƁA�w�肵���T�C�Y�̋�Ԃ̍��v�����o����B
	auto getSectionSum = [&](s3d::Point p) {

		const s3d::Point leftUp = p + Point(section_size * -1 + -1, section_size * -1 + -1);
		const s3d::Point rightUp = p + Point(section_size, section_size * -1 + -1);
		const s3d::Point leftDown = p + Point(section_size * -1 + -1, section_size);
		const s3d::Point rightDown = p + Point(section_size, section_size);

		const double leftUpScore = InRange(leftUp) ? ruiseki[leftUp.y][leftUp.x] : 0;
		const double rightUpScore = InRange(rightUp) ? ruiseki[rightUp.y][rightUp.x] : 0;
		const double leftDownScore = InRange(leftDown) ? ruiseki[leftDown.y][leftDown.x] : 0;
		const double rightDownScore = InRange(rightDown) ? ruiseki[rightDown.y][rightDown.x] : 0;


		return leftUpScore + rightDownScore - leftDownScore - rightUpScore;
	};

	s3d::Point dirs[9] = { {0,0},{1,0},{-1,0},{0,-1},{0,1},{-1,-1},{-1,1},{1,-1},{1,1} };

	std::array<std::pair<double, s3d::Point>, 9> sortedDirs;

	for (int agent_num = 0; agent_num < teams.first.agentNum; agent_num++) {
		const auto& agent = teams.first.agents[agent_num];
		for (int i = 0; i < 9; i++) {
			sortedDirs[i].first = getSectionSum(agent.nowPosition + dirs[i] * section_size);
			sortedDirs[i].second = dirs[i];
		}
		sort(sortedDirs.begin(), sortedDirs.end(),
			[](const std::pair<double, s3d::Point> left, const std::pair<double, s3d::Point> right) {return left.first < right.first; });

		for (int i = 8; i > 8 - canSimulateNum; i--) {
			enumerateDir[agent_num][8 - i] = sortedDirs[i].second;
		}
		enumerateDir[agent_num][canSimulateNum] = { -2,-2 };
	}

	return true;
}

void Procon30::PruneBranchesAlgorithm::initilize(const Game& game)
{
}
