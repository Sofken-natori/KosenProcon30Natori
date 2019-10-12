#include <queue>
#include <future>
#include <cmath>
#include "Agent.hpp"
#include "Algorithm.hpp"
#include "Algorithm/SuzukiAlgorithm.hpp"

/*
Procon30::PrivateAlgorithm::PrivateAlgorithm(int32 beamWidth, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms, std::unique_ptr<Algorithm> secondAlgorithm)
	: pruneBranchesAlgorithms(std::move(PBAlgorithms)), secondBeamSearchAlgorithm(std::move(secondAlgorithm))
{
	parameter.beamWidth = beamWidth;
	const int32 canSimulateNums[9] = { 0,0,9,9,9,6,5,3,3 };
	const int32 wishSearchDepth[9] = { 0,0,10,15,15,15,20,20,15 };

	parameter.targetSearchDepth = wishSearchDepth

}
*/

Procon30::ProconAlgorithm::ProconAlgorithm(FilePath parameterFile, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms, std::unique_ptr<Algorithm> secondAlgorithm)
	: pruneBranchesAlgorithms(std::move(PBAlgorithms)), secondBeamSearchAlgorithm(std::move(secondAlgorithm)), parameterFilePath(parameterFile)
{
}

void Procon30::ProconAlgorithm::initilize(const Game& game)
{
	if (isInitilized)return;
	isInitilized = true;
	SafeConsole(U"ProconAlgo init");
	for (int32 i = 0; i < parallelSize; i++)
		this->pruneBranchesAlgorithms[i]->initilize(game);

	//parameterの読み込みはinitilizeで行われる。
	{
		s3d::INIData parameterData(parameterFilePath);

		/*
				int32 beamWidth;
				size_t resultSize = 3;
				int32 targetSearchDepth = 10;
				double sameLocationDemerit = 0.7;
				double sameAreaDemerit = 0.7;
				int wasMovedDemerit = -5;
				int waitDemerit = -10;
				double diagonalBonus = 1.5;
				double fastBonus = 1.03;
				double enemyPeelBonus = 0.9;
				double myAreaMerit = 0.4;
				double enemyAreaMerit = 0.8;
				int minusDemerit = -2;
				int mineRemoveDemerit = -1;
				int32 timeMargin = 1000;
				double cancelDemerit = 0.9;
		*/
		this->parameter.resultSize = Parse<int32>(parameterData.getGlobalVaue(U"resultSize"));
		this->parameter.sameLocationDemerit = Parse<double>(parameterData.getGlobalVaue(U"sameLocationDemerit"));
		this->parameter.sameAreaDemerit = Parse<double>(parameterData.getGlobalVaue(U"sameAreaDemerit"));
		this->parameter.wasMovedDemerit = Parse<int32>(parameterData.getGlobalVaue(U"wasMovedDemerit"));
		this->parameter.waitDemerit = Parse<int32>(parameterData.getGlobalVaue(U"waitDemerit"));
		this->parameter.diagonalBonus = Parse<double>(parameterData.getGlobalVaue(U"diagonalBonus"));
		this->parameter.fastBonus = Parse<double>(parameterData.getGlobalVaue(U"fastBonus"));
		this->parameter.enemyPeelBonus = Parse<double>(parameterData.getGlobalVaue(U"enemyPeelBonus"));
		this->parameter.myAreaMerit = Parse<double>(parameterData.getGlobalVaue(U"myAreaMerit"));
		this->parameter.enemyAreaMerit = Parse<double>(parameterData.getGlobalVaue(U"enemyAreaMerit"));
		this->parameter.minusDemerit = Parse<double>(parameterData.getGlobalVaue(U"minusDemerit"));
		this->parameter.mineRemoveDemerit = Parse<double>(parameterData.getGlobalVaue(U"mineRemoveDemerit"));
		this->parameter.timeMargin = Parse<double>(parameterData.getGlobalVaue(U"timeMargin"));
		this->parameter.cancelDemerit = Parse<double>(parameterData.getGlobalVaue(U"cancelDemerit"));
	}


	//ビーム幅の自動調整
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
			beam_size = (size_t)((double)beam_size * 9 / 10 + Min((double)beam_size / 2.2, 100.0));
		}
		else {
			beam_size = (size_t)((double)beam_size * 9.5 / 10);
		}
		beam_size = Max(beam_size, (size_t)30);
		SafeConsole(U"Static Width");
	}
	search_depth = wishSearchDepth[game.teams.first.agentNum];
	can_simulate_num = canSimulateNums[game.teams.first.agentNum];

	SafeConsole(U"PrivateAlgorithm ビーム幅：", beam_size);

	Game gameCopy;
	gameCopy = game;
	gameCopy.turnMillis /= 2;

	secondBeamSearchAlgorithm->initilize(gameCopy);

	if (game.fieldType != PublicField::NONE && game.fieldType != PublicField::NON_MATCHING) {
		SafeConsole(U"matching");
		//定石の読み込み
		//許して
		std::array<String, 15> fileNames = {
			U"A-1",U"A-2",U"A-3",U"A-4",
			U"B-1",U"B-2",U"B-3",
			U"C-1",U"C-2",
			U"D-1",U"D-2",
			U"E-1",U"E-2",
			U"F-1",U"F-2" };

		FilePath filePath = U"pbook/";
		FilePath path = Format(filePath, fileNames[(int32)game.fieldType - 2], U".pbook");

		//以下読み込み
		s3d::TextReader reader(path);

		if (reader) {

			Optional<String> line = reader.readLine();
			book.firstPos.x = ParseInt<int32>(line->split(' ')[0]);
			book.firstPos.y = ParseInt<int32>(line->split(' ')[1]);

			line = reader.readLine();
			book.secondPos.x = ParseInt<int32>(line->split(' ')[0]);
			book.secondPos.y = ParseInt<int32>(line->split(' ')[1]);

			line = reader.readLine();
			int32 agentNum = ParseInt<int32>(line.value_or(U"0"));

			line = reader.readLine();
			int32 readTurn = ParseInt<int32>(line.value_or(U"0"));
			book.readTurn = readTurn;
			if (agentNum * readTurn == 0) {
				SafeConsole(U"File read Fail");
				return;
			}
			for (int32 i = 0; i < readTurn; i++) {
				line = reader.readLine();
				auto points = line->split(' ');
				if (points.size() < agentNum * 4)
					SafeConsole(U"WTF");
				for (int j = 0; j < agentNum; j++) {
					book.firstData[i][j].first.x = ParseInt<int32>(points[j * 4 + 0]);
					book.firstData[i][j].first.y = ParseInt<int32>(points[j * 4 + 1]);
					book.firstData[i][j].second.x = ParseInt<int32>(points[j * 4 + 2]);
					book.firstData[i][j].second.y = ParseInt<int32>(points[j * 4 + 3]);
				}
			}
			for (int32 i = 0; i < readTurn; i++) {
				line = reader.readLine();
				auto points = line->split(' ');
				if (points.size() < agentNum * 4)SafeConsole(U"WTF");
				for (int j = 0; j < agentNum; j++) {
					book.secondData[i][j].first.x = ParseInt<int32>(points[j * 4 + 0]);
					book.secondData[i][j].first.y = ParseInt<int32>(points[j * 4 + 1]);
					book.secondData[i][j].second.x = ParseInt<int32>(points[j * 4 + 2]);
					book.secondData[i][j].second.y = ParseInt<int32>(points[j * 4 + 3]);
				}
			}
			//以下patternのマッチング
			for (auto i : game.teams.first.agents) {
				if (i.nowPosition == book.firstPos) {
					book.pattern = 1;
					break;
				}
				if (i.nowPosition == book.secondPos) {
					book.pattern = 2;
					break;
				}
			}

			SafeConsole(U"Booking load:", path);
		}
		else {
			SafeConsole(U"Read Fail:", path);
		}
	}

}

