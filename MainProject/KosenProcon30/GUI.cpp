#include "GUI.hpp"



void Procon30::GUI::draw() {
	//ゲームが信仰していなときに死ぬ or 初期状態で0番目のゲーム参照になる
	SimpleGUI::RadioButtons(match, viewerStrings, Vec2(MaxFieldX * TileSize + 10, 10));
	SimpleGUI::RadioButtons(drawType, { U"タイル + エージェント",U"タイル + 点数" }, Vec2(MaxFieldX * TileSize + 200, 10));

	//要テスト
	//ここで盤面描画する
	for (size_t y : step(observer->getStock((int32)match).field.boardSize.y)) {
		for (size_t x : step(observer->getStock((int32)match).field.boardSize.x)) {
			Procon30::TeamColor ID = observer->getStock((int32)match).field.m_board[y][x].color;
			if (ID == observer->getStock((int32)match).teams.first.color) {
				teamTile[match].movedBy(x * correctedTileSize[match] + 1, y * correctedTileSize[match] + 1).draw(myTeamColor);
			}
			else if (ID == observer->getStock((int32)match).teams.second.color) {
				teamTile[match].movedBy(x * correctedTileSize[match] + 1, y * correctedTileSize[match] + 1).draw(enemyTeamColor);
			}
			else {
				teamTile[match].movedBy(x * correctedTileSize[match] + 1, y * correctedTileSize[match] + 1).draw(noneTeamColor);
			}
			//点数描画が必要ならする
			if (drawType == 1) {
				font(observer->getStock((int32)match).field.m_board[y][x].score).draw(x * correctedTileSize[match] + 1, y * correctedTileSize[match] + 1);

			}
		}
	}

	//エージェントの描画
	if (drawType == 0) {
		for (Agent agent : observer->getStock((int32)match).teams.first.agents) {
			
		}
	}


	//あとでけす
	font(match).draw(Vec2(10, 10));
	font(drawType).draw(Vec2(50, 10));

	//場所は本当にここでええんか
	font(U"現在のターン : " + observer->getStock((int32)match).turn).draw(Vec2(MaxFieldX * TileSize + 20, 150), Palette::White);


	//ここから各情報について載せていきたい所存

	//本当にこれでいいのかテストします
	const int height = 20;
	const int width = 20;

	std::array<std::array<int, width>, height> tiles;
	for (int i : step(height))
		for (int j : step(width))tiles[i][j] = 0;

	double cts = (TileSize * MaxFieldX + .0) / (Max(width, height) * TileSize + .0) * TileSize;

	RectF dtile(cts * 0.96, cts * 0.96);

	for (int i : step(height))
		for (int j : step(width)) {

			dtile.movedBy(i * cts + cts * 0.02, j * cts + cts * 0.02).draw(myTeamColor);
			if (drawType == 1) {
				test(U"-16").drawAt(Vec2((i + 0.5) * cts, (j + 0.5) * cts), Palette::Black);
			}
		}

	std::array<Vec2, 10> agents;
	for (int i : step(10))agents[i] = Vec2(i, i);

	if (drawType == 0) {
		for (int i = 0; i < 10; i++) {
			Shape2D::NStar(8, cts * 0.45, cts * 0.35, Vec2((agents[i].x + 0.5) * cts, (agents[i].y + 0.5) * cts)).draw();
			Shape2D::NStar(8, cts * 0.375, cts * 0.275, Vec2((agents[i].x + 0.5) * cts, (agents[i].y + 0.5) * cts)).draw(myTeamColor);
		}
	}
}

void Procon30::GUI::dataUpdate()
{
	//追加で開始されるetcが発生した場合、match_1がmatch_0にスライドされちゃうです

	viewerStrings.clear();

	for (size_t i = 0; i < observer->getStock().getMatchNum(); i++) {
		size_t width = observer->getStock((int32)i).field.boardSize.x;
		size_t height = observer->getStock((int32)i).field.boardSize.y;
		correctedTileSize[i] = (TileSize * MaxFieldX + .0) / (Max(width, height) * TileSize + .0) * TileSize;

		teamTile[i] = RectF(correctedTileSize[i] * 0.96, correctedTileSize[i] * 0.96);

		scoreFont[i] = Font(correctedTileSize[i] * 0.46);

		viewerStrings.push_back(U"match_{}"_fmt(i));
	}
}


Procon30::GUI::GUI()
	:observer(new Observer())
{
	//色変えたい場合はここ
	myTeamColor = Color(64, 191, 191);
	enemyTeamColor = Color(192, 128, 128);
	noneTeamColor = Color(227);

	match = 0;
	drawType = 0;

	font = Font(30, Typeface::Bold);

	viewerBox = Rect();

	test = Font(50 * 0.46);
}


Procon30::GUI::~GUI()
{
}

std::shared_ptr<Procon30::Observer> Procon30::GUI::getObserver()
{
	return observer;
}
