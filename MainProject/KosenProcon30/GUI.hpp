#pragma once
#include "KosenProcon30.hpp"
#include "Observer.hpp"

namespace Procon30 {

	class GUI
	{
	private:
		//DON'T DELETE
		std::shared_ptr<Observer> observer;

		//試合ごとにサイズが変わったりする変数達
		std::array<RectF, MaxGameNumber> teamTile;
		std::array<double, MaxGameNumber> correctedTileSize;
		std::array<Font, MaxGameNumber>  scoreFont;
		Array<String> viewerStrings;


		//以下全試合を通して共通のもの

		//主にタイルの色で使用
		Color myTeamColor;
		Color enemyTeamColor;
		Color noneTeamColor;

		//表示時に選別するための変数
		size_t match;
		size_t drawType;

		//文字描画に必要な変数
		Font bigFont;
		Font smallFont;

		//表示の際にいい感じになる箱
		RectF viewerBox;

		Texture texLoser;
		Texture texEven;
		Texture texWinner;

		const double myInfoX = MaxFieldX * TileSize * 1.03;
		const double enemyInfoX = MaxFieldX * TileSize * 1.425;

		//デバッグ用
		Font test;
	public:
		void draw();


		//開催中の試合が変更されたとき
		//新規に試合が開始した際に初期データを生成したいので呼んであげてください
		void dataUpdate();

		GUI();
		~GUI();

		std::shared_ptr<Observer> getObserver();
	};

}
