#include "Observer.hpp"

void Procon30::Observer::notify(int32 game_id, const Game& stock)
{
	(this->game[game_id]) = stock;
}

void Procon30::Observer::notify(const HTTPCommunication& stock)
{
	this->http = stock;
}

inline const Procon30::HTTPCommunication& Procon30::Observer::getStock()
{
	return http;
}

inline const Procon30::Game& Procon30::Observer::getStock(int32 game_id)
{
	return this->game[game_id];
}