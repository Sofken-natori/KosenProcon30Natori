#include <queue>
#include <future>
#include <cmath>
#include "Agent.hpp"
#include "Algorithm.hpp"
#include "Algorithm/SuzukiAlgorithm.hpp"

Procon30::PrivateAlgorithm::PrivateAlgorithm(int32 beamWidth, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms, std::unique_ptr<Algorithm> secondAlgorithm)
	: m_beamWidth(beamWidth), pruneBranchesAlgorithms(std::move(PBAlgorithms)), secondBeamSearchAlgorithm(std::move(secondAlgorithm))
{
}

void Procon30::PrivateAlgorithm::initilize(const Game& game)
{
	if (isInitilized)return;
	isInitilized = true;
	for (int32 i = 0; i < parallelSize; i++)
		this->pruneBranchesAlgorithms[i]->initilize(game);
}

Procon30::SearchResult Procon30::PrivateAlgorithm::execute(const Game& game)
{
	using BeamSearchData = Procon30::BeamSearchAlgorithm::BeamSearchData;

	//TODO:���݃r�[���T�[�`��ǉ�����

	//����̓���𓾂�B
	Game gameCopy;
	gameCopy.field = game.field;
	gameCopy.teams = game.teams;
	std::swap(gameCopy.teams.first, gameCopy.teams.second);
	gameCopy.turn = game.turn;
	gameCopy.MaxTurn = game.MaxTurn;
	//TODO:���݃r�[���T�[�`�̎������`�[���Ɉˑ����Ȃ��悤�ɂ���B
	const auto& second_result = secondBeamSearchAlgorithm->execute(gameCopy);

	//�����萔�̓X�l�[�N�P�[�X�œ��ꋖ����

	//�����Œ�������
	//�Ƃ肠�����b��Ń}�b�v�̍L���ƒT�����ԂɊ֌W�͂Ȃ����߁A�icalcScore�̉e���͖���j
	//�r�[�����́A�G�[�W�F���g���Ō��߂܂��B

	//constexpr int32 canBeamWidths[9] = { 0,0,100,100,100, 100, 100, 100, 100 };

	//beamWidth = canBeamWidths[game.teams.first.agentNum];

	const size_t beam_size = m_beamWidth;

	constexpr size_t result_size = 3;
	constexpr TeamColor my_team = TeamColor::Blue;
	constexpr TeamColor enemy_team = TeamColor::Red;
	const int32 search_depth = std::min(10, game.MaxTurn - game.turn);
	const int32 MaxTurn = game.MaxTurn;
	const int32 turn = game.turn;
	const int32 field_width = game.field.boardSize.x;
	const int32 field_height = game.field.boardSize.y;
	constexpr double same_location_demerit = 0.7;
	constexpr double same_area_demerit = 0.7;
	constexpr int was_moved_demerit = -5;
	constexpr int wait_demerit = -10;
	constexpr double diagonal_bonus = 1.5;
	constexpr double fast_bonus = 1.03;
	constexpr double enemy_peel_bonus = 0.9;
	constexpr double my_area_merit = 0.4;
	constexpr double enemy_area_merit = 0.8;
	constexpr int minus_demerit = -2;
	constexpr int mine_remove_demerit = -1;
	constexpr int32 timeMargin = 1000;
	constexpr int32 parallelSize = 3;
	constexpr double cancel_demerit = 0.9;

	//TODO:�^�[�����i�߂ΐi�ނقǎ��ۂ̕]���Ɠ����悤�ɂȂ�悤�ɂ���B
	//���Z�q�̏���
	//�����������炵������operator���I�[�o�[���C�h���Ď���
	//auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };

	int32 beforeOneDepthSearchEnd = game.turnTimer.ms();
	int32 nowSearchDepth;

	Array<BeamSearchData> nowBeamBucketArray;

	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nowBeamBucketQueues[parallelSize];

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	//nowBeamBucketArray.push_back(first_state);
	nowBeamBucketQueues[0].push(first_state);
	/*
		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			nowBeamBucketQueues[parallelNum] = std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>>();
		}
	*/
	//�b���ɍ��킹�Čv�Z�ł��؂�H�v�ǉ��B�Ƃ肠�������ꂾ������Γ����Ȃ����Ƃ͂Ȃ��B
	//WAGNI:�]�������Ԃɍ��킹�ăr�[�����𒲐�����H�v�H
	for (nowSearchDepth = 0; nowSearchDepth < search_depth; nowSearchDepth++) {

		if (nowSearchDepth != 0) {
			beforeOneDepthSearchEnd = game.turnTimer.ms() - beforeOneDepthSearchEnd;
			if ((game.turnMillis - timeMargin) < game.turnTimer.ms() + beforeOneDepthSearchEnd) {
				break;
			}
		}

		//���̃��[�v����񉻂���B�����A
		std::future<std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>>> beamSearchFuture[parallelSize];

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			//CATION:second_result�̃R�s�[���ł��Ă��邩���m�F���邱�ƁB
			beamSearchFuture[parallelNum] = std::async(std::launch::async, [nowSearchDepth, search_depth, beam_size, field_width, field_height, cancel_demerit,
				MaxTurn,turn,second_result](
				std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nowBeamBucketQueue, std::unique_ptr<PruneBranchesAlgorithm>& pruneBranches) {

					std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nextBeamBucketQueue;

					std::bitset<1023> visitFast = {};
					std::bitset<1023> isTeamColorFast = {};
					unsigned short qFast[2000] = {};

					//�����̏W���p�ɂ����Ŋm�ۂ���B
					//[�G�[�W�F���g�ԍ�][�����ԍ��i�I�[��-2�ɂ��Ă����āj] = ����;
					std::array<std::array<Point, 10>, 8> enumerateDir;

					constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

					auto InRange = [&](s3d::Point p) {
						return 0 <= p.x && p.x < field_width && 0 <= p.y && p.y < field_height;
					};

					//enumerate
					while (!nowBeamBucketQueue.empty()) {

						BeamSearchData now_state = std::move(nowBeamBucketQueue.top());
						nowBeamBucketQueue.pop();

						//8^9�̓r�[���T�[�`�ł��v�Z�s�\�ɋ߂����Ȃ���
						//�}���T�����Ăяo���āA�����ł��������ɂ���B
						//�@�\�Ƃ��ẮAField��teams��^���邱�Ƃ�1000-10000�O��̕����̏W����Ԃ��B
						//assert(pruneBranchesAlgorithm);

						//8�G�[�W�F���g�̏ꍇ 8^10=10^9���炢�ɂȂ肦��̂ł��܂��
						//5sec����15sec�炵���A
						//2^10=1024�ō�N�A1sec������
						//8^4=4096���炢�ɂ������B
						//�\�ȃV�~�����[�V�����萔�ꗗ�B
						//+1�͕��ʂɌ��ς����B
						//3���炢�܂ł͍�N�Ɠ����ł�����B
						//TODO:�A���S���Y���ł܂��Ă���v�Z�ʌ��r�[�����̒������܂����B
						const int32 canSimulationNums[9] = { 0,0,9,8,8,5,4,3,3 };

						pruneBranches->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);

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
									if (nowSearchDepth == 0) {
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
																	next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score + minus_demerit) * pow(fast_bonus, search_depth - agent_num);
																else {
																	next_state.evaluatedScore += isDiagonal * diagonal_bonus;
																	next_state.evaluatedScore += next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num);
																}

																agent.nowPosition = agent.nextPosition;
																next_state.field.m_board.at(agent.nextPosition).color = TeamColor::Blue;
															}
															else if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
																next_state.evaluatedScore += was_moved_demerit * pow(fast_bonus, search_depth - agent_num);
																agent.nowPosition = agent.nextPosition;
															}
															break;
														case Action::Remove:

															if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
																if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
																	next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num) + mine_remove_demerit) * enemy_peel_bonus;
																else
																	next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
															}
															else {
																if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
																	next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num) + minus_demerit) * enemy_peel_bonus;
																else
																	next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num)) * enemy_peel_bonus;
															}

															next_state.field.m_board.at(agent.nextPosition).color = TeamColor::None;
															break;
														case Action::Stay:
															next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
															break;
														}
													}
													else if (flag[0][agent_num] == 2) {
														//next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
														if (firstDestroyedFlag[agent_num]) {//�������Ԃ����
															next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
														}
														else {//�������Ԃ�����
															if (next_state.teams.first.score > next_state.teams.second.score)
																next_state.evaluatedScore += 1.2 * cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num);
															else
																next_state.evaluatedScore += cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num);
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
															next_state.evaluatedScore -= wait_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num);
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

										for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

											Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

											/*
											if (nowSearchDepth == 0) {

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
											*/

											//�t�B�[���h�ƃG�[�W�F���g�̈ʒu�X�V
											//�G�[�W�F���g�̎��ɍs���^�C���̐F
											switch (targetTile.color) {
											case TeamColor::Blue:
												if (!next_act[agent_num]) {//Move
													if (next_state.teams.first.agents[agent_num].nowPosition != next_state.teams.first.agents[agent_num].nextPosition) {//Moved
														next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;
														next_state.evaluatedScore += was_moved_demerit * pow(fast_bonus, search_depth - nowSearchDepth);
													}
													else//Wait
														next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - nowSearchDepth);
												}
												else {//Remove
													targetTile.color = TeamColor::None;

													{
														//�����e2�����ȏ゠������
														int tileCount = 0;

														for (int direction = 0; direction < 9; direction++) {
															if (dxy[direction] == 0 && dxy[direction + 1] == 0)
																continue;
															if (InRange(Point(next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
																next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1])) &&
																next_state.field.m_board.at(
																	next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
																	next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1]).
																color == next_state.teams.first.color)
																tileCount++;
														}

														if (tileCount >= 2)
															mustCalcFirstScore = true;
														else {
															next_state.teams.first.tileScore -= targetTile.score;
															next_state.teams.first.areaScore -= targetTile.score;
														}
													}

													if (targetTile.score <= 0)
														next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - nowSearchDepth) + mine_remove_demerit) * enemy_peel_bonus;
													else
														next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
												}
												break;
											case TeamColor::Red:
												targetTile.color = TeamColor::None;

												{
													//�Ԃ����e2�����ȏ゠������
													int tileCount = 0;

													for (int direction = 0; direction < 9; direction++) {
														if (dxy[direction] == 0 && dxy[direction + 1] == 0)
															continue;
														if (InRange(Point(next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
															next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1])) &&
															next_state.field.m_board.at(
																next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
																next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1]).
															color == next_state.teams.second.color)
															tileCount++;
													}

													if (tileCount >= 2)
														mustCalcSecondScore = true;
													else {
														next_state.teams.second.tileScore -= targetTile.score;
														next_state.teams.second.areaScore -= targetTile.score;
													}
												}

												if (targetTile.score <= 0)
													next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - nowSearchDepth) + minus_demerit) * enemy_peel_bonus;
												else
													next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - nowSearchDepth)) * enemy_peel_bonus;

												break;
											case TeamColor::None:
												const bool isDiagonal = (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).x != 0
													&& (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).y != 0;
												next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;

												targetTile.color = next_state.teams.first.color;
												{
													//�����e2�����ȏ゠������
													int tileCount = 0;

													for (int direction = 0; direction < 9; direction++) {
														if (dxy[direction] == 0 && dxy[direction + 1] == 0)
															continue;
														if (InRange(Point(next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
															next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1])) &&
															next_state.field.m_board.at(
																next_state.teams.first.agents[agent_num].nextPosition.y + dxy[direction],
																next_state.teams.first.agents[agent_num].nextPosition.x + dxy[direction + 1]).
															color == next_state.teams.first.color)
															tileCount++;
													}

													if (tileCount >= 2)
														mustCalcFirstScore = true;
													else {
														next_state.teams.first.tileScore -= targetTile.score;
														next_state.teams.first.areaScore -= targetTile.score;
													}
												}

												next_state.evaluatedScore += isDiagonal * diagonal_bonus;
												if (targetTile.score <= 0)
													next_state.evaluatedScore += (targetTile.score + minus_demerit) * pow(fast_bonus, search_depth - nowSearchDepth);
												else
													next_state.evaluatedScore += targetTile.score * pow(fast_bonus, search_depth - nowSearchDepth);

												break;
											}
										}
									}

									if (mustCalcFirstScore) {
										std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, TeamColor::Blue, qFast, visitFast, isTeamColorFast);
										next_state.teams.first.tileScore = s.first;
										next_state.teams.first.areaScore = s.second;
										next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;
									}
									if (mustCalcSecondScore) {
										std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, TeamColor::Red, qFast, visitFast, isTeamColorFast);
										next_state.teams.second.tileScore = s.first;
										next_state.teams.second.areaScore = s.second;
										next_state.teams.second.score = next_state.teams.second.tileScore + next_state.teams.second.areaScore;
									}

									next_state.evaluatedScore += (next_state.teams.first.areaScore - now_state.teams.first.areaScore) * my_area_merit +
										(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * pow(fast_bonus, search_depth - nowSearchDepth);


									//�I���ł��񂾂�]���l���A�Q�[�����̂̏��s�Ɠ������Ȃ�悤�ɒ����B
									const double finish_slope = std::max(1 - (MaxTurn - turn - nowSearchDepth) / 10.0, 0.0);

									next_state.evaluatedScore = (1 - finish_slope) * next_state.evaluatedScore + finish_slope * ((next_state.teams.first.score - next_state.teams.second.score));


									//��������������push����B
									if (nextBeamBucketQueue.size() > beam_size) {
										if (nextBeamBucketQueue.top().evaluatedScore < next_state.evaluatedScore) {
											nextBeamBucketQueue.pop();
											nextBeamBucketQueue.push(std::move(next_state));
										}
									}
									else {
										nextBeamBucketQueue.push(std::move(next_state));
									}

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

					return nextBeamBucketQueue;
				}
			, nowBeamBucketQueues[parallelNum], ref(pruneBranchesAlgorithms[parallelNum]));
		}

		nowBeamBucketArray.clear();

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> result = beamSearchFuture[parallelNum].get();

			//pop����B
			while (!result.empty()) {
				nowBeamBucketArray.push_back(std::move(result.top()));
				result.pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end());


		//������Ԃ�΂点�Ȃ��H�v�B���N���Q�l�ɂ��āA�ł��ΐ킳���Ȃ��炩�ȁB�����܂łŃ��C����BeamSearch������̈�U�I������

		//�ŏ��̕������ꏏ�ȏꍇ
		//���݂̐�L���Ă���}�b�v���ꏏ�Ȃ猸�_������
		//�������́A���݂̈ʒu���ꏏ�Ȃ猸�_

		//stack����B
		//���_����(�Ƃ肠����2��)
		nowBeamBucketArray.reverse();

		//�����ς�:�����ՖʂȂ�}���
		//�����ς�:�����G�[�W�F���g�ʒu�Ȃ�}���B
		//�����ς�:������Ԃ�΂点�Ȃ��H�v�B���N���Q�l�ɂ��āA�ł��ΐ킳���Ȃ��炩�ȁB=>���ʍ����債�ĂȂ��̂ł͂܂��A�x���͂Ȃ��ĂȂ������[��B
		for (auto itr = nowBeamBucketArray.begin(); itr != nowBeamBucketArray.end(); itr++) {
			for (auto minScoreItr = itr; minScoreItr != nowBeamBucketArray.end(); minScoreItr++) {
				if (minScoreItr == itr)
					continue;
				bool same = true;
				bool sameLocation = true;
				for (int agent_num = 0; agent_num < itr->teams.first.agentNum; agent_num++) {
					if (itr->first_dir[agent_num] != minScoreItr->first_dir[agent_num])
						same = false;
					if (itr->teams.first.agents[agent_num].nowPosition != minScoreItr->teams.first.agents[agent_num].nowPosition)
						sameLocation = false;
				}

				if (same) {
					if (sameLocation) {
						minScoreItr->evaluatedScore *= same_location_demerit;
					}

					bool sameArea = true;
					for (const auto p : step(itr->field.boardSize)) {
						if (itr->field.m_board.at(p).color != minScoreItr->field.m_board.at(p).color) {
							sameArea = false;
						}
					}
					if (sameArea) {
						minScoreItr->evaluatedScore *= same_area_demerit;
					}
				}

			}
		}

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			while (!nowBeamBucketQueues[parallelNum].empty()) {
				nowBeamBucketQueues[parallelNum].pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end(), std::greater<BeamSearchData>());

		for (int32 beamCount = 0; beamCount < beam_size; beamCount++) {
			nowBeamBucketQueues[beamCount % parallelSize].push(nowBeamBucketArray[beamCount]);
		}

	}

	if (search_depth != nowSearchDepth) {
		SafeConsole(U"�T���ł��؂�[��:", nowSearchDepth);
	}

	SearchResult result;

	result.code = AlgorithmStateCode::None;

	if (nowBeamBucketArray.size() == 0) {
		assert(nowBeamBucketArray.size() != 0);
	}
	else {


		int32 count = 0;
		for (int i = 0; i < nowBeamBucketArray.size() && count < result_size; i++) {
			const auto& now_state = nowBeamBucketArray[i];

			bool same = false;
			for (int k = 0; k < i; k++)
			{
				bool check = true;
				for (int m = 0; m < now_state.teams.first.agentNum; m++) {
					if (nowBeamBucketArray[k].first_dir[m] != now_state.first_dir[m] || nowBeamBucketArray[k].first_act[m] != now_state.first_act[m]) {
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