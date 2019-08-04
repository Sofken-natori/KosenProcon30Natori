#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"
#include "SendBuffer.hpp"

namespace Procon30 {

	class Observer;

	class Game
	{
	private:
		static std::mutex HTTPWaitMtx;
		static std::condition_variable HTTPWaitCond;
		static bool dataReceived;
		static std::mutex ReceiveMtx;

		void parseAgentsData(Team& team, JSONValue object);
		void parseTeamsData(JSONValue object);
		void parseActionsData(JSONValue object);

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
		int32 startedAtUnixTime;
		String matchTo;
		int32 turnMillis;

		//YASAI
		void updateData();
		void sendToHTTP();

		//nasatame
		//TODO : AgentIDなどを前もって聞いていないので適当実装、後で書き換える必要あり
		void parseJson(FilePath fileName);

		//actionデータをjsonにパースするだけの機能しか持たない。
		void convertToJson(FilePath fileName);

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;
		std::shared_ptr<SendBuffer> buffer;

		//TODO:初期化時にteamID設定しておく。
		Game();
		~Game();

		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);
	};

}