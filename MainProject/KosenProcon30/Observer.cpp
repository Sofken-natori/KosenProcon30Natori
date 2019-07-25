#include "Observer.hpp"

void Procon30::Observer::notify(int32 gameID, const Game& stock)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	(this->games.at(gameID)) = stock;
}

void Procon30::Observer::notify(const HTTPCommunication& stock)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	this->http = stock;
}

inline const Procon30::HTTPCommunication& Procon30::Observer::getStock()
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	return http;
}

inline const Procon30::Game& Procon30::Observer::getStock(int32 gameID)
{
	std::lock_guard<std::mutex> lock(this->resourceMtx);
	return this->games.at(gameID);
}

Procon30::Observer::Observer()
{
}
