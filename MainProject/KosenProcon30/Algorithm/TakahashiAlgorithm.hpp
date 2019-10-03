#pragma once
#include "../KosenProcon30.hpp"
#include "../Field.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30 {
	namespace YASAI {
		class CompressBranch : public PruneBranchesAlgorithm {
		private:
			Size fieldSize;
			std::array<std::array<double, 20>, 20> weightSum;
			std::array<double, 21> distanceWeight;
			double weight;
		public:
			//weight:　マスの距離に対する重さ(weight^-n)
			CompressBranch(double weight);
			virtual void initilize(const Game& game) override;
			virtual bool pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, Field& field, std::pair<Team, Team> teams) override;
		};
	}
}