Procon30::SearchResult Procon30::ProconAlgorithm::execute(const Game& game)
{
	using BeamSearchData = Procon30::BeamSearchAlgorithm::BeamSearchData;

	//相手の動作を得る。
	Game gameCopy;
	gameCopy = game;
	std::swap(gameCopy.teams.first, gameCopy.teams.second);
	//制限時間はバランシングを考えて半分
	gameCopy.turnMillis /= 2;
	//TODO:交互ビームサーチの実装をチームに依存しないようにする。
	auto second_result = secondBeamSearchAlgorithm->execute(gameCopy);

	//たまに相手がどん詰まりになって動作が返ってこなくなることがあったので再発するようなら
	/*
	if (second_result.orders.size() == 0)
	{
		for (int32 i = 0; i < game.teams.first.agentNum; i++) {
			second_result.orders[0][i].action = Action::Move;
			second_result.orders[0][i].dir = Point(0, 0);
		}
	}
	*/

	//内部定数はスネークケースで統一許して
	//試合に依存するパラメーター（変化、こちらで手を触れなくていい）
	const int32 max_turn = game.MaxTurn;
	const int32 turn = game.turn;
	const int32 field_width = game.field.boardSize.x;
	const int32 field_height = game.field.boardSize.y;
	const PublicField fieldType = game.fieldType;

	//アルゴリズムに関係するパラメーター(非変化)
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
	const double cancel_demerit = parameter.cancelDemerit;

	//TODO:ターンが進めば進むほど実際の評価と同じようになるようにする?終盤で評価値を変えた方がいいのでは？
	//演算子の準備
	//騒乱をもたらしたためoperatorをオーバーライドして実装
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
	//秒数に合わせて計算打ち切る工夫追加。とりあえずこれだけあれば動けないことはない。
	//WAGNI:余った時間に合わせてビーム幅を調整する工夫？
	for (nowSearchDepth = 0; nowSearchDepth < std::min(search_depth, game.MaxTurn - game.turn); nowSearchDepth++) {

		if (nowSearchDepth != 0) {
			const int32 turnTimerMs = game.turnTimer.ms();
			beforeOneDepthSearchUsingTime = turnTimerMs - beforeOneDepthSearchEndTime;
			beforeOneDepthSearchEndTime = turnTimerMs;
			if ((game.turnMillis - time_margin) < turnTimerMs + beforeOneDepthSearchUsingTime) {
				break;
			}
		}

		//このループを並列化する。多分、
		std::future<std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>>> beamSearchFuture[parallelSize];

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			//CAUTION:second_resultのコピーができているかを確認すること。
			beamSearchFuture[parallelNum] = std::async(std::launch::async, [nowSearchDepth, field_width, field_height, max_turn, turn, fieldType,
				cancel_demerit, was_moved_demerit, enemy_peel_bonus, enemy_area_merit, mine_remove_demerit, my_area_merit, fast_bonus,
				wait_demerit, diagonal_bonus, minus_demerit, second_result](
					size_t beam_size, int32 search_depth, int32 can_simulate_num, Book book,
					std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nowBeamBucketQueue, std::unique_ptr<PruneBranchesAlgorithm>& pruneBranches) {

						std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> nextBeamBucketQueue;

						std::bitset<1023> visitFast = {};
						unsigned short qFast[2000] = {};

						//方向の集合用にここで確保する。
						//[エージェント番号][方向番号（終端を-2にしておいて）] = 方向;
						std::array<std::array<Point, 10>, 8> enumerateDir;

						constexpr int dxy[10] = { 1,-1,-1,0,-1,1,0,0,1,1 };

						auto InRange = [&](s3d::Point p) {
							return 0 <= p.x && p.x < field_width && 0 <= p.y && p.y < field_height;
						};

						//enumerate
						while (!nowBeamBucketQueue.empty()) {

							BeamSearchData now_state = (nowBeamBucketQueue.top());
							nowBeamBucketQueue.pop();

							//8^9はビームサーチでも計算不能に近い削らないと
							//枝狩り探索を呼び出して、ここでいい感じにする。
							//機能としては、Fieldとteamsを与えることで1000-10000前後の方向の集合を返す。
							//assert(pruneBranchesAlgorithm);

							//8エージェントの場合 8^10=10^9ぐらいになりえるのでたまらん
							//5secから15secらしい、
							//2^10=1024で昨年、1secだから
							//8^4=4096ぐらいにしたい。
							//可能なシミュレーション手数一覧。
							//+1は普通に見積もれる。
							//3ぐらいまでは昨年と同じでいける。
							pruneBranches->pruneBranches(can_simulate_num, enumerateDir, now_state.field, now_state.teams);

							//できるならここらへんであれしたい。initilizeであれできるからあっちでもいい。でもうーん。turnあるからそとかな
							if (fieldType != PublicField::NON_MATCHING && fieldType != PublicField::NONE) {

								//pattern1 {x,y,dxyNum};
								//pattern2 
								//pattern1 , pattern2 の見分けは、initializeでやり、構造体を注入する

								if (book.readTurn > turn) {

									if (book.pattern == 1) {
										//firstPosと合致
										for (int32 agent_num1 = 0; agent_num1 < now_state.teams.first.agentNum; agent_num1++) {//now_state
											for (int32 agent_num2 = 0; agent_num2 < now_state.teams.first.agentNum; agent_num2++) {//book
												//x,yが合致していたら代入
												if (book.firstData[turn][agent_num2].first.x == -1 || book.firstData[turn][agent_num2].first.y == -1)
													continue;
												if (now_state.teams.first.agents[agent_num1].nowPosition == book.firstData[turn][agent_num2].first) {
													enumerateDir[agent_num1][0] = book.firstData[turn][agent_num2].second;
													enumerateDir[agent_num1][1] = Point(-2, -2);
												}
											}
										}
									}
									else if (book.pattern == 2) {
										//secondPosと合致
										for (int32 agent_num1 = 0; agent_num1 < now_state.teams.first.agentNum; agent_num1++) {//now_state
											for (int32 agent_num2 = 0; agent_num2 < now_state.teams.first.agentNum; agent_num2++) {//book
											//x,yが合致していたら代入
												if (book.secondData[turn][agent_num2].first.x == -1 || book.secondData[turn][agent_num2].first.y == -1)
													continue;
												if (now_state.teams.first.agents[agent_num1].nowPosition == book.secondData[turn][agent_num2].first) {
													enumerateDir[agent_num1][0] = book.secondData[turn][agent_num2].second;
													enumerateDir[agent_num1][1] = Point(-2, -2);
												}
											}
										}
									}
								}
							}


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
										if (nowSearchDepth == 0) {
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
																	//この場合交差して指定しているので
																	//つまり動かないことで相手の動きもつぶせる
																	if (s_agent.nextPosition == agent.nowPosition)
																		firstDestroyedFlag[agent_num] = true;

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
																	//一方的にfirstにつぶされた時だけカウントしたい
																	if (agent.nowPosition != f_agent.nextPosition)
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
																		next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score + minus_demerit) * pow(fast_bonus, search_depth - nowSearchDepth);
																	else {
																		next_state.evaluatedScore += isDiagonal * diagonal_bonus;
																		next_state.evaluatedScore += next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth);
																	}

																	agent.nowPosition = agent.nextPosition;
																	next_state.field.m_board.at(agent.nextPosition).color = TeamColor::Blue;
																}
																else if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
																	next_state.evaluatedScore += was_moved_demerit * pow(fast_bonus, search_depth - nowSearchDepth);
																	agent.nowPosition = agent.nextPosition;
																}
																break;
															case Action::Remove:

																if (next_state.field.m_board.at(agent.nextPosition).color == TeamColor::Blue) {
																	if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
																		next_state.evaluatedScore += (abs(next_state.field.m_board.at(agent.nextPosition).score) * pow(fast_bonus, search_depth - nowSearchDepth) + mine_remove_demerit) * enemy_peel_bonus;
																	else
																		next_state.evaluatedScore = -100000000;//あり得ない、動かん方がまし
																}
																else {
																	if (next_state.field.m_board.at(agent.nextPosition).score <= 0)
																		next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth) + minus_demerit) * enemy_peel_bonus;
																	else
																		next_state.evaluatedScore += (next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth)) * enemy_peel_bonus;
																}

																next_state.field.m_board.at(agent.nextPosition).color = TeamColor::None;
																break;
															case Action::Stay:
																next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - nowSearchDepth);
																break;
															}
														}
														else if (flag[0][agent_num] == 2) {
															//next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - agent_num);
															if (firstDestroyedFlag[agent_num]) {//動きをつぶされるが同時につぶせる。
																if (next_state.teams.first.score > next_state.teams.second.score)
																	next_state.evaluatedScore += 1.2 * cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth);
																else
																	next_state.evaluatedScore += cancel_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth);
															}
															else {//あいてにつぶされるだけ
																next_state.evaluatedScore += wait_demerit * pow(fast_bonus, search_depth - nowSearchDepth);

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
															if (secondDestroyedFlag[agent_num]) {//動きをうまくつぶした。
																next_state.evaluatedScore -= wait_demerit * next_state.field.m_board.at(agent.nextPosition).score * pow(fast_bonus, search_depth - nowSearchDepth);
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
										}

										if (mustCalcFirstScore) {
											std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, TeamColor::Blue, qFast, visitFast);
											next_state.teams.first.tileScore = s.first;
											next_state.teams.first.areaScore = s.second;
											next_state.teams.first.score = next_state.teams.first.tileScore + next_state.teams.first.areaScore;
										}
										if (mustCalcSecondScore) {
											std::pair<int32, int32> s = innerCalculateScoreFast(next_state.field, TeamColor::Red, qFast, visitFast);
											next_state.teams.second.tileScore = s.first;
											next_state.teams.second.areaScore = s.second;
											next_state.teams.second.score = next_state.teams.second.tileScore + next_state.teams.second.areaScore;
										}

										next_state.evaluatedScore += (next_state.teams.first.areaScore - now_state.teams.first.areaScore) * my_area_merit +
											(now_state.teams.second.areaScore - next_state.teams.second.areaScore) * enemy_area_merit * pow(fast_bonus, search_depth - nowSearchDepth);


										//終了でだんだん評価値が、ゲーム自体の勝敗と同じくなるように調整。
										const double finish_slope = std::max(1 - (max_turn - turn - nowSearchDepth) / 10.0, 0.0);

										next_state.evaluatedScore = (1 - finish_slope) * next_state.evaluatedScore + finish_slope * ((next_state.teams.first.score - next_state.teams.second.score));


										//いけそうだからpushする。
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
				, beam_size, search_depth, can_simulate_num, book,
					nowBeamBucketQueues[parallelNum], ref(pruneBranchesAlgorithms[parallelNum]));
		}

		nowBeamBucketArray.clear();

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			std::priority_queue<BeamSearchData, std::vector<BeamSearchData>, std::greater<BeamSearchData>> result = beamSearchFuture[parallelNum].get();

			//popする。
			while (!result.empty()) {
				nowBeamBucketArray.push_back((result.top()));
				result.pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end());


		//ここ状態を偏らせない工夫。去年を参考にして、でも対戦させながらかな。ここまででメインのBeamSearchいじるの一旦終了かも

		//最初の方向が一緒な場合
		//現在の占有しているマップが一緒なら減点したい
		//もしくは、現在の位置が一緒なら減点

		//stackする。
		//減点する(とりあえず2乗)
		nowBeamBucketArray.reverse();

		//実装済み:同じ盤面なら枝狩り
		//実装済み:同じエージェント位置なら枝狩り。
		//実装済み:ここ状態を偏らせない工夫。去年を参考にして、でも対戦させながらかな。=>結果差が大してないのではまあ、遅くはなってないしうーん。
		for (auto itr = nowBeamBucketArray.begin(); itr != nowBeamBucketArray.end(); itr++) {
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
		}

		for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++) {
			while (!nowBeamBucketQueues[parallelNum].empty()) {
				nowBeamBucketQueues[parallelNum].pop();
			}
		}

		std::sort(nowBeamBucketArray.begin(), nowBeamBucketArray.end(), std::greater<BeamSearchData>());

		for (int32 beamCount = 0; beamCount < Min(nowBeamBucketArray.size(), beam_size); beamCount++) {
			nowBeamBucketQueues[beamCount % parallelSize].push(nowBeamBucketArray[beamCount]);
		}

	}

	if (search_depth != nowSearchDepth) {
		SafeConsole(U"PrivateAlgorithm探索打ち切り深さ:", nowSearchDepth, U" 時間", game.turnTimer.ms());
	}
	else {
		SafeConsole(U"PrivateAlgorithm探索終了時間", game.turnTimer.ms());
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