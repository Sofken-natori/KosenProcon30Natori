#include "Game.hpp"

bool Procon30::Game::dataReceived = false;
std::mutex Procon30::Game::HTTPWaitMtx;
std::condition_variable Procon30::Game::HttpWaitCond;
std::mutex Procon30::Game::ReceiveMtx;

void Procon30::Game::HTTPReceived()
{
	std::lock_guard<std::mutex> lock(HTTPWaitMtx);
	dataReceived = true;
	HttpWaitCond.notify_all();
}

int32 Procon30::Game::calculateScore(TeamColor color)
{
	return int32();
}

void Procon30::Game::dataUpdate()
{
	//Wait•”
	std::unique_lock<std::mutex> lockWait(HTTPWaitMtx);
	while (!dataReceived) {
		HttpWaitCond.wait(lockWait);
	}

	//ˆ—•”

	//–¢óM•ÏX•”
	std::lock_guard<std::mutex> lockFlag(ReceiveMtx);
	dataReceived = false;
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
	this->isSearchfinished = right.isSearchfinished;

	return (*this);
}
