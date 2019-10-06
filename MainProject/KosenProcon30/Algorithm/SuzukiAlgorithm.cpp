#include "SuzukiAlgorithm.hpp"
#include <queue>

Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::AlternatelyBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches) : BeamSearchAlgorithm(beamWidth, std::move(pruneBranches))
{
}

Procon30::SearchResult Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::execute(const Game& game)
{

	//相手の動作を得る。
	Game gameCopy;
	gameCopy.field = game.field;
	gameCopy.teams = game.teams;
	std::swap(gameCopy.teams.first, gameCopy.teams.second);
	gameCopy.turn = game.turn;
	gameCopy.MaxTurn = game.MaxTurn;
	const auto& second_result = BeamSearchAlgorithm::execute(gameCopy);

	//列挙のアルゴとかは変更なしでいいと思うけど、評価を行う段階でシミュレーションをしてそのうえで評価する。
	constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	//内部定数はスネークケースで統一許して

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
	const int cancel_demerit = 0.9;

	//WAGNI:Listへの置き換え。追加は早くなる気がする。


	//方向の集合用にここで確保する。
	//[エージェント番号][方向番号（終端を-2にしておいて）] = 方向;
	std::array<std::array<Point, 10>, 8> enumerateDir;

	//WAGNI:左下に滞留問題＝＞解決。そもそもPOSTされてないせいだった。その内、時間で自動で切るように

	//演算子の準備
	auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };

	std::vector<BeamSearchData> nowContainer;
	nowContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nowBeamBucketQueue(
		compare, std::move(nowContainer));

	std::vector<BeamSearchData> nextContainer;
	nextContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nextBeamBucketQueue(
		compare, std::move(nextContainer));

	//8エージェントの場合 8^10=10^9ぐらいになりえるのでたまらん
	//5secから15secらしい、
	//2^10=1024で昨年、1secだから
	//8^4=4096ぐらいにしたい。
	//可能なシミュレーション手数一覧。
	//+1は普通に見積もれる。
	//3ぐらいまでは昨年と同じでいける。
	const int canSimulationNums[9] = { 0,0,13,8,8,5,5,4,4 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucketQueue.push(first_state);

	for (int i = 0; i < search_depth; i++) {
		//enumerate
		while (!nowBeamBucketQueue.empty()) {
			BeamSearchData now_state = std::move(nowBeamBucketQueue.top());
			nowBeamBucketQueue.pop();

			//8^9はビームサーチでも計算不能に近い削らないと
			//枝狩り探索を呼び出して、ここでいい感じにする。
			//機能としては、Fieldとteamsを与えることで1000-10000前後の方向の集合を返す。
			assert(pruneBranchesAlgorithm);

			pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);

			//bool okPrune = pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);
			//assert(okPrune);

			int32 next_dir[8] = {};

			bool enumerateLoop = true;
			while (enumerateLoop) {

				bool skip = false;

				//実際に移動させてみる。
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					now_state.teams.first.agents[agent_num].nextPosition
						= now_state.teams.first.agents[agent_num].nowPosition
						+ enumerateDir[agent_num][next_dir[agent_num]];

					if (!InRange(now_state.teams.first.agents[agent_num].nextPosition))
						skip = true;
				}

				//now nextの被り検出
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

				//next nextの被り検出
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

				//ここで次の移動方向を更新する。
				//9方向だから9まで
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

						//シミュレーションしてみる。やばそうには理論的にならない
						//WAGNI:負けている際、にらみ合いのロック解除の機構
						//自分色のマイナス点をmoveとremoveするステートを作る。
						if (i == 0) {
							//i==0のとき、相手との衝突もシミュレーションする

							//そのため、自分のactionを設定して、その上相手の動作をnext_stateに入れて置く

							//firstのagentのactionについての設定
							for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {
								//nextPositionに合わせて、actionを更新しておく
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

								//i == 0でfirst系を初期化する
								next_state.first_act[agent_num] = next_state.teams.first.agents[agent_num].action;
								next_state.first_dir[agent_num] =
									next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition;


							}

							//secondのagentのnextPosition,nowPosition,actionについて設定。
							for (int agent_num = 0; agent_num < next_state.teams.second.agentNum; agent_num++) {

								//[n番目のの選択肢][エージェントの番号]
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

							//VirtualServerのsimulationを流用。自分で書いたらばぐらせそうなので仕方ない
							//nextPositionをupdateで書き換えて
							for (int loop = 0; loop < 16; loop++) {
								int flag[2][8] = {};
								bool firstDestroyedFlag[8] = {};
								bool secondDestroyedFlag[8] = {};
								int count = 0;

								//以下を移植していく。
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
													//ここら辺はそれぞれ1回しか通らない
													flag[0][agent_num] = 2;//味方の動きをつぶす
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
													if (flag[0][agent_num] != 2)
														flag[0][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == f_agent.nowPosition) {
													flag[0][agent_num] = 2;//味方に動きをつぶされる
												}
												else {
													if (flag[0][agent_num] == 0)
														flag[0][agent_num] = 1;
												}
											}
											const auto& s_team = next_state.teams.second;
											for (const auto& s_agent : s_team.agents) {
												if (agent.nextPosition == s_agent.nextPosition) {
													flag[0][agent_num] = 2;//相手と動きをつぶしあう
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == s_agent.nowPosition && s_agent.action == Action::Move) {
													if (flag[0][agent_num] != 2)
														flag[0][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == s_agent.nowPosition) {
													flag[0][agent_num] = 2;//相手に動きをつぶされる
													//つまり動かないことで相手の動きもつぶせる
													if (s_agent.nextPosition != agent.nowPosition)
														firstDestroyedFlag[agent_num] = true;//多分ここの判定がバグっているはず。

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
													flag[1][agent_num] = 2;//firstと動きをつぶしあう
												}//move or remove , move (nowPosition)
												else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
													if (flag[1][agent_num] != 2)
														flag[1][agent_num] = 3;
												}//move or remove , remove (nowPosition) or stay
												else if (agent.nextPosition == f_agent.nowPosition) {
													flag[1][agent_num] = 2;
													secondDestroyedFlag[agent_num] = true;//firstに動きをつぶされる
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
														next_state.evaluatedScore = -100000000;//あり得ない、動かん方がまし
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
											if (firstDestroyedFlag[agent_num]) {//動きをつぶされる
												next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
											}
											else {//動きをつぶしあう
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
											if (secondDestroyedFlag[agent_num]) {//動きをうまくつぶした
												next_state.evaluatedScore -= wait_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - agent_num);
											}
											else {//動きをつぶしあった

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
							//スコア計算しなおし。
							//要らない
							mustCalcFirstScore = true;
							mustCalcSecondScore = true;
						}
						else {

							//こっから既存の実装。
							for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

								Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

								//フィールドとエージェントの位置更新
								//エージェントの次に行くタイルの色
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
											next_state.evaluatedScore = -100000000;//あり得ない、動かん方がまし
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

						//終了でだんだん評価値が、ゲーム自体の勝敗と同じくなるように調整。
						const double finish_slope = std::max(1 - (game.MaxTurn - game.turn - i) / 10.0, 0.0);

						next_state.evaluatedScore = (1 - finish_slope) * next_state.evaluatedScore + finish_slope * ((next_state.teams.first.score - next_state.teams.second.score));

						//いけそうだからpushする。
						if (nextBeamBucketQueue.size() > beam_size) {
							if (nextBeamBucketQueue.top().evaluatedScore < next_state.evaluatedScore) {
								nextBeamBucketQueue.pop();
								nextBeamBucketQueue.push(std::move(next_state));
							}
						}
						else {
							nextBeamBucketQueue.push(std::move(next_state));
						}

						//次の移動方向への更新。先頭でやると入ってすぐ更新されておかしな話になる。
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

/*
	memo

	auto getAction = [&](s3d::Point p) {
		if (game.field.m_board.at(p).color == game.teams.second.color)
			return Action::Remove;
		else
			return Action::Move;
	};

	auto convXY = [](const s3d::Point& p) {
		return p.x + p.y * 20;//最大マップ大きさ20
	};

*/

Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::SuzukiBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches) : BeamSearchAlgorithm(beamWidth, std::move(pruneBranches))
{
}

Procon30::SearchResult Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::execute(const Game& game)
{
	//ここらへんで、4以上のあれと分岐する。
	if (game.teams.first.agentNum >= 4) {
		return this->PruningExecute(game);
	}

	constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	//内部定数はスネークケースで統一許して

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

	//WAGNI:Listへの置き換え。追加は早くなる気がする。

	//TODO:もっと強い枝狩りを実装しよう。
	//WAGNI:左下に滞留問題＝＞解決。そもそもPOSTされてないせいだった。その内、時間で自動で切るように

	//Array<BeamSearchData> nowBeamBucket;
	//nowBeamBucket.reserve(beam_size);
	//Array<BeamSearchData> nextBeamBucket;
	//nextBeamBucket.reserve(beam_size * 100000);

	auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };

	std::vector<BeamSearchData> nowContainer;
	nowContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nowBeamBucketQueue(
		compare, std::move(nowContainer));

	std::vector<BeamSearchData> nextContainer;
	nextContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nextBeamBucketQueue(
		compare, std::move(nextContainer));

	//8エージェントの場合 8^10=10^9ぐらいになりえるのでたまらん
	//5secから15secらしい、
	//2^10=1024で昨年、1secだから
	//8^4=4096ぐらいにしたい。
	//可能なシミュレーション手数一覧。
	//+1は普通に見積もれる。
	//3ぐらいまでは昨年と同じでいける。
	const int canSimulationNum[9] = { 0,0,13,8,7,5,5,4,4 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucketQueue.push(first_state);

	for (int i = 0; i < search_depth; i++) {
		//enumerate
		while (!nowBeamBucketQueue.empty()) {
			BeamSearchData now_state = std::move(nowBeamBucketQueue.top());
			nowBeamBucketQueue.pop();

			int32 next_dir[8] = {};

			bool enumerateLoop = true;
			while (enumerateLoop) {

				bool skip = false;

				//実際に移動させてみる。
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					now_state.teams.first.agents[agent_num].nextPosition
						= now_state.teams.first.agents[agent_num].nowPosition
						+ Point(dxy[next_dir[agent_num]], dxy[next_dir[agent_num] + 1]);

					if (!InRange(now_state.teams.first.agents[agent_num].nextPosition))
						skip = true;
				}

				//now nextの被り検出
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

				//next nextの被り検出
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

				//9方向だから9まで
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					if (next_dir[agent_num] == 8) {
						next_dir[agent_num] = 0;
						if (agent_num == now_state.teams.first.agentNum - 1) {
							//次の奴に終端フラグが立つと、このシミュレーションが終わったタイミングでループから抜ける。
							enumerateLoop = false;
							break;
						}
					}
					else {
						next_dir[agent_num]++;
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

						//シミュレーションしてみる。やばそうには理論的にならない
						//WAGNI:負けている際、にらみ合いのロック解除の機構
						//自分色のマイナス点をmoveとremoveするステートを作る。
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

							//フィールドとエージェントの位置更新
							//エージェントの次に行くタイルの色
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
									{
										//青が両脇2か所以上あったら
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
										next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i) + mine_remove_demerit) * enemy_peel_bonus;
									else
										next_state.evaluatedScore = -100000000;//あり得ない、動かん方がまし
								}
								break;
							case TeamColor::Red:

								targetTile.color = TeamColor::None;
								{
									//赤が両脇2か所以上あったら
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
									next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i) + minus_demerit) * enemy_peel_bonus;
								else
									next_state.evaluatedScore += (targetTile.score * pow(fast_bonus, search_depth - i)) * enemy_peel_bonus;

								break;
							case TeamColor::None:
								const bool isDiagonal = (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).x != 0
									&& (next_state.teams.first.agents[agent_num].nextPosition - next_state.teams.first.agents[agent_num].nowPosition).y != 0;
								next_state.teams.first.agents[agent_num].nowPosition = next_state.teams.first.agents[agent_num].nextPosition;

								targetTile.color = next_state.teams.first.color;
								{
									//青が両脇2か所以上あったら
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
										next_state.teams.first.tileScore += targetTile.score;
										next_state.teams.first.areaScore += targetTile.score;
									}
								}

								next_state.evaluatedScore += isDiagonal * diagonal_bonus;
								if (targetTile.score <= 0)
									next_state.evaluatedScore += (targetTile.score + minus_demerit) * pow(fast_bonus, search_depth - i);
								else
									next_state.evaluatedScore += targetTile.score * pow(fast_bonus, search_depth - i);

								break;
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

						//いけそうだからpushする。
						if (nextBeamBucketQueue.size() > beam_size) {
							if (nextBeamBucketQueue.top().evaluatedScore < next_state.evaluatedScore) {
								nextBeamBucketQueue.pop();
								nextBeamBucketQueue.push(std::move(next_state));
							}
						}
						else {
							nextBeamBucketQueue.push(std::move(next_state));
						}

						//次の移動方向への更新。先頭でやると入ってすぐ更新されておかしな話になる。
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

Procon30::SearchResult Procon30::SUZUKI::SuzukiBeamSearchAlgorithm::PruningExecute(const Game& game)
{
	constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

	auto InRange = [&](s3d::Point p) {
		return 0 <= p.x && p.x < game.field.boardSize.x && 0 <= p.y && p.y < game.field.boardSize.y;
	};

	//内部定数はスネークケースで統一許して

	//ここで調整する
	//とりあえず暫定でマップの広さと探索時間に関係はないため、（calcScoreの影響は未定）
	//ビーム幅は、エージェント数で決めます。

	//constexpr int32 canBeamWidths[9] = { 0,0,100,100,100, 100, 100, 100, 100 };

	//beamWidth = canBeamWidths[game.teams.first.agentNum];

	const size_t beam_size = beamWidth;

	constexpr size_t result_size = 3;
	constexpr TeamColor my_team = TeamColor::Blue;
	constexpr TeamColor enemy_team = TeamColor::Red;
	const int search_depth = std::min(10, game.MaxTurn - game.turn);
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

	//TODO:ターンが進めば進むほど実際の評価と同じようになるようにする。
	//演算子の準備
	auto compare = [](const BeamSearchData& left, const BeamSearchData& right) {return left.evaluatedScore > right.evaluatedScore; };

	std::vector<BeamSearchData> nowContainer;
	nowContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nowBeamBucketQueue(
		compare, std::move(nowContainer));

	std::vector<BeamSearchData> nextContainer;
	nextContainer.reserve(10000);
	std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, decltype(compare)> nextBeamBucketQueue(
		compare, std::move(nextContainer));



	//方向の集合用にここで確保する。
	//[エージェント番号][方向番号（終端を-2にしておいて）] = 方向;
	std::array<std::array<Point, 10>, 8> enumerateDir;

	//8エージェントの場合 8^10=10^9ぐらいになりえるのでたまらん
	//5secから15secらしい、
	//2^10=1024で昨年、1secだから
	//8^4=4096ぐらいにしたい。
	//可能なシミュレーション手数一覧。
	//+1は普通に見積もれる。
	//3ぐらいまでは昨年と同じでいける。
	//TODO:アルゴリズム固まってから計算量見つつビーム幅の調整しませう。
	constexpr int32 canSimulationNums[9] = { 0,0,13,8,8,5,4,3,3 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucketQueue.push(first_state);

	int32 beforeOneDepthSearchEnd = game.turnTimer.ms();
	int32 nowSearchDepth;

	//秒数に合わせて計算打ち切る工夫追加。とりあえずこれだけあれば動けないことはない。
	//WAGNI:余った時間に合わせてビーム幅を調整する工夫？
	for (nowSearchDepth = 0; nowSearchDepth < search_depth; nowSearchDepth++) {

		if (nowSearchDepth != 0) {
			beforeOneDepthSearchEnd = game.turnTimer.ms() - beforeOneDepthSearchEnd;
			if ((game.turnMillis - timeMargin) < game.turnTimer.ms() + beforeOneDepthSearchEnd) {
				break;
			}
		}


		//enumerate
		while (!nowBeamBucketQueue.empty()) {
			BeamSearchData now_state = std::move(nowBeamBucketQueue.top());
			nowBeamBucketQueue.pop();

			//8^9はビームサーチでも計算不能に近い削らないと
			//枝狩り探索を呼び出して、ここでいい感じにする。
			//機能としては、Fieldとteamsを与えることで1000-10000前後の方向の集合を返す。
			assert(pruneBranchesAlgorithm);

			pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);

			//bool okPrune = pruneBranchesAlgorithm->pruneBranches(canSimulationNums[now_state.teams.first.agentNum], enumerateDir, now_state.field, now_state.teams);
			//assert(okPrune);

			int32 next_dir[8] = {};

			bool enumerateLoop = true;
			while (enumerateLoop) {

				bool skip = false;

				//実際に移動させてみる。
				for (int agent_num = 0; agent_num < now_state.teams.first.agentNum; agent_num++) {
					now_state.teams.first.agents[agent_num].nextPosition
						= now_state.teams.first.agents[agent_num].nowPosition
						+ enumerateDir[agent_num][next_dir[agent_num]];

					if (!InRange(now_state.teams.first.agents[agent_num].nextPosition))
						skip = true;
				}

				//now nextの被り検出
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

				//next nextの被り検出
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

				//ここで次の移動方向を更新する。
				//9方向だから9まで
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

						//シミュレーションしてみる。やばそうには理論的にならない
						//WAGNI:負けている際、にらみ合いのロック解除の機構
						//自分色のマイナス点をmoveとremoveするステートを作る。
						for (int agent_num = 0; agent_num < next_state.teams.first.agentNum; agent_num++) {

							Tile& targetTile = next_state.field.m_board.at(next_state.teams.first.agents[agent_num].nextPosition);

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

							//フィールドとエージェントの位置更新
							//エージェントの次に行くタイルの色
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
										//青が両脇2か所以上あったら
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
										next_state.evaluatedScore = -100000000;//あり得ない、動かん方がまし
								}
								break;
							case TeamColor::Red:
								targetTile.color = TeamColor::None;

								{
									//赤が両脇2か所以上あったら
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
									//青が両脇2か所以上あったら
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
							(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * pow(fast_bonus, search_depth - nowSearchDepth);

						//いけそうだからpushする。
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

		//ここ状態を偏らせない工夫。去年を参考にして、でも対戦させながらかな。ここまででメインのBeamSearchいじるの一旦終了かも

		//最初の方向が一緒な場合
		//現在の占有しているマップが一緒なら減点したい
		//もしくは、現在の位置が一緒なら減点


		std::list<BeamSearchData> nowBeamBucket;

		//popする。
		while (!nextBeamBucketQueue.empty()) {
			nowBeamBucket.push_back(std::move(nextBeamBucketQueue.top()));
			nextBeamBucketQueue.pop();
		}
		//stackする。
		//減点する(とりあえず2乗)
		nowBeamBucket.reverse();

		//実装済み:同じ盤面なら枝狩り
		//実装済み:同じエージェント位置なら枝狩り。
		//実装済み:ここ状態を偏らせない工夫。去年を参考にして、でも対戦させながらかな。=>結果差が大してないのではまあ、遅くはなってないしうーん。
		for (auto itr = nowBeamBucket.begin(); itr != nowBeamBucket.end(); itr++) {
			for (auto minScoreItr = itr; minScoreItr != nowBeamBucket.end(); minScoreItr++) {
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

		//pushする。
		for (auto itr = nowBeamBucket.begin(); itr != nowBeamBucket.end(); itr++) {
			nowBeamBucketQueue.push(*itr);
		}
		//nowBeamBucketQueue.swap(nextBeamBucketQueue);

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
