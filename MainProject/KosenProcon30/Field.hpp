#pragma once
#include "KosenProcon30.hpp"
#include "Agent.hpp"
#include "Tile.hpp"

namespace Procon30 {

	class Field
	{
	public:
		using TileGrid = Grid<Tile>;

		TileGrid m_board{ MaxFieldX,MaxFieldY };
		Size boardSize;
		Field();
		~Field();
	};

}