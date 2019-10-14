#pragma once
#include "KosenProcon30.hpp"
#include "Field.hpp"
#include "Team.hpp"
#include "Agent.hpp"

namespace Procon30 {
	
	class SendBuffer;
	class Observer;

	class VirtualServer
	{
		Field field;
		Grid<int32> tile{ MaxFieldX,MaxFieldY };
		std::pair<Team,Team> teams;

		/*Array<Array<int32>> points;
		Array<Array<int32>> tiles;
		Array<Array<int32>> agents1;
		Array<Array<int32>> agents2;*/
		int32 agent_count;
		int32 width = 20;
		int32 height = 20;
		int32 negative_percent = 0;

		////////////  �ΐ�p��HTTPCommunication�Ɠ����悤�ȋ@�\����������B  //////////////////

		static const int32 v_turnMillis = 10000;
		static const int32 v_intervalMillis = 1000;
		static const int32 v_MaxTurn = 50;

		bool isStrategyStep;

	public:
		VirtualServer(int32 field_type = 3);
		void putPoint(int32 fieldType);
		void putAgent(int32 fieldType);
		void writeJson(FilePath path);
		void writeFieldJson(FilePath path);
		void negativePercent(int32 percent,int32 fieldType);
		int32 calculateScore(TeamColor color);

		////////////  �ΐ�p��HTTPCommunication�Ɠ����悤�ȋ@�\����������B  //////////////////

		static void VirtualServerMain(FilePath matchField = U"json/VirtualServer/matchField.json");

		//��{��ɂQ
		[[nodiscard]] const size_t getMatchNum() const;

		//0->1 1->2
		[[nodiscard]] const size_t getGameIDfromGameNum(const int32& num) const;

		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;

		[[nodiscard]] std::shared_ptr<SendBuffer> getBufferPtr();

		std::shared_ptr<Observer> observer;

		std::shared_ptr<std::atomic<bool>> programEnd;

		//thread�ɓ��������
		void Loop();
		std::thread thisThread;

		//Main����Ăяo������
		void ThreadRun();

		void update();

		Stopwatch gameTimer;
		Stopwatch turnTimer;

		//�ΐ�t�@�C���ۑ��ꏊ
		FilePath matchField;

		//���O�ۑ���t�H���_��
		FilePath logFolderName;


		bool checkPostAction();

		String getPostData(const FilePath& filePath);

		int32 posted[2];

		int turn;
	
		bool initMatch(const FilePath& filePath);
		bool parseActionData(const FilePath& filePath);

		void simulation();

		struct LogData {
			//�^�[����, �`�[��1�^�C���_��,�G���A�_��, �`�[���Q�^�C���_��,�G���A�_��, �`�[���Pstay�������������i�Փ˂ɂ����̂��܂ށj, �`�[��2stay������������, �`�[��1��post���ꂽms, �`�[��2��post���ꂽms
			int32 firstTileScore = 0;
			int32 firstAreaScore = 0;
			int32 secondTileScore = 0;
			int32 secondAreaScore = 0;
			int32 firstStayNum = 0;
			int32 secondStayNum = 0;
			int32 firstPostMS = -1;
			int32 secondPostMS = -1;
		};

		Array<LogData> logData;
	};
}
