#include "GUI.hpp"



void Procon30::GUI::draw() {
	//ゲームが開催していなときに死ぬ or 初期状態で0番目のゲーム参照になる
	SimpleGUI::RadioButtons(selectingMatchNum, viewerStrings, Vec2(40, 40));
	SimpleGUI::RadioButtons(selectingMatchNum, { U"match_{}"_fmt(0),U"match_{}"_fmt(1) }, Vec2(MaxFieldX * TileSize + 10, 10));


	//あとでけす
	font(selectingMatchNum).draw(Vec2(10, 10));

	//これ書き直すべき
	/*Point myAgentPoint = { 10,10 };
	Shape2D::NStar(8, correctTileSize * 0.45, correctTileSize * 0.35, Vec2(
		myAgentPoint.x * correctTileSize + correctTileSize / 2.0
		, myAgentPoint.y * correctTileSize + correctTileSize / 2.0)).draw();
	Shape2D::NStar(8, correctTileSize * 0.4, correctTileSize * 0.3, Vec2(
		myAgentPoint.x * correctTileSize + correctTileSize / 2.0
		, myAgentPoint.y * correctTileSize + correctTileSize / 2.0)).draw(myTeamColor);*/

}

void Procon30::GUI::dataUpdate()
{
}


Procon30::GUI::GUI()
	:observer(new Observer())
{
	//各試合に合ったタイルを作る
	for (size_t i = 0; i < observer->getStock().getMatchNum(); i++) {
		size_t width = observer->getStock(i).field.boardSize.x;
		size_t height = observer->getStock(i).field.boardSize.y;
		double correctionTileSize = (TileSize * MaxFieldX) / (Max(width, height) * TileSize + .0) * TileSize;
		teamTile[i] = Rect(correctionTileSize - 2, correctionTileSize - 2);
		viewerStrings.push_back(U"match_{}"_fmt(i));
	}
	//ボールド体でもいいですか...(すきなので)
	font = Font(50, Typeface::Bold);

	selectingMatchNum = 0;

	myTeamColor = Color(0, 255, 255, 127);
	enemyTeamColor = Color(255, 0, 0, 127);
	noneTeamColor = Color(227);
}


Procon30::GUI::~GUI()
{
}

std::shared_ptr<Procon30::Observer> Procon30::GUI::getObserver()
{
	return observer;
}
