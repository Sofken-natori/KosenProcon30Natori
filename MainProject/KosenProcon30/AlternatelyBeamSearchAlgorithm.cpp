#include <queue>
#include <future>
#include "Algorithm/SuzukiAlgorithm.hpp"
#include "Algorithm/TakahashiAlgorithm.hpp"

Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::AlternatelyBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches) : BeamSearchAlgorithm(beamWidth, std::move(pruneBranches))
{
}

Procon30::SearchResult Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::execute(const Game& game)
{

	//����̓���𓾂�B
	Game gameCopy;
	gameCopy.field = game.field;
	gameCopy.teams = game.teams;
	std::swap(gameCopy.teams.first, gameCopy.teams.second);
	gameCopy.turn = game.turn;
	gameCopy.MaxTurn = game.MaxTurn;
	const auto& second_result = BeamSearchAlgorithm::execute(gameCopy);

	//�񋓂̃A���S�Ƃ��͕ύX�Ȃ��ł����Ǝv�����ǁA�]�����s���i�K�ŃV�~�����[�V���������Ă��̂����ŕ]������B
	constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	//�����萔�̓X�l�[�N�P�[�X�œ��ꋖ����

	const size_t beam_size = beamWidth;
	const size_t result_size = 3;
	const TeamColor my_team = TeamColor::Blue;
	const TeamColor enemy_team = TeamColor::Red;
	const int search_depth = std::min(10, game.MaxTurn - game.turn);
	const int was_moved_demerit = -5;
	const int wait_demerit = -10;
	const double diagonal_bonus = 1.5;
	const double fast_bonus = 1.03;
	const double enemy_peel_bonus = 0.9;
	const double my_area_merit = 0.4;
	const double enemy_area_merit = 0.8;
	const int minus_demerit = -2;
	const int mine_remove_demerit = -1;
	const double cancel_demerit = 0.9;

	//WAGNI:List�ւ̒u�������B�ǉ��͑����Ȃ�C������B


	//�����̏W���p�ɂ����Ŋm�ۂ���B
	//[�G�[�W�F���g�ԍ�][�����ԍ��i�I�[��-2�ɂ��Ă����āj] = ����;
	std::array<std::array<Point, 10>, 8> enumerateDir;

	//WAGNI:�����ɑؗ���聁�������B��������POST����ĂȂ������������B���̓��A���ԂŎ����Ő؂�悤��

	//���Z�q�̏���
	auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };

	std::vector<BeamSearchData> nowContainer;
	nowContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nowBeamBucketQueue(
		compare, std::move(nowContainer));

	std::vector<BeamSearchData> nextContainer;
	nextContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nextBeamBucketQueue(
		compare, std::move(nextContainer));

	//8�G�[�W�F���g�̏ꍇ 8^10=10^9���炢�ɂȂ肦��̂ł��܂��
	//5sec����15sec�炵���A
	//2^10=1024�ō�N�A1sec������
	//8^4=4096���炢�ɂ������B
	//�\�ȃV�~�����[�V�����萔�ꗗ�B
	//+1�͕��ʂɌ��ς����B
	//3���炢�܂ł͍�N�Ɠ����ł�����B
	const int canSimulationNums[9] = { 0,0,13,8,8,5,5,4,4 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucketQueue.push(first_state);

	for (int i = 0; i < search_depth; i++) {
		//enumerate
		while (!nowBeamBucketQueue.empty()) {
			BeamSearchData now_state = nowBeamBucketQueue.top();
			nowBeamBucketQueue.pop();

			//8^9�̓r�[���T�[�`�ł��v�Z�s�\�ɋ߂����Ȃ���
			//�}���T�����Ăяo���āA�����ł��������ɂ���B
			//�@�\�Ƃ��ẮAField��teams��^���邱�Ƃ�1000-10000�O��̕����̏W����Ԃ��B
			assert(pruneBranchesAlgorithm);

			pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);

			//bool okPrune = pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);
			//assert(okPrune);

			int32 next_dir[8] = {};

			bool enumerateLoop = true;
			while (enumerateLoop) {

				bool skip = false;

				//���ۂɈړ������Ă݂�B
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					now_state.teams.first.agents[agent_num].nextPosition
						= now_state.teams.first.agents[agent_num].nowPosition
						+ enumerateDir[agent_num][next_dir[agent_num]];

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

				//�����Ŏ��̈ړ��������X�V����B
				//9����������9�܂�
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					next_dir[agent_num]++;
					if (enumerateDir[agent_num][next_dir[agent_num]] == Point(-2, -2)) {
						next_dir[agent_num] = 0;
						if (agent_num == now_state.teams.first.agentNum - 1) {
							enumerateLoop = false;
							break;
						}
					}
					else {
						break;
					}
				}

				if (skip)
					continue;

				{
					//move : false , remove : true
					bool next_act[8] = {};
					bool actionLoop = true;

					while (actionLoop) {
						BeamSearchData next_state = now_state;

						bool mustCalcFirstScore = false;
						bool mustCalcSecondScore = false;

						//�V�~�����[�V�������Ă݂�B��΂����ɂ͗��_�I�ɂȂ�Ȃ�
						//WAGNI:�����Ă���ہA�ɂ�ݍ����̃��b�N�����̋@�\
						//�����F�̃}�C�i�X�_��move��remove����X�e�[�g�����B
						if (i == 0) {
							//i==0�̂Ƃ��A����Ƃ̏Փ˂��V�~�����[�V��������

							//���̂��߁A������action��ݒ肵�āA���̏㑊��̓����next_state�ɓ���Ēu��

							//first��agent��action�ɂ��Ă̐ݒ�
							for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {
								//nextPosition�ɍ��킹�āAaction���X�V���Ă���
								if (next_state.teams.first.agents[agent_num].nextPosition ==
									next_state.teams.first.agents[agent_num].nowPosition)
									next_state.teams.first.agents[agent_num].action = Action::Stay;

								switch (next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition).color) {
								case TeamColor::Blue://Move or Remove
									if (next_act[agent_num] == false) {
										next_state.teams.first.agents[agent_num].action = Action::Move;
									}
									else {
										next_state.teams.first.agents[agent_num].action = Action::Remove;
									}
									break;
								case TeamColor::Red://Remove
									next_state.teams.first.agents[agent_num].action = Action::Remove;
									break;
								case TeamColor::None://Move
									next_state.teams.first.agents[agent_num].action = Action::Move;
									break;
								}

								//i == 0��first�n������������
								next_state.first_act[agent_num] = next_state.teams.first.agents[agent_num].action;
								next_state.first_dir[agent_num] =
									next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition;


							}

							//second��agent��nextPosition,nowPosition,action�ɂ��Đݒ�B
							for (int agent_num = 0; agent_num < next_state.teams.second.agentNum; agent_num++) {

								//[n�Ԗڂ̂̑I����][�G�[�W�F���g�̔ԍ�]
								switch (second_result.orders[0][agent_num].action) {
								case Action::Move:
								case Action::Remove:
									next_state.teams.second.agents[agent_num].nextPosition
										= second_result.orders[0][agent_num].dir +
										next_state.teams.second.agents[agent_num].nowPosition;

									break;
								case Action::Stay:
									break;
								}
								next_state.teams.second.agents[agent_num].action =
									second_result.orders[0][agent_num].action;

							}

							//VirtualServer��simulation�𗬗p�B�����ŏ�������΂��点�����Ȃ̂Ŏd���Ȃ�
							//nextPosition��update�ŏ���������
							for (int loop = 0; loop < 16; loop++) {
								int flag[2][8] = {};
								bool firstDestroyedFlag[8] = {};
								bool secondDestroyedFlag[8] = {};
								int count = 0;

								//�ȉ����ڐA���Ă����B
								{//check
									const auto& team = next_state.teams.first;
									int agent_num = 0;
									for (const auto& agent : team.agents) {
										if (flag[0][agent_num] == 4) {
											continue;
										}
										else if (agent.action == Action::Stay) {
											flag[0][agent_num] = 1;
										}
										else if (agent.action == Action::Move || agent.action == Action::Remove) {
											const auto& f_team = next_state.teams.first;
											for (const auto& f_agent : f_team.agents) {
												if (agent.nowPosition == f_agent.nowPosition) {
													continue;
												}
												if (agent.nextPosition == f_agent.nextPosition) {
													//������ӂ͂��ꂼ��1�񂵂��ʂ�Ȃ�
													flag[0][agent_num] = 2;//�����̓������Ԃ�
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
													if (flag[0][agent_num] != 2)
														flag[0][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == f_agent.nowPosition) {
													flag[0][agent_num] = 2;//�����ɓ������Ԃ����
												}
												else {
													if (flag[0][agent_num] == 0)
														flag[0][agent_num] = 1;
												}
											}
											const auto& s_team = next_state.teams.second;
											for (const auto& s_agent : s_team.agents) {
												if (agent.nextPosition == s_agent.nextPosition) {
													flag[0][agent_num] = 2;//����Ɠ������Ԃ�����
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == s_agent.nowPosition && s_agent.action == Action::Move) {
													if (flag[0][agent_num] != 2)
														flag[0][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == s_agent.nowPosition) {
													flag[0][agent_num] = 2;//����ɓ������Ԃ����
													//�܂蓮���Ȃ����Ƃő���̓������Ԃ���
													if (s_agent.nextPosition != agent.nowPosition)
														firstDestroyedFlag[agent_num] = true;//���������̔��肪�o�O���Ă���͂��B

												}
												else {
													if (flag[0][agent_num] == 0)
														flag[0][agent_num] = 1;
												}
											}
										}
										agent_num++;
									}
								}
								{//check
									const auto& team = next_state.teams.second;
									int agent_num = 0;
									for (const auto& agent : team.agents) {
										if (flag[1][agent_num] == 4) {
											continue;
										}
										else if (agent.action == Action::Stay) {
											flag[1][agent_num] = 1;
										}
										else if (agent.action == Action::Move || agent.action == Action::Remove) {
											const auto& f_team = next_state.teams.first;
											for (const auto& f_agent : f_team.agents) {
												if (agent.nextPosition == f_agent.nextPosition) {
													flag[1][agent_num] = 2;//first�Ɠ������Ԃ�����
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
													if (flag[1][agent_num] != 2)
														flag[1][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == f_agent.nowPosition) {
													flag[1][agent_num] = 2;
													secondDestroyedFlag[agent_num] = true;//first�ɓ������Ԃ����
												}
												else {
													if (flag[1][agent_num] == 0)
														flag[1][agent_num] = 1;
												}
											}
											const auto& s_team = next_state.teams.second;
											for (const auto& s_agent : s_team.agents) {
												if (agent.nowPosition == s_agent.nowPosition) {
													continue;
												}
												if (agent.nextPosition == s_agent.nextPosition) {
													flag[1][agent_num] = 2;
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == s_agent.nowPosition && s_agent.action == Action::Move) {
													if (flag[1][agent_num] != 2)
														flag[1][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == s_agent.nowPosition) {
													flag[1][agent_num] = 2;
												}
												else {
													if (flag[1][agent_num] == 0)
														flag[1][agent_num] = 1;
												}
											}
										}
										agent_num++;
									}
								}
								{//update
									int agent_num = 0;
									auto& team = next_state.teams.first;
									for (auto& agent : team.agents) {
										if (flag[0][agent_num] == 0 || flag[0][agent_num] == 1) {
											switch (agent.action) {
											case Action::Move:
												if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::None) {

													const bool isDiagonal = (agent.nextPosition - agent.nowPosition).x != 0
														&& (agent.nextPosition - agent.nowPosition).y != 0;

													mustCalcFirstScore = true;

													if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
														next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score + minus_demerit) * pow(fast_bonus, search_depth - i);
													else {
														next_state.evaluatedScore += isDiagonal * diagonal_bonus;
														next_state.evaluatedScore += next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i);
													}

													agent.nowPosition = agent.nextPosition;
													next_state.field.m_board.at(agent.nextPosition).color = TeamColor::Blue;
												}
												else if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
													next_state.evaluatedScore += was_moved_demerit * pow(fast_bonus, search_depth - i);
													agent.nowPosition = agent.nextPosition;
												}
												break;
											case Action::Remove:

												if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
													if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
														next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i) + mine_remove_demerit) * enemy_peel_bonus;
													else
														next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
												}
												else {
													if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
														next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i) + minus_demerit) * enemy_peel_bonus;
													else
														next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i)) * enemy_peel_bonus;
												}

												next_state.field.m_board.at(agent.nextPosition).color = TeamColor::None;
												break;
											case Action::Stay:
												next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - i);
												break;
											}
										}
										else if (flag[0][agent_num] == 2) {
											//next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
											if (firstDestroyedFlag[agent_num]) {//�������Ԃ����
												next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - i);
											}
											else {//�������Ԃ�����
												if (next_state.teams.first.score > next_state.teams.second.score)
													next_state.evaluatedScore += 1.2 * cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i);
												else
													next_state.evaluatedScore += cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i);
											}
										}
										if (flag[0][agent_num] == 0 || flag[0][agent_num] == 1 || flag[0][agent_num] == 2) {
											agent.action = Action::Stay;
											agent.nextPosition = agent.nowPosition;
											flag[0][agent_num] = 4;
										}
										if (flag[0][agent_num] == 3)
											count++;
										agent_num++;
									}
								}
								{//update
									int agent_num = 0;
									auto& team = next_state.teams.second;
									for (auto& agent : team.agents) {
										if (flag[1][agent_num] == 0 || flag[1][agent_num] == 1) {
											switch (agent.action) {
											case Action::Move:
												if (next_state.field.m_board.at(agent.nextPosition).color != TeamColor::Blue) {
													agent.nowPosition = agent.nextPosition;
													next_state.field.m_board.at(agent.nextPosition).color = TeamColor::Red;
												}
												break;
											case Action::Remove:
												next_state.field.m_board.at(agent.nextPosition).color = TeamColor::None;
												break;
											case Action::Stay:
												break;
											}
										}
										if (flag[1][agent_num] == 2) {
											if (secondDestroyedFlag[agent_num]) {//���������܂��Ԃ���
												next_state.evaluatedScore -= wait_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - i);
											}
											else {//�������Ԃ�������

											}
										}
										if (flag[1][agent_num] == 0 || flag[1][agent_num] == 1 || flag[1][agent_num] == 2) {
											agent.action = Action::Stay;
											agent.nextPosition = agent.nowPosition;
											flag[1][agent_num] = 4;
										}

										if (flag[1][agent_num] == 3)
											count++;
										agent_num++;
									}
								}

								if (count == 0) {
									break;
								}
							}
							//�X�R�A�v�Z���Ȃ����B
							//�v��Ȃ�
							mustCalcFirstScore = true;
							mustCalcSecondScore = true;
						}
						else {

							//������������̎����B
							for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

								Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

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

										mustCalcFirstScore = true;

										if (targetTile.score <= 0)
											next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i) + mine_remove_demerit) * enemy_peel_bonus;
										else
											next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
									}
									break;
								case TeamColor::Red:
									targetTile.color = TeamColor::None;

									mustCalcSecondScore = true;

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

									mustCalcFirstScore = true;

									next_state.evaluatedScore += isDiagonal * diagonal_bonus;
									if (targetTile.score <= 0)
										next_state.evaluatedScore += (targetTile.score + minus_demerit) * pow(fast_bonus, search_depth - i);
									else
										next_state.evaluatedScore += targetTile.score * pow(fast_bonus, search_depth - i);

									break;
								}
							}
						}

						if (mustCalcFirstScore) {
							std::pair<int32, int32> s = this->calculateScoreFast(next_state.field, TeamColor::Blue);
							next_state.teams.first.tileScore = s.first;
							next_state.teams.first.areaScore = s.second;
							next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;
						}
						if (mustCalcSecondScore) {
							std::pair<int32, int32> s = this->calculateScoreFast(next_state.field, TeamColor::Red);
							next_state.teams.second.tileScore = s.first;
							next_state.teams.second.areaScore = s.second;
							next_state.teams.second.score = next_state.teams.second.tileScore + next_state.teams.second.areaScore;
						}

						next_state.evaluatedScore += (next_state.teams.first.areaScore - now_state.teams.first.areaScore) * my_area_merit +
							(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * pow(fast_bonus, search_depth - i);

						//�I���ł��񂾂�]���l���A�Q�[�����̂̏��s�Ɠ������Ȃ�悤�ɒ����B
						const double finish_slope = std::max(1 - (game.MaxTurn - game.turn - i) / 10.0, 0.0);

						next_state.evaluatedScore = (1 - finish_slope) * next_state.evaluatedScore + finish_slope * ((next_state.teams.first.score - next_state.teams.second.score));

						//��������������push����B
						if (nextBeamBucketQueue.size() > beam_size) {
							if (nextBeamBucketQueue.top().evaluatedScore < next_state.evaluatedScore) {
								nextBeamBucketQueue.pop();
								nextBeamBucketQueue.push((next_state));
							}
						}
						else {
							nextBeamBucketQueue.push((next_state));
						}

						//���̈ړ������ւ̍X�V�B�擪�ł��Ɠ����Ă����X�V����Ă������Șb�ɂȂ�B
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
					}
				}

			}
		}

		nowBeamBucketQueue.swap(nextBeamBucketQueue);

	}


	SearchResult result;

	result.code = AlgorithmStateCode::None;


	if (nowBeamBucketQueue.size() == 0) {
		assert(nowBeamBucketQueue.size() != 0);
	}
	else {

		Array<BeamSearchData> nowBeamBucket;

		while (!nowBeamBucketQueue.empty()) {
			nowBeamBucket << nowBeamBucketQueue.top();
			nowBeamBucketQueue.pop();
		}

		nowBeamBucket.reverse();

		int32 count = 0;
		for (int i = 0; i < nowBeamBucket.size() && count < result_size; i++) {
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
				count++;
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