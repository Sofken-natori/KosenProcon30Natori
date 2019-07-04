#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"

namespace Procon30 {

	class Observer;

	class Game
	{
		static std::mutex HTTPWaitMtx;
		static std::condition_variable HttpWaitCond;
		static bool dataReceived;
		static std::mutex ReceiveMtx;
	private:

		void parseAgentsData(Team &team,JSONValue object);
		void parseTeamsData(JSONValue object);
		void parseActionsData(JSONValue object);

	public:
		//NEED:HTTPCommunicationから呼び出す
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
		int32 Maxturn;

		int32 startedAtUnixTime;

		//YASAI
		void dataUpdate();

		//nasatame
		//TODO : AgentIDなどを前もって聞いていないので適当実装、後で書き換える必要あり
		void parseJson(String fileName);
		void convertToJson(String fileName);

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;

		Game();
		~Game();
		
		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);
	};

}