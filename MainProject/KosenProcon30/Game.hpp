#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include <Siv3D.hpp>
#include <utility>
namespace Procon30 {

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
		Game();
		~Game();
	};

}