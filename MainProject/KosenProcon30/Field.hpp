#pragma once
#include "KosenProcon30.hpp"
#include "Agent.hpp"
#include "Tile.hpp"

namespace Procon30
{

	using TileGrid = Grid<Tile>;

	class Field
	{
	public:
		TileGrid m_board{ MaxFieldX,MaxFieldY };
		Size boardSize;
		Field();
		~Field();
	};

}