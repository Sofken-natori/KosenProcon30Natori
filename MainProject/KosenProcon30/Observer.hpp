#pragma once
#include "KosenProcon30.hpp"
#include "Game.hpp"
#include "HTTPCommunication.hpp"

constexpr int32 observerStockMAX = 10;

namespace Procon30 {
	class Observer
	{
	private:
		//TODO: thread partial lock.
		//��������S�Ẵ��\�[�X�����b�N����悤�ɂ���B����Ńo�O��Ȃ��͂��B
		std::mutex resourceMtx;


		HTTPCommunication http;
		std::array<Game, observerStockMAX> games;

	public:
		void notify(int32 gameID,const Game &stock);
		void notify(const HTTPCommunication &stock);

		const HTTPCommunication& getStock();
		const Game& getStock(int32 gameNum);

		Observer();
	};
}
