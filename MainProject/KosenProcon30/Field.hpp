#pragma once
#include "KosenProcon30.hpp"
#include "Agent.hpp"
#include "Tile.hpp"

namespace Procon30 {

	class Field
	{

		using TileGrid = Grid<Tile>;

	public:
		TileGrid m_board{ MaxFieldX,MaxFieldY };
		Size boardSize;
		Field();
		~Field();
	};

}