#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"
#include "HTTPCommunication.hpp"

constexpr int32 observerStockMAX = 10;

namespace Procon30 {
	class Observer
	{
	private:
		//TODO: thread lock.
		HTTPCommunication http;
		Game game[observerStockMAX];
	public:
		void notify(int32 game_id,const Game &stock);
		void notify(const HTTPCommunication &stock);

		const HTTPCommunication& getStock();
		const Game& getStock(int32 game_id);
	};
}
