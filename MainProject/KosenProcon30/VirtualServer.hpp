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

		////////////  対戦用にHTTPCommunicationと同じような機能を持たせる。  //////////////////

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

		////////////  対戦用にHTTPCommunicationと同じような機能を持たせる。  //////////////////

		static void VirtualServerMain(FilePath matchField = U"json/VirtualServer/matchField.json");

		//基本常に２
		[[nodiscard]] const size_t getMatchNum() const;

		//0->1 1->2
		[[nodiscard]] const size_t getGameIDfromGameNum(const int32& num) const;

		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;

		[[nodiscard]] std::shared_ptr<SendBuffer> getBufferPtr();

		std::shared_ptr<Observer> observer;

		std::shared_ptr<std::atomic<bool>> programEnd;

		//threadに投げるもの
		void Loop();
		std::thread thisThread;

		//Mainから呼び出すもの
		void ThreadRun();

		void update();

		Stopwatch gameTimer;
		Stopwatch turnTimer;

		//対戦ファイル保存場所
		FilePath matchField;

		//ログ保存先フォルダ名
		FilePath logFolderName;


		bool checkPostAction();

		String getPostData(const FilePath& filePath);

		int32 posted[2];

		int turn;
	
		bool initMatch(const FilePath& filePath);
		bool parseActionData(const FilePath& filePath);

		void simulation();

		struct LogData {
			//ターン数, チーム1タイル点数,エリア点数, チーム２タイル点数,エリア点数, チーム１stayが発生した数（衝突によるものも含む）, チーム2stayが発生した数, チーム1のpostされたms, チーム2のpostされたms
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
