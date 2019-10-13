#include <queue>
#include <future>
#include "SuzukiAlgorithm.hpp"
#include "TakahashiAlgorithm.hpp"


/*
	memo

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else
			return Action::Move;
	};

	auto convXY = [](const s3d::Point& p) {
		return p.x + p.y * 20;//�ő�}�b�v�傫��20
	};

*/

/*

Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::SuzukiBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches) : BeamSearchAlgorithm(beamWidth, std::move(pruneBranches))
{
}

*/

Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::SuzukiBeamSearchAlgorithm(FilePath parameterFile, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms)
	: pruneBranchesAlgorithms(std::move(PBAlgorithms)), parameterFilePath(parameterFile), BeamSearchAlgorithm(0, nullptr)
{
}

void Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::initilize(const Game& game)
{
	if (isInitilized)
		return;
	isInitilized = true;
	for (int32 i = 0; i < parallelSize; i++)
		this->pruneBranchesAlgorithms[i]->initilize(game);

	{
		s3d::JSONReader parameterData(parameterFilePath);
		this->parameter.resultSize = parameterData[(U"resultSize")].get<int32>();
		this->parameter.sameLocationDemerit = parameterData[(U"sameLocationDemerit")].get<double>();
		this->parameter.sameAreaDemerit = parameterData[(U"sameAreaDemerit")].get<double>();
		this->parameter.wasMovedDemerit = parameterData[(U"wasMovedDemerit")].get<int32>();
		this->parameter.waitDemerit = parameterData[(U"waitDemerit")].get<int32>();
		this->parameter.diagonalBonus = parameterData[(U"diagonalBonus")].get<double>();
		this->parameter.fastBonus = parameterData[(U"fastBonus")].get<double>();
		this->parameter.enemyPeelBonus = parameterData[(U"enemyPeelBonus")].get<double>();
		this->parameter.myAreaMerit = parameterData[(U"myAreaMerit")].get<double>();
		this->parameter.enemyAreaMerit = parameterData[(U"enemyAreaMerit")].get<double>();
		this->parameter.minusDemerit = parameterData[(U"minusDemerit")].get<double>();
		this->parameter.mineRemoveDemerit = parameterData[(U"mineRemoveDemerit")].get<double>();
		this->parameter.timeMargin = parameterData[(U"timeMargin")].get<double>();
		this->parameter.cancelDemerit = parameterData[(U"cancelDemerit")].get<double>();
	}


	//�r�[�����̎�������
	const int32 canSimulateNums[9] = { 0,0,9,9,9,6,5,3,3 };
	const int32 wishSearchDepth[9] = { 0,0,10,15,15,15,20,20,15 };
	const double staticBeamWidth[9] = { 0,0,100,40,13,7,3.4,2.9,2.3 };
	const double calcPerSec = 300'000'000;
	const double actionNum = 3;
	const double secondCalcTime = 0.4;

	int32 autoBeamWidth = (int32)((parallelSize * (game.turnMillis - parameter.timeMargin) * calcPerSec)
		/ (1000.0 * wishSearchDepth[game.teams.first.agentNum] *
			pow(canSimulateNums[game.teams.first.agentNum], game.teams.first.agentNum) *
			actionNum * game.field.boardSize.x * game.field.boardSize.y) * secondCalcTime);

	

	autoBeamWidth = std::min(1000, autoBeamWidth);
	autoBeamWidth = (int32)((double)autoBeamWidth / (1 + Log2(autoBeamWidth) * 0.17));
	beam_size = autoBeamWidth;
	if (staticBeamWidth[game.teams.first.agentNum] != 0) {
		beam_size = (size_t)(staticBeamWidth[game.teams.first.agentNum] * game.turnMillis / 1000.0);
		if (beam_size >= 100) {
			beam_size = (size_t)((double)beam_size * 9 / 10 + Min((double)beam_size/2.2,100.0));
		}
		else {
			beam_size = (size_t)((double)beam_size * 9.5 / 10);
		}
		beam_size = Max(beam_size, (size_t)30);
		SafeConsole(U"Static Width");
	}
	search_depth = wishSearchDepth[game.teams.first.agentNum];
	can_simulate_num = canSimulateNums[game.teams.first.agentNum];


	SafeConsole(U"SuzukiAlgorithm �r�[�����F", beam_size);
}

