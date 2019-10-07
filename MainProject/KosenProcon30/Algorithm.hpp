#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"
#include <bitset>

namespace Procon30 {

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
		virtual bool pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams);
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

	PublicField checkPublicField(const Game& game);
}