#pragma once
#include "KosenProcon30.hpp"
#include "Observer.hpp"

namespace Procon30 {

	class GUI
	{
	private:
		//DON'T DELETE
		std::shared_ptr<Observer> observer;

		Rect teamTile[3];
		Color myTeamColor;
		Color enemyTeamColor;
		Color noneTeamColor;
		Color backGroundColor;

		Array<String> viewerStrings;
		size_t selectingMatchNum;
		Font font;
	public:

		void draw();
		void dataUpdate();

		GUI();
		~GUI();

		std::shared_ptr<Observer> getObserver();
	};

}
