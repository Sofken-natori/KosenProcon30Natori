#include "Observer.hpp"
#include "Game.hpp"

int32 Procon30::Game::calculateScore(TeamColor color)
{
	return int32();
}

void Procon30::Game::dataUpdate()
{
	
}

void Procon30::Game::parseJson(String fileName)
{
}

void Procon30::Game::convertToJson(String fileName)
{
}

Procon30::Game::Game()
{

}


Procon30::Game::~Game()
{
}

Procon30::Game& Procon30::Game::operator=(const Procon30::Game& right)
{
	this->field = right.field;
	this->gameTimer = right.gameTimer;
	this->turn = right.turn;
	this->Maxturn = right.Maxturn;
	this->turnTimer = right.turnTimer;

	return (*this);
}
