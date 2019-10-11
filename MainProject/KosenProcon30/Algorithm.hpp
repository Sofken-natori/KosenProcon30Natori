#pragma once
#include <bitset>
#include <array>
#include "KosenProcon30.hpp"
#include "Game.hpp"

namespace Procon30 {

	struct BeamSearchParameter {
		size_t resultSize = 3;
		double sameLocationDemerit = 0.7;
		double sameAreaDemerit = 0.7;
		int wasMovedDemerit = -5;
		int waitDemerit = -10;
		double diagonalBonus = 1.5;
		double fastBonus = 1.03;
		double enemyPeelBonus = 0.9;
		double myAreaMerit = 0.4;
		double enemyAreaMerit = 0.8;
		double minusDemerit = -2.0;
		double mineRemoveDemerit = -1.0;
		double timeMargin = 1000.0;
		double cancelDemerit = 0.9;
	};

	enum class AlgorithmStateCode : uint32 {
		None,
		TimeOver,
		UnknownError
	};

	struct Order {
		Action action;
		s3d::Point dir;
	};

	using agentsOrder = std::array<Order, 8>;

	struct SearchResult {
		AlgorithmStateCode code;
		Array<agentsOrder> orders;
	};

	class Algorithm
	{
	private:
		int q_front = 0;
		int q_end = 0;
		std::array<std::array<bool, 22>, 22> visit = {};
		s3d::Point q[2000] = {};
		std::bitset<1023> visitFast = {};
		std::bitset<1023> isTeamColorFast = {};
		unsigned short qFast[2000] = {};
	protected:
		bool isInitilized = false;
	public:
		//tile area
		std::pair<int32, int32> calculateScore(Field& field, TeamColor teamColor);
		std::pair<int32, int32> calculateScoreFast(Field& field, TeamColor teamColor);
		virtual SearchResult execute(const Game& game) = 0;
		virtual void initilize(const Game& game) = 0;
	};

	class RandAlgorithm : public Algorithm {
		SearchResult execute(const Game& game) override final;
		void initilize(const Game& game) override final;
	};

	class PruneBranchesAlgorithm {
	public:
		//canSimulateNumは1エージェント当たりの列挙可能数、enumerateDirに探索する方向を入れる。終端は、-1,-1にして。
		virtual bool pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir,const Field& field,const std::pair<Team, Team>& teams) const;
		virtual void initilize(const Game& game);
	};

	class BeamSearchAlgorithm : public Algorithm {
	protected:
		int32 beamWidth;
		virtual SearchResult PruningExecute(const Game& game);
	public:
		struct BeamSearchData {
			double evaluatedScore;
			Point first_dir[8] = {};
			Action first_act[8] = {};
			std::pair<Team, Team> teams;
			Field field;

			bool operator <(const BeamSearchData& right) const {
				return this->evaluatedScore < right.evaluatedScore;
			}
			bool operator >(const BeamSearchData& right) const {
				return this->evaluatedScore > right.evaluatedScore;
			}
		};
		BeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
		virtual SearchResult execute(const Game& game) override;
		virtual void initilize(const Game& game) override;

		std::unique_ptr<PruneBranchesAlgorithm> pruneBranchesAlgorithm;
	};

	constexpr int32 parallelSize = 3;

	//デバッグする
	class PrivateAlgorithm : public Algorithm {
	private:
	private:
		FilePath parameterFilePath;
		
		std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> pruneBranchesAlgorithms;
		std::unique_ptr<Algorithm> secondBeamSearchAlgorithm;
		BeamSearchParameter parameter;

		//CAUTION:ここら辺のパラメーターはinitilizeで決定します。
		//試合に依存するパラメーター（頻繁に変更の必要がある）
		//考える余地は、これらのパラメーターを追い出すべきか、、、追い出すべきではない。むしろ一ヵ所に固めるべき。変更し忘れを防ぐ
		//Agent数による。自動更新が望ましい。parameterを使わず明示的に、privateにおいては？
		size_t beam_size;
		//エージェント数による。しかし、調整してしまえば固定値。追い出すのが望ましい。
		int32 search_depth;
		//エージェント数による。しかし、調整してしまえば固定値。追い出すのが望ましい。
		int32 can_simulate_num;

	public:
		/*
		//非推奨なので消しました。
		PrivateAlgorithm(int32 beamWidth, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms,
			std::unique_ptr<Algorithm> secondAlgorithm);
		*/
		PrivateAlgorithm(FilePath parameterFile, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms,
			std::unique_ptr<Algorithm> secondAlgorithm);
		virtual SearchResult execute(const Game& game) override final;
		virtual void initilize(const Game& game) override final;
	};


	//TODO:実装NOW、目標は、土曜日には実装終わり。
	//公開フィールドのpruneBranchesAlgorithmには、初期化時にフィールド番号がわたるようにして置く。
	class PublicAlgorithm : public Algorithm {
	private:


	public:


	};

	std::pair<int32, int32> innerCalculateScoreFast(Procon30::Field& field, Procon30::TeamColor teamColor, unsigned short qFast[2000], std::bitset<1023> & visitFast);

}


