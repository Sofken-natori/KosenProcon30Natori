#include "SuzukiAlgorithm.hpp"
#include <queue>

Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::AlternatelyBeamSearchAlgorithm(int32 beamWidth)
{
}

Procon30::SearchResult Procon30::SUZUKI::AlternatelyBeamSearchAlgorithm::execute(const Game& game)
{
	return SearchResult();
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
	const int canSimulationNums[9] = { 0,0,13,8,8,5,4,3,3 };

	BeamSearchData first_state;

	first_state.evaluatedScore = 0;
	first_state.field = game.field;
	first_state.teams = game.teams;

	nowBeamBucketQueue.push(first_state);

	//TODO:いい加減秒数に合わせて計算打ち切る工夫追加しろ！あほ！このままだと本番死ぬぞ
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
