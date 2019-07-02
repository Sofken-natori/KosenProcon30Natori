#pragma once
#include <utility>

#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"

namespace Procon30 {

	class Observer;

	class Game
	{
	public:
		Field field;

		//first: MyTeam
		//second: EnemyTeam
		std::pair<Team, Team> teams;


		int32 turn;
		int32 Maxturn;

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;

		Game();
		~Game();
		
		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);
	};

}