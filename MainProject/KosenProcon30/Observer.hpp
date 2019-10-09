#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"
#include "HTTPCommunication.hpp"

constexpr int32 observerStockMAX = 10;

namespace Procon30 {
	class Observer
	{
	private:
		//いったん全てのリソースをロックするようにする。これでバグらないはず。
		std::mutex resourceMtx;

		int32 httpUpdateCount;
		std::array<int32, observerStockMAX> gamesUpdateCount;

		bool httpUpdateFlag;
		std::array<bool, observerStockMAX> gamesUpdateFlag;

		HTTPCommunication http;
		std::array<Game, observerStockMAX> games;

	public:
		void notify(int32 gameID, const Game& stock);
		void notify(const HTTPCommunication& stock);

		const HTTPCommunication& getStock();
		const Game& getStock(int32 gameNum);

		//CATION:一旦読みだしたらフラグ消すので注意
		const bool getUpdateFlag();
		const bool getUpdateFlag(int32 gameNum);

		Observer();
	};
}
