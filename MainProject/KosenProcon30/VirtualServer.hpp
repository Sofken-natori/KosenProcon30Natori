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
		int32 agent_count = 8;
		int32 width = 20;
		int32 height = 20;

		////////////  �ΐ�p��HTTPCommunication�Ɠ����悤�ȋ@�\����������B  //////////////////

		static const int32 v_turnMillis = 1000;
		static const int32 v_intervalMillis = 1000;

		bool isStrategyStep;

	public:
		VirtualServer();
		void putPoint(int32 fieldType);
		void putAgent();
		void writeJson(FilePath path);
		void writeFieldJson(FilePath path);
		void negativePercent(int32 percent,int32 fieldType);
		int32 calculateScore(TeamColor color);

		////////////  �ΐ�p��HTTPCommunication�Ɠ����悤�ȋ@�\����������B  //////////////////

		static void VirtualServerMain();

		//��{��ɂQ
		[[nodiscard]] const size_t getMatchNum() const;

		//0->1 1->2
		[[nodiscard]] const size_t getGameIDfromGameNum(const int32& num) const;

		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;

		[[nodiscard]] std::shared_ptr<SendBuffer> getBufferPtr();

		std::shared_ptr<Observer> observer;

		//thread�ɓ��������
		void Loop();
		std::thread thisThread;

		//Main����Ăяo������
		void ThreadRun();

		void update();

		Stopwatch gameTimer;
		Stopwatch turnTimer;


		bool checkPostAction();

		String getPostData(const FilePath& filePath);

		bool posted[2];

		int turn;
	
		bool initMatch(const FilePath& filePath);
		bool parseActionData(const FilePath& filePath);

		void simulation();
	};
}
