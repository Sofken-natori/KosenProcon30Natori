#pragma once
#include "../KosenProcon30.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30::SUZUKI {

	//ASAP:���݃r�[���T�[�`�����̂��߁A���E������
	class AlternatelyBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
		int32 beamWidth;
	public:
		AlternatelyBeamSearchAlgorithm(int32 beamWidth);
		SearchResult execute(const Game& game);
	};

	//���O���łł������̍H�v�������������킹�邽�߂̃N���X
	class SuzukiBeamSearchAlgorithm : public BeamSearchAlgorithm {
	public:
		SuzukiBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
	
		SearchResult execute(const Game& game);
		SearchResult PruningExecute(const Game& game);
	};

}