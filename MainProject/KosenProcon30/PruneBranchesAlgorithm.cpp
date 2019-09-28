#include "Algorithm.hpp"


//enumerateDir�͎g���܂킵�Ă���̂ŃS�~��񂪓����Ă���B
bool Procon30::PruneBranchesAlgorithm::pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams)
{
	//�Ƃ�܂��̕����̃^�C���̒l�̍��v�A�����̐F���ƁA0.3�{�A����̐F����1.1�{�A�}�C�i�X�l����0.7�{�A�G�[�W�F���g�������0�A
	//�t�B�[���h�p�̔z�񏀔����āA�ݐϘa���āA��Ԏ擾����΂������B

	constexpr double my_ratio = 0.3;
	constexpr double enemy_ratio = 1.1;
	constexpr double minus_ratio = 0.7;
	constexpr double agent_ratio = 0;

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

	//TODO:�����ɃR�[�h��}�����Ă��������B


	return false;
}
