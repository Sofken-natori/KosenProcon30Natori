#include "Observer.hpp"

void Procon30::Observer::notify(int32 gameID, const Game& stock)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	(this->gamesUpdateCount.at(gameID))++;
	(this->gamesUpdateFlag.at(gameID)) = true;
	(this->games.at(gameID)) = stock;
}

void Procon30::Observer::notify(const HTTPCommunication& stock)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	(this->httpUpdateCount)++;
	(this->httpUpdateFlag) = true;
	this->http = stock;
}

const Procon30::HTTPCommunication& Procon30::Observer::getStock()
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	return http;
}

const Procon30::Game& Procon30::Observer::getStock(int32 gameNum)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	return this->games.at(gameNum);
}

const bool Procon30::Observer::getUpdateFlag()
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	if (this->httpUpdateFlag) {
		this->httpUpdateFlag = false;
		return true;
	}
	return false;
}

const bool Procon30::Observer::getUpdateFlag(int32 gameNum)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	if (this->gamesUpdateFlag.at(gameNum)) {
		this->gamesUpdateFlag.at(gameNum) = false;
		return true;
	}
	return false;
}

Procon30::Observer::Observer()
{
}
