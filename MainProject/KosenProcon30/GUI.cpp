#include "GUI.hpp"



void Procon30::GUI::draw() {
	//データが入ってないので仮置きですが
	Field field = observer->getStock(0).field;

	//仮置き盤面値（受け取る値
	const int width = 17;
	const int height = 17;

	//チームを判別します（受け取る値
	int myTeamID = 1;
	int enemyTeamID = 2;

	//仮置きのタイル配置
	std::array < std::array<int, width>, height> tiles;
	for (int i = 0; i < height; i++)tiles[i][i] = tiles[height - i - 1][i] = 1;

	//範囲にちゃんと入るようにする補正値
	double correctionValue = (TileSize * MaxFieldX) / (Max(height, width) * TileSize + .0);
	//正規版
	//double correctionValue = (TileSize * MaxFieldX) / (Max(field.boardSize.x, field.boardSize.y) * TileSize + .0);

	//補正した場合のタイルサイズ
	int correctTileSize = TileSize * correctionValue;
	//補正を効かせたタイル
	//privに書いたほうがよさそう
	Rect correctTile = Rect(correctTileSize - 2, correctTileSize - 2);

	//動かす場所が同じ
	//drawする色だけ違う場合とか短く書けないですかね
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			if (tiles[i][j] == myTeamID) {
				correctTile.movedBy(i * correctTileSize + 1, j * correctTileSize + 1).draw(myTeamColor);
			}
			else if (tiles[i][j] == enemyTeamID) {
				correctTile.movedBy(i * correctTileSize + 1, j * correctTileSize + 1).draw(enemyTeamColor);
			}
			else {
				correctTile.movedBy(i * correctTileSize + 1, j * correctTileSize + 1).draw(noneTeamColor);
			}
		}
	}
	//これ書き直すべき
	Point myAgentPoint = { 10,10 };
	Shape2D::NStar(8, correctTileSize * 0.45, correctTileSize * 0.35, Vec2(
		myAgentPoint.x * correctTileSize + correctTileSize / 2.0
		, myAgentPoint.y * correctTileSize + correctTileSize / 2.0)).draw();
	Shape2D::NStar(8, correctTileSize * 0.4, correctTileSize * 0.3, Vec2(
		myAgentPoint.x * correctTileSize + correctTileSize / 2.0
		, myAgentPoint.y * correctTileSize + correctTileSize / 2.0)).draw(myTeamColor);

}

void Procon30::GUI::dataUpdate()
{
}


Procon30::GUI::GUI()
	:observer(new Observer())
{
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
