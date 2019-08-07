#include "Field.hpp"


Procon30::Field::Field()
{
	this->m_board = TileGrid(20,20);
	this->boardSize = Size(0,0);
}


Procon30::Field::~Field()
{
}
