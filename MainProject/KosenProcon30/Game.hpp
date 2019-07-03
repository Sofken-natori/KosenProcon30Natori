#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"

namespace Procon30 {

	class Observer;

	class Game
	{

	public:
		

		Field field;

		//first: MyTeam
		//second: EnemyTeam
		std::pair<Team, Team> teams;

		bool isSearchfinished;

		//nasatame
		int32 calculateScore(TeamColor color);
		int32 calculateTileScore(TeamColor color);

		int32 turn;
		int32 Maxturn;

		//YASAI
		void dataUpdate();

		//nasatame
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