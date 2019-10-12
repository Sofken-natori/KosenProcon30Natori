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
		//canSimulateNum��1�G�[�W�F���g������̗񋓉\���AenumerateDir�ɒT���������������B�I�[�́A-1,-1�ɂ��āB
		virtual bool pruneBranches(const int canSimulateNum, std::array<std::array<Point, 10>, 8> & enumerateDir, const Field& field, const std::pair<Team, Team>& teams) const;
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

	//�f�o�b�O����
	class ProconAlgorithm : public Algorithm {
	private:

		struct Book {
			//����̃G�[�W�F���g�̈ʒu�B�p�^�[���̔��ʂɎg���܂��B
			Point firstPos;
			Point secondPos;

			int32 pattern;

			//pos.x==-1�͎w��Ȃ��B�ǂ������Ă�����agent�́Apos.x==-1�ɂ��Ă����Ă��������Bpos.x==-1�œǂݔ�΂��̂ŁA���͓K���ł����ł��B
			//trun agent (point,point) == (���݈ʒu�A����)
			std::array<std::array<std::pair<Point, Point>, 8>, 20> firstData;
			std::array<std::array<std::pair<Point, Point>, 8>, 20> secondData;

			int32 readTurn;

			/*
			//��΃t�@�C������
			//�����l�́Apos.x == -1, pos.y == -1�ł��B�ǂ����������ł������ł��B
			5 4
			3 2
			3
			2
			3 2 1 -1 5 3 0 1  5  0 1 0
			4 1 1  1 5 4 1 0 -1 -1 0 0

			firstPos.x firstPos.y
			secondPos.x secondPos.y
			agent_num
			read_turn
			pattern1_turn1_line => agent[0].pos.x pos.y dir.x dir.y agent[1].pos.x pos.y dir.x dir.y agent[2].pos.x pos.y dir.x dir.y
			pattern1_turn2_line => agent[0].pos.x pos.y dir.x dir.y agent[1].pos.x pos.y dir.x dir.y agent[2].pos.x pos.y dir.x dir.y
			pattern2_turn1_line =>
			pattern2_turn2_line =>
			*/
		};

		//��΃f�[�^
		Book book;

		FilePath parameterFilePath;

		std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> pruneBranchesAlgorithms;
		std::unique_ptr<Algorithm> secondBeamSearchAlgorithm;
		BeamSearchParameter parameter;

		//CAUTION:������ӂ̃p�����[�^�[��initilize�Ō��肵�܂��B
		//�����Ɉˑ�����p�����[�^�[�i�p�ɂɕύX�̕K�v������j
		//�l����]�n�́A�����̃p�����[�^�[��ǂ��o���ׂ����A�A�A�ǂ��o���ׂ��ł͂Ȃ��B�ނ���ꃕ���Ɍł߂�ׂ��B�ύX���Y���h��
		//Agent���ɂ��B�����X�V���]�܂����Bparameter���g�킸�����I�ɁAprivate�ɂ����ẮH
		size_t beam_size;
		//�G�[�W�F���g���ɂ��B�������A�������Ă��܂��ΌŒ�l�B�ǂ��o���̂��]�܂����B
		int32 search_depth;
		//�G�[�W�F���g���ɂ��B�������A�������Ă��܂��ΌŒ�l�B�ǂ��o���̂��]�܂����B
		int32 can_simulate_num;

	public:
		/*
		//�񐄏��Ȃ̂ŏ����܂����B
		PrivateAlgorithm(int32 beamWidth, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms,
			std::unique_ptr<Algorithm> secondAlgorithm);
		*/
		ProconAlgorithm(FilePath parameterFile, std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBAlgorithms,
			std::unique_ptr<Algorithm> secondAlgorithm);
		virtual SearchResult execute(const Game& game) override final;
		virtual void initilize(const Game& game) override final;
	};

	std::pair<int32, int32> innerCalculateScoreFast(Procon30::Field& field, Procon30::TeamColor teamColor, unsigned short qFast[2000], std::bitset<1023> & visitFast);

}


