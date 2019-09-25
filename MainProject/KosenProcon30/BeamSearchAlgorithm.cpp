#include "Algorithm.hpp"

Procon30::BeamSearchAlgorithm::BeamSearchAlgorithm(int32 beamWidth)
{
	this->beamWidth = beamWidth;
}

Procon30::SearchResult Procon30::BeamSearchAlgorithm::execute(const Game& game)
{

	constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else
			return Action::Move;
	};

	auto convXY = [](const s3d::Point& p) {
		return p.x + p.y * 20;//�ő�}�b�v�傫��20
	};

	//�����萔�̓X�l�[�N�P�[�X�œ��ꋖ����

	const size_t beam_size = beamWidth;
	const size_t result_size = 3;
	const TeamColor my_team = TeamColor::Blue;
	const TeamColor enemy_team = TeamColor::Red;
	const int search_depth = std::min(10, game.MaxTurn - game.turn);
	const int same_demerit = -30;
	const int was_moved_demerit = -5;
	const int wait_demerit = -10;
	const double diagonal_bonus = 1.5;
	const double fast_bonus = 1.03;
	const double enemy_peel_bonus = 0.9;
	const double my_area_merit = 0.4;
	const double enemy_area_merit = 0.8;
	const int minus_demerit = -2;
	const int mine_remove_demerit = -1;

	//WAGNI:List�ւ̒u�������B�ǉ��͑����Ȃ�C������B

	//TODO:�x���A���������\�����Ȃ��̂�nextBeamBucket��push���Ȃ��悤�ɁA�܂�}�����������悤�B
	//WAGNI:�����ɑؗ���聁�������B��������POST����ĂȂ������������B���̓��A���ԂŎ����Ő؂�悤��

	Array<BeamSearchData> nowBeamBucket;
	nowBeamBucket.reserve(beam_size);
	Array<BeamSearchData> nextBeamBucket;
	nextBeamBucket.reserve(beam_size * 100000);
	//8�G�[�W�F���g�̏ꍇ 8^10=10^9���炢�ɂȂ肦��̂ł��܂��
	//5sec����15sec�炵���A
	//2^10=1024�ō�N�A1sec������
	//8^4=4096���炢�ɂ������B
	//�\�ȃV�~�����[�V�����萔�ꗗ�B
	//+1�͕��ʂɌ��ς����B
	//3���炢�܂ł͍�N�Ɠ����ł�����B
	const int canSimulationNum[9] = { 0,0,13,8,6,5,5,4,4 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucket << first_state;

	for (int i = 0; i < search_depth; i++) {
		//enumlate
		for (int k = 0; k < nowBeamBucket.size(); k++) {
			auto& now_state = nowBeamBucket[k];

			//8^9�̓r�[���T�[�`�ł��v�Z�s�\�ɋ߂����Ȃ���
			int32 next_dir[8] = {};

			bool enumlateLoop = true;
			while (enumlateLoop) {
				//9����������9�܂�
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					if (next_dir[agent_num] == 8) {
						next_dir[agent_num] = 0;
						if (agent_num == now_state.teams.first.agentNum - 1) {
							enumlateLoop = false;
							break;
						}
					}
					else {
						next_dir[agent_num]++;
						break;
					}
				}

				bool skip = false;

				//���ۂɈړ������Ă݂�B
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					now_state.teams.first.agents[agent_num].nextPosition
						= now_state.teams.first.agents[agent_num].nowPosition
						+ Point(dxy[next_dir[agent_num]], dxy[next_dir[agent_num] + 1]);

					if (!InRange(now_state.teams.first.agents[agent_num].nextPosition))
						skip = true;
				}

				//now next�̔�茟�o
				for (int agent_num1 = 0; agent_num1 < now_state.teams.first.agentNum; agent_num1++) {
					for (int agent_num2 = 0; agent_num2 < now_state.teams.first.agentNum; agent_num2++) {
						if (agent_num1 == agent_num2) {
							continue;
						}
						if (now_state.teams.first.agents[agent_num1].nowPosition == now_state.teams.first.agents[agent_num2].nextPosition) {
							skip = true;
							agent_num1 = agent_num2 = 10;
						}
					}
				}

				//next next�̔�茟�o
				for (int agent_num1 = 0; agent_num1 < now_state.teams.first.agentNum; agent_num1++) {
					for (int agent_num2 = 0; agent_num2 < now_state.teams.first.agentNum; agent_num2++) {
						if (agent_num1 == agent_num2) {
							continue;
						}
						if (now_state.teams.first.agents[agent_num1].nextPosition == now_state.teams.first.agents[agent_num2].nextPosition) {
							skip = true;
							agent_num1 = agent_num2 = 10;
						}
					}
				}

				if (skip || !enumlateLoop)
					continue;

				{
					//move : false , remove : true
					bool next_act[8] = {};
					bool actionLoop = true;

					while (actionLoop) {
						BeamSearchData next_state = now_state;

						//move or remove
						for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {
							if (next_act[agent_num] == true) {
								next_act[agent_num] = false;
							}
							else {
								if (next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition).color
									== next_state.teams.first.color
									&& next_state.teams.first.agents[agent_num].nowPosition != next_state.teams.first.agents[agent_num].nextPosition) {
									next_act[agent_num] = true;
									break;
								}
							}
							if (agent_num == next_state.teams.first.agentNum - 1) {
								actionLoop = false;
							}
						}

						//�V�~�����[�V�������Ă݂�B��΂����ɂ͗��_�I�ɂȂ�Ȃ�
						//WAGNI:�����Ă���ہA�ɂ�ݍ����̃��b�N�����̋@�\
						//�����F�̃}�C�i�X�_��move��remove����X�e�[�g�����B
						for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

							Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

							if (i == 0) {

								next_state.first_dir[agent_num] =
									next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition;

								switch (targetTile.color) {
								case TeamColor::Blue:
									next_state.first_act[agent_num] = next_act[agent_num] ? Action::Remove : Action::Move;
									break;
								case TeamColor::Red:
									next_state.first_act[agent_num] = Action::Remove;
									break;
								case TeamColor::None:
									next_state.first_act[agent_num] = Action::Move;
									break;
								}
							}

							//�t�B�[���h�ƃG�[�W�F���g�̈ʒu�X�V
							//�G�[�W�F���g�̎��ɍs���^�C���̐F
							switch (targetTile.color) {
							case TeamColor::Blue:
								if (!next_act[agent_num]) {//Move
									if (next_state.teams.first.agents[agent_num].nowPosition != next_state.teams.first.agents[agent_num].nextPosition) {//Moved
										next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;
										next_state.evaluatedScore += was_moved_demerit * pow(fast_bonus, search_depth - i);
									}
									else//Wait
										next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - i);
								}
								else {//Remove
									targetTile.color = TeamColor::None;

									next_state.teams.first.tileScore -= targetTile.score;
									next_state.teams.first.areaScore = this->calculateScore(next_state.field, TeamColor::Blue) - next_state.teams.first.tileScore;
									next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;

									if (targetTile.score <= 0)
										next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i) + mine_remove_demerit) * enemy_peel_bonus;
									else
										next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
								}
								break;
							case TeamColor::Red:
								targetTile.color = TeamColor::None;

								next_state.teams.second.tileScore -= targetTile.score;
								next_state.teams.second.areaScore = this->calculateScore(next_state.field, TeamColor::Red) - next_state.teams.second.tileScore;
								next_state.teams.second.score = next_state.teams.second.tileScore + next_state.teams.second.areaScore;

								if (targetTile.score <= 0)
									next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i) + minus_demerit) * enemy_peel_bonus;
								else
									next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i)) * enemy_peel_bonus;

								break;
							case TeamColor::None:
								const bool isDiagonal = (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).x != 0
									&& (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).y != 0;
								next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;

								targetTile.color = TeamColor::Blue;

								next_state.teams.first.tileScore += targetTile.score;
								next_state.teams.first.areaScore = this->calculateScore(next_state.field, TeamColor::Blue) - next_state.teams.first.tileScore;
								next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;

								next_state.evaluatedScore += isDiagonal * diagonal_bonus;
								if (targetTile.score <= 0)
									next_state.evaluatedScore += (targetTile.score + minus_demerit) * pow(fast_bonus, search_depth - i);
								else
									next_state.evaluatedScore += targetTile.score * pow(fast_bonus, search_depth - i);

								break;
							}
						}

						next_state.evaluatedScore += (next_state.teams.first.areaScore - now_state.teams.first.areaScore) * my_area_merit +
							(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * pow(fast_bonus, search_depth - i);

						//��������������push����B
						nextBeamBucket << std::move(next_state);
					}
				}

			}
		}
		//sort
		std::sort(nextBeamBucket.begin(), nextBeamBucket.end(), [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; });

		//TODO:�����ՖʂȂ�}���
		//TODO:�����G�[�W�F���g�ʒu�Ȃ�}���B
		nowBeamBucket.clear();

		//now�Ƀv�b�V��
		for (int k = 0; k < std::min(nextBeamBucket.size(), beam_size); k++) {
			nowBeamBucket << std::move(nextBeamBucket[k]);
		}

	}



	SearchResult result;

	result.code = AlgorithmStateCode::None;


	if (nowBeamBucket.size() == 0) {
		assert(nowBeamBucket.size() != 0);
	}
	else {

		for (int i = 0; i < std::min(nowBeamBucket.size(), result_size); i++) {
			const auto& now_state = nowBeamBucket[i];

			bool same = false;
			for (int k = 0; k < i; k++)
			{
				bool check = true;
				for (int m = 0; m < now_state.teams.first.agentNum; m++) {
					if (nowBeamBucket[k].first_dir[m] != now_state.first_dir[m] || nowBeamBucket[k].first_act[m] != now_state.first_act[m]) {
						check = false;
					}
				}
				if (check) {
					same = true;
				}
			}
			if (same) {
				continue;
			}

			{
				agentsOrder order;

				for (int32 k = 0; k < game.teams.first.agentNum; k++) {
					order[k].dir = now_state.first_dir[k];
					order[k].action = now_state.first_act[k];
				}

				result.orders << order;
			}

		}
	}

	return result;
}
