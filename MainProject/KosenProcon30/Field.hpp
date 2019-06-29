#pragma once
#include "KosenProcon30.hpp"
#include "Agent.hpp"
#include "Tile.hpp"

namespace Procon30 {

	class Field
	{
	public:
		Grid<Tile> board;
		Size boardSize;
		void update(Array<Agent> agents);
		Field();
		~Field();
	};

}