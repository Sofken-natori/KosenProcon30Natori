#pragma once
#include <memory>
#include "../KosenProcon30.hpp"
#include "../Game.hpp"
#include "../Algorithm.hpp"

namespace Procon30::SUZUKI {

	//ASAP:���݃r�[���T�[�`�����̂��߁A���E������
	class AlternatelyBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
	public:
		AlternatelyBeamSearchAlgorithm(int32 beamWidth, std::unique_ptr<PruneBranchesAlgorithm> pruneBranches = nullptr);
		SearchResult execute(const Game& game) override final;
	};

	//���O���łł������̍H�v�������������킹�邽�߂̃N���X
	class SuzukiBeamSearchAlgorithm : public BeamSearchAlgorithm {
	private:
		FilePath parameterFilePath;

		//CAUTION:������ӂ̃p�����[�^�[��initilize�Ō��肵�܂��B
		//�����Ɉˑ�����p�����[�^�[�i�p�ɂɕύX�̕K�v������j
		size_t beam_size;
		//TODO:�������I�������ǂ��o��
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

	//TODO:PruneAlgorithm�����ꒃ��������P�B���Ȃ��Ƃ��A������ƌ��z�œ_������悤�ɁB�����ƍ����N������Ă����͂��B

}