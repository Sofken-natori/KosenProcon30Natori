#pragma once
#include "../KosenProcon30.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30::SUZUKI {

	//ASAP:交互ビームサーチ実装のため、求・高速化
	class AlternatelyBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
		int32 beamWidth;
	public:
		AlternatelyBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
		SearchResult execute(const Game& game) override final;
	};

	//自前環境でできる限りの工夫を試したたかわせるためのクラス
	class SuzukiBeamSearchAlgorithm : public BeamSearchAlgorithm {
	public:
		SuzukiBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
	
		SearchResult execute(const Game& game) override final;
		SearchResult PruningExecute(const Game& game) override final;
	};

	//TODO:PruneAlgorithm無茶苦茶だから改善。少なくとも、きちんと勾配で点数つけるように。

}