Procon30::SearchResult Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::execute(const Game& game)
{

	return this->PruningExecute(game);
}

std::pair<int32, int32> Procon30::innerCalculateScoreFast(Procon30::Field& field, Procon30::TeamColor teamColor, unsigned short qFast[2000], std::bitset<1023> & visitFast)
{

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

	while (q_front < q_end) {

		const auto& now = qFast[q_front++];

		if (!visitFast[now]) {

			visitFast[now] = true;

			if (field.m_board.at(SHORT_TO_Y(now), SHORT_TO_X(now)).color != teamColor) {
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


	int32 resultTile = 0;
	int32 resultArea = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (field.m_board.at(y, x).color == teamColor) {
				resultTile += field.m_board.at(y, x).score;
			}
			else if (visitFast[XY_TO_SHORT(x, y)] == false) {
				resultArea += abs(field.m_board.at(y, x).score);
			}
		}
	}

	visitFast.reset();

	return std::pair<int32, int32>(resultTile, resultArea);
}

Procon30::SearchResult Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::PruningExecute(const Game& game)
{

	//�����萔�̓X�l�[�N�P�[�X�œ��ꋖ����

	//�����Œ�������
	//�Ƃ肠�����b��Ń}�b�v�̍L���ƒT�����ԂɊ֌W�͂Ȃ����߁A�icalcScore�̉e���͖���j
	//�r�[�����́A�G�[�W�F���g���Ō��߂܂��B

	//constexpr int32 canBeamWidths[9] = { 0,0,100,100,100, 100, 100, 100, 100 };

	//beamWidth = canBeamWidths[game.teams.first.agentNum];

	const int32 canSimulateNums[9] = { 0,0,9,9,9,7,5,3,3 };
	const int32 wishSearchDepth[9] = { 0,0,10,15,15,15,20,20,10 };

	//�����萔�̓X�l�[�N�P�[�X�œ��ꋖ����
	//�����Ɉˑ�����p�����[�^�[�i�ω��A������Ŏ��G��Ȃ��Ă����j
	const int field_width = game.field.boardSize.x;
	const int field_height = game.field.boardSize.y;

	//�A���S���Y���Ɋ֌W����p�����[�^�[(��ω�)
	const double time_margin = parameter.timeMargin;
	const size_t result_size = parameter.resultSize;
	const double same_location_demerit = parameter.sameLocationDemerit;
	const double same_area_demerit = parameter.sameAreaDemerit;
	const int was_moved_demerit = parameter.wasMovedDemerit;
	const int wait_demerit = parameter.waitDemerit;
	const double diagonal_bonus = parameter.diagonalBonus;
	const double fast_bonus = parameter.fastBonus;
	const double enemy_peel_bonus = parameter.enemyPeelBonus;
	const double my_area_merit = parameter.myAreaMerit;
	const double enemy_area_merit = parameter.enemyAreaMerit;
	const double minus_demerit = parameter.minusDemerit;
	const double mine_remove_demerit = parameter.mineRemoveDemerit;
	std::array<double,30> turn_weight = {};

	//�������̂��߂ɂ����Ōv�Z
	for (int i = 0; i < 30; i++) {
		turn_weight[i] = pow(fast_bonus, search_depth - i);
	}

	//���Z�q�̏���
	//�����������炵������operator���I�[�o�[���C�h���Ď���
	//auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };


	int32 beforeOneDepthSearchUsingTime = 0;
	int32 beforeOneDepthSearchEndTime = game.turnTimer.ms();
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
	for (nowSearchDepth = 0; nowSearchDepth < Min(search_depth, game.MaxTurn - game.turn); nowSearchDepth++) {

		if (nowSearchDepth != 0) {
			const int32 turnTimerMs = game.turnTimer.ms();
			beforeOneDepthSearchUsingTime = turnTimerMs - beforeOneDepthSearchEndTime;
			beforeOneDepthSearchEndTime = turnTimerMs;
			if ((game.turnMillis - time_margin) < game.turnTimer.ms() + beforeOneDepthSearchUsingTime) {
				break;
			}
		}

		//���̃��[�v����񉻂���B�����A
		std::future<std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>>> beamSearchFuture[parallelSize];

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			beamSearchFuture[parallelNum] = std::async(std::launch::async, [nowSearchDepth, field_width, field_height, my_area_merit, enemy_area_merit, fast_bonus, minus_demerit, enemy_peel_bonus, wait_demerit, diagonal_bonus,
				was_moved_demerit, mine_remove_demerit, turn_weight](
					size_t beam_size, [[maybe_unused]]int32 search_depth, int32 can_simulate_num,
					std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nowBeamBucketQueue, std::unique_ptr<PruneBranchesAlgorithm>& pruneBranches) {

						std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nextBeamBucketQueue;

						std::bitset<1023> visitFast = {};
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

							BeamSearchData now_state = (nowBeamBucketQueue.top());
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
							//TODO:�A���S���Y���ł܂��Ă���v�Z�ʌ��r�[�����̒������܂����BNOW

							pruneBranches->pruneBranches(can_simulate_num, enumerateDir, now_state.field, now_state.teams);

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

									if (!InRange(now_state.teams.first.agents[agent_num].nextPosition)) {
										skip = true;
										//TODO:�����Ŕ͈͊O�̃`�F�b�N������������
									}
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
										for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

											Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

											if (nowSearchDepth == 0) {

												next_state.first_dir[agent_num] =
													next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition;

												if (targetTile.color == next_state.teams.first.color) {
													next_state.first_act[agent_num] = next_act[agent_num] ? Action::Remove : Action::Move;
												}
												else if (targetTile.color == next_state.teams.second.color) {
													next_state.first_act[agent_num] = Action::Remove;
												}
												else if (targetTile.color == TeamColor::None) {
													next_state.first_act[agent_num] = Action::Move;
												}
											}

											//�t�B�[���h�ƃG�[�W�F���g�̈ʒu�X�V
											//�G�[�W�F���g�̎��ɍs���^�C���̐F
											if (targetTile.color == next_state.teams.first.color) {
												if (!next_act[agent_num]) {//Move
													if (next_state.teams.first.agents[agent_num].nowPosition != next_state.teams.first.agents[agent_num].nextPosition) {//Moved
														next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;
														next_state.evaluatedScore += was_moved_demerit * turn_weight[nowSearchDepth];
													}
													else//Wait
														next_state.evaluatedScore += wait_demerit * turn_weight[nowSearchDepth];
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
														next_state.evaluatedScore += (abs(targetTile.score) * turn_weight[nowSearchDepth] + mine_remove_demerit) * enemy_peel_bonus;
													else
														next_state.evaluatedScore = -100000000;//���蓾�Ȃ��A����������܂�
												}
											}
											else if (targetTile.color == next_state.teams.second.color) {
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
													next_state.evaluatedScore += (targetTile.score * turn_weight[nowSearchDepth] + minus_demerit) * enemy_peel_bonus;
												else
													next_state.evaluatedScore += (targetTile.score * turn_weight[nowSearchDepth]) * enemy_peel_bonus;

											}
											else if (targetTile.color == TeamColor::None) {
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
													next_state.evaluatedScore += (targetTile.score + minus_demerit) * turn_weight[nowSearchDepth];
												else
													next_state.evaluatedScore += targetTile.score * turn_weight[nowSearchDepth];
											}
										}

										if (mustCalcFirstScore) {
											std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, next_state.teams.first.color, qFast, visitFast);
											next_state.teams.first.tileScore = s.first;
											next_state.teams.first.areaScore = s.second;
											next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;
										}
										if (mustCalcSecondScore) {
											std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, next_state.teams.second.color, qFast, visitFast);
											next_state.teams.second.tileScore = s.first;
											next_state.teams.second.areaScore = s.second;
											next_state.teams.second.score = next_state.teams.second.tileScore + next_state.teams.second.areaScore;
										}

										next_state.evaluatedScore += (next_state.teams.first.areaScore - now_state.teams.first.areaScore) * my_area_merit +
											(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * turn_weight[nowSearchDepth];

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

										//move or remove
										for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {
											if (next_act[agent_num] == true) {
												next_act[agent_num] = false;
											}
											else {
												if (now_state.field.m_board.at(now_state.teams.first.agents[agent_num].nextPosition).color
													== now_state.teams.first.color
													//���������������������Ă���B��next_state�����������ĂȂ�����OK�ł́H����Move�Ȃǂł͏��������Ă���
													//���߂���now_state�ɂ���΂悳����
													&& now_state.teams.first.agents[agent_num].nowPosition != now_state.teams.first.agents[agent_num].nextPosition) {
													next_act[agent_num] = true;
													break;
												}
											}
											if (agent_num == now_state.teams.first.agentNum - 1) {
												actionLoop = false;
											}
										}
									}
								}
							}
						}

						if (nextBeamBucketQueue.size() == 0 && nowSearchDepth == 10)
							SafeConsole(U"death", U" nowSerchDepth=", nowSearchDepth, U" nowQueueSize", nowBeamBucketQueue.size());

						return nextBeamBucketQueue;
				}
				, beam_size, search_depth, can_simulate_num,
					nowBeamBucketQueues[parallelNum], ref(pruneBranchesAlgorithms[parallelNum]));
		}

		nowBeamBucketArray.clear();

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> result = beamSearchFuture[parallelNum].get();

			if (result.size() == 0 && nowBeamBucketQueues[parallelNum].size() != 0) {
				SafeConsole(U"result = 0", U" nowSerchDepth=", nowSearchDepth, U" parallelNum=", parallelNum);
			}

			//pop����B
			while (!result.empty()) {
				nowBeamBucketArray.push_back((result.top()));
				result.pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end());


		//TODO:������Ԃ�΂点�Ȃ��H�v�B���N���Q�l�ɂ��āA�ł��ΐ킳���Ȃ��炩�ȁB�����܂łŃ��C����BeamSearch������̈�U�I������

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
				
				int32 sameLocation = 0;
				for (int agent_num1 = 0; agent_num1 < itr->teams.first.agentNum; agent_num1++) {
					bool same = false;
					for (int agent_num2 = 0; agent_num2 < minScoreItr->teams.first.agentNum; agent_num2++) {
						if (itr->first_dir[agent_num1] == minScoreItr->first_dir[agent_num2])
							same = true;
					}
					if (same)
						sameLocation++;
				}

				if (sameLocation == minScoreItr->teams.first.agentNum) {
					minScoreItr->evaluatedScore *= same_location_demerit;
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

		//�w�͂̔s�k
		if (nowBeamBucketArray.size() == 0) {
			SafeConsole(U"[�w�͂̔s�k] SuzukiAlgorithm ����Ԃ������̂őł��؂�܂��B GameID:",game.gameID);
			nowBeamBucketArray.push_back(nowBeamBucketQueues[0].top());
			break;
		}

		//�����ŏ�Ԃ���ɂ��Ă���B
		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			while (!nowBeamBucketQueues[parallelNum].empty()) {
				nowBeamBucketQueues[parallelNum].pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end(), std::greater<BeamSearchData>());

		//CAUTION:�S�����ŐςނƂ��������A�łǂ����悤���Ȃ��P�[�X�ɓ�����\������
		for (int32 beamCount = 0; beamCount < Min(nowBeamBucketArray.size(), beam_size); beamCount++) {
			nowBeamBucketQueues[beamCount % parallelSize].push(nowBeamBucketArray[beamCount]);
		}

	}

	if (search_depth != nowSearchDepth) {
		SafeConsole(U"SuzukiAlgorithm�T���ł��؂�[��:", nowSearchDepth, U" �ł��؂莞��[ms]:", game.turnTimer.ms());
	}
	else {
		SafeConsole(U"SuzukiAlgo �T����������[ms]:", game.turnTimer.ms(),U" GameID:",game.gameID);
	}
	SearchResult result;

	if (nowBeamBucketArray.size() == 0) {
		assert(nowBeamBucketArray.size() != 0);
		SafeConsole(U"Suzuki UnknownError");
		result.code = AlgorithmStateCode::UnknownError;
	}
	else {

		result.code = AlgorithmStateCode::None;

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
