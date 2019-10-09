#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"
#include "HTTPCommunication.hpp"

constexpr int32 observerStockMAX = 10;

namespace Procon30 {
	class Observer
	{
	private:
		//��������S�Ẵ��\�[�X�����b�N����悤�ɂ���B����Ńo�O��Ȃ��͂��B
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

		//CATION:��U�ǂ݂�������t���O�����̂Œ���
		const bool getUpdateFlag();
		const bool getUpdateFlag(int32 gameNum);

		Observer();
	};
}
