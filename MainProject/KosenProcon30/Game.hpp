#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"
#include "SendBuffer.hpp"
#include "Algorithm.hpp"

namespace Procon30 {

	class Observer;
	class Algorithm;

	class Game
	{
	private:
		static std::mutex HTTPWaitMtx;
		static std::condition_variable HTTPWaitCond;
		static bool dataReceived;
		static std::mutex ReceiveMtx;

		//下の3つ整合性チェックが必要か議論あり
		void parseAgentsData(Team& team, JSONValue object);
		void parseTeamsData(JSONValue object);
		void parseActionsData(JSONValue object);
		std::thread thisThread;
		
	public:
		//NEED:HTTPCommunicationから呼び出す
		//これ思ったんだけど個別に起こしたほうが死ににくくて良い感じじゃないのか.jp
		static void HTTPReceived();

		Field field;

		//first: MyTeam
		//second: EnemyTeam
		std::pair<Team, Team> teams;

		bool isSearchFinished;

		//nasatame
		int32 calculateScore(TeamColor color);
		int32 calculateTileScore(TeamColor color);

		int32 turn;
		int32 MaxTurn;
		//任意に決まるあれ
		int32 gameID;
		//0,1,2...
		int32 gameNum;
		uint64 startedAtUnixTime;
		String matchTo;
		int32 turnMillis;
		int32 intervalMillis;

		//YASAI
		void updateData();
		void sendToHTTP();
		//threadに投げるもの
		void Loop();

		//Mainから呼び出すもの
		void ThreadRun();

		void parseJson(FilePath fileName);

		//actionデータをjsonにパースするだけの機能しか持たない。
		void convertToJson(FilePath fileName);

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;
		std::shared_ptr<SendBuffer> buffer;
		std::shared_ptr<std::atomic<bool>> programEnd;

		//初期化時にteamID設定しておく。 =L Teamのコンストラクタでやってる。
		Game();
		~Game();

		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);

		std::unique_ptr<Algorithm> algorithm;

		PublicField fieldType;
	};

}