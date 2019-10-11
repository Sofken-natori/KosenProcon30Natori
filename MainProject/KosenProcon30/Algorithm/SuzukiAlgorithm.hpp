#pragma once
#include <memory>
#include "../KosenProcon30.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30::SUZUKI {

	//ASAP:交互ビームサーチ実装のため、求・高速化
	class AlternatelyBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
	public:
		AlternatelyBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
		SearchResult execute(const Game& game) override final;
	};

	//自前環境でできる限りの工夫を試したたかわせるためのクラス
	class SuzukiBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
		FilePath parameterFilePath;

		//CAUTION:ここら辺のパラメーターはinitilizeで決定します。
		//試合に依存するパラメーター（頻繁に変更の必要がある）
		size_t beam_size;
		//TODO:調整が終わったら追い出す
		int32 search_depth;
		int32 can_simulate_num;

		std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> pruneBranchesAlgorithms;

	public:
		//SuzukiBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
		SuzukiBeamSearchAlgorithm(FilePath parameterFile, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithm);
	
		virtual void initilize(const Game& game) override final;
		SearchResult execute(const Game& game) override final;
		SearchResult PruningExecute(const Game& game) override final;
		BeamSearchParameter parameter;
	};

	//TODO:PruneAlgorithm無茶苦茶だから改善。少なくとも、きちんと勾配で点数つけるように。きっと高橋君がやってくれるはず。

}