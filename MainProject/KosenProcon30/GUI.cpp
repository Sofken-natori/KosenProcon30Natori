#include "GUI.hpp"



void Procon30::GUI::draw() {
	//ゲームが信仰していなときに死ぬ or 初期状態で0番目のゲーム参照になる
	SimpleGUI::RadioButtons(match, viewerStrings, Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.01));
	SimpleGUI::RadioButtons(drawType, { U"タイル + エージェント",U"タイル + 点数" }, Vec2(MaxFieldX * TileSize * 1.2, MaxFieldY * TileSize * 0.01));

	if (observer->getUpdateFlag((int32)match)) {
		dataUpdate();
	}

	//TODO:Gameのコピーコンストラクタを作ってほしいかも。
	Game game;
	game = observer->getStock((int32)match);

	//ここで盤面描画する
	for (size_t y : step(game.field.boardSize.y)) {
		for (size_t x : step(game.field.boardSize.x)) {
			Procon30::TeamColor tileColor = game.field.m_board[y][x].color;
			Vec2 pos = { (x + 0.02) * correctedTileSize[match] ,(y + 0.02) * correctedTileSize[match] };
			if (tileColor == game.teams.first.color) {
				teamTile[match].movedBy(pos).draw(myTeamColor);
			}
			else if (tileColor == game.teams.second.color) {
				teamTile[match].movedBy(pos).draw(enemyTeamColor);
			}
			else {
				teamTile[match].movedBy(pos).draw(noneTeamColor);
			}
			//点数描画が必要ならする
			if (drawType == 1) {
				//文字色切り離したほうがよさげかも
				scoreFont[match](game.field.m_board[y][x].score).drawAt(Vec2((x + 0.5) * correctedTileSize[match], (y + 0.5) * correctedTileSize[match]), Palette::Black);
			}
		}
	}

	//エージェントの描画
	if (drawType == 0) {
		//自分
		for (Agent agent : game.teams.first.agents) {
			//ポリゴン生成めんどいしShape2D::NStarでいいですか
			//その場生成しかできない代わりに軽いらしいので
			Vec2 pos = { (agent.nowPosition.x + 0.5) * correctedTileSize[match],(agent.nowPosition.y + 0.5) * correctedTileSize[match] };
			Shape2D::NStar(8, correctedTileSize[match] * 0.45, correctedTileSize[match] * 0.35, pos).draw();
			Shape2D::NStar(8, correctedTileSize[match] * 0.375, correctedTileSize[match] * 0.275, pos).draw(myTeamColor);
		}
		//相手
		for (Agent agent : game.teams.second.agents) {
			Vec2 pos = { (agent.nowPosition.x + 0.5) * correctedTileSize[match],(agent.nowPosition.y + 0.5) * correctedTileSize[match] };
			Shape2D::NStar(8, correctedTileSize[match] * 0.45, correctedTileSize[match] * 0.35, pos).draw();
			Shape2D::NStar(8, correctedTileSize[match] * 0.375, correctedTileSize[match] * 0.275, pos).draw(enemyTeamColor);
		}
	}


	bigFont(U"現在のターン : ", game.turn).draw(Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.15), Palette::White);

	//絵文字表示
	//ここ要りますか（ぇ
	if (game.teams.first.score > game.teams.second.score + 50) {
		texWinner.resized(90).draw(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.19);
	}
	else if (game.teams.first.score + 50 < game.teams.second.score) {
		texLoser.resized(90).draw(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.19);
	}
	else {
		texEven.resized(90).draw(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.19);
	}


	//左側情報欄
	viewerBox.rounded(10).movedBy(Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.28)).draw();
	viewerBox.rounded(10).movedBy(Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.28)).drawFrame(10, 0, myTeamColor);

	viewerBox.rounded(10).movedBy(Vec2(MaxFieldX * TileSize * 1.405, MaxFieldY * TileSize * 0.28)).draw();
	viewerBox.rounded(10).movedBy(Vec2(MaxFieldX * TileSize * 1.405, MaxFieldY * TileSize * 0.28)).drawFrame(10, 0, enemyTeamColor);

	//自チーム情報
	bigFont(U"MyTeam").draw(Vec2(myInfoX, MaxFieldY * TileSize * 0.30), myTeamColor);
	bigFont(U"Score : {}"_fmt(game.teams.first.score)).draw(Vec2(myInfoX, MaxFieldY * TileSize * 0.34), myTeamColor);
	smallFont(U"  TileScore  : {}"_fmt(game.teams.first.tileScore)).draw(Vec2(myInfoX, MaxFieldY * TileSize * 0.385), myTeamColor);
	smallFont(U"  AreaScore : {}"_fmt(game.teams.first.areaScore)).draw(Vec2(myInfoX, MaxFieldY * TileSize * 0.425), myTeamColor);

	bigFont(U"Agents").draw(Vec2(myInfoX, MaxFieldY * TileSize * 0.48), myTeamColor);

	for (int i : step(game.teams.first.agentNum)) {
		smallFont(U"ID {0} , pos {1} → {2} "_fmt(
			game.teams.first.agents[i].agentID,
			game.teams.first.agents[i].nowPosition,
			game.teams.first.agents[i].nextPosition)).draw(Vec2(myInfoX, MaxFieldY * TileSize * (0.53 + i * 0.04)), myTeamColor);
	}

	//相手チーム情報
	bigFont(U"EnemyTeam").draw(Vec2(enemyInfoX, MaxFieldY * TileSize * 0.30), enemyTeamColor);
	bigFont(U"Score : {}"_fmt(game.teams.second.score)).draw(Vec2(enemyInfoX, MaxFieldY * TileSize * 0.34), enemyTeamColor);
	smallFont(U"  TileScore : {} "_fmt(game.teams.second.tileScore)).draw(Vec2(enemyInfoX, MaxFieldY * TileSize * 0.385), enemyTeamColor);
	smallFont(U"  AreaScore : {}"_fmt(game.teams.second.areaScore)).draw(Vec2(enemyInfoX, MaxFieldY * TileSize * 0.425), enemyTeamColor);

	bigFont(U"Agents").draw(Vec2(enemyInfoX, MaxFieldY * TileSize * 0.48), enemyTeamColor);

	for (int i : step(game.teams.second.agentNum)) {
		smallFont(U"ID {0} , pos {1} → {2} "_fmt(
			game.teams.second.agents[i].agentID,
			game.teams.second.agents[i].nowPosition,
			game.teams.second.agents[i].nextPosition)).draw(Vec2(enemyInfoX, MaxFieldY * TileSize * (0.53 + i * 0.04)), enemyTeamColor);
	}
	//for (int i : step(game.teams.second.agentNum)) {
	//	smallFont(agentInfo[match].second[i]).draw(Vec2(enemyInfoX, MaxFieldY * TileSize * (0.53 + i * 0.04)), enemyTeamColor);
	//}






	////本当にこれでいいのかテストします
	////後で全部消す
	//const int height = 20;
	//const int width = 20;

	//std::array<std::array<int, width>, height> tiles;
	//for (int i : step(height))
	//	for (int j : step(width))tiles[i][j] = 0;
	//for (int i : step(height))tiles[i][(i + 2) % 20] = 1;

	//double cts = (TileSize * MaxFieldX + .0) / (Max(width, height) * TileSize + .0) * TileSize;

	//RectF dtile(cts * 0.96, cts * 0.96);

	//for (int i : step(height))
	//	for (int j : step(width)) {
	//		dtile.movedBy(i * cts + cts * 0.02, j * cts + cts * 0.02).draw((i + j) % 2 == 0 ? myTeamColor : enemyTeamColor);
	//		if (drawType == 1) {
	//			test(U"-16").drawAt(Vec2((i + 0.5) * cts, (j + 0.5) * cts), Palette::Black);
	//		}
	//	}

	//std::array<Vec2, 10> agents;
	//for (int i : step(10))agents[i] = Vec2(i, i);

	//if (drawType == 0) {
	//	for (int i = 0; i < 10; i++) {
	//		Shape2D::NStar(8, cts * 0.45, cts * 0.35, Vec2((agents[i].x + 0.5) * cts, (agents[i].y + 0.5) * cts)).draw();
	//		Shape2D::NStar(8, cts * 0.375, cts * 0.275, Vec2((agents[i].x + 0.5) * cts, (agents[i].y + 0.5) * cts)).draw(myTeamColor);
	//	}
	//}


	//Array<String> testS;
	//for (int i = 0; i < 8; i++) {
	//	testS.push_back(U"  ID : 10 ( 20 , 20 )");
	//}

	//for (int i = 0; i < 8; i++) {
	//	smallFont(testS[i]).draw(Vec2(MaxFieldX * TileSize * 1.04, MaxFieldY * TileSize * (0.53 + i * 0.04)), myTeamColor);
	//}


}

void Procon30::GUI::dataUpdate()
{
	//追加で開始されるetcが発生した場合、match_1がmatch_0にスライドされちゃうです

	viewerStrings.clear();

	//CATION:ごめんちょっとだけ許して。
	const size_t matchNum = (virtualServerMode ? 2 : observer->getStock().getMatchNum());
	for (size_t i = 0; i < matchNum; i++) {
		size_t width = observer->getStock((int32)i).field.boardSize.x;
		size_t height = observer->getStock((int32)i).field.boardSize.y;
		correctedTileSize[i] = (TileSize * MaxFieldX + .0) / (Max(width, height) * TileSize + .0) * TileSize;

		teamTile[i] = RectF(correctedTileSize[i] * 0.96, correctedTileSize[i] * 0.96);

		scoreFont[i] = Font((int32)(correctedTileSize[i] * 0.46));

		viewerStrings.push_back(U"match_{}"_fmt(i));
	}
}


Procon30::GUI::GUI()
	:observer(new Observer())
{
	//色変えたい場合はここ
	myTeamColor = Color(64, 191, 191);
	//enemyTeamColor = Color(192, 128, 128);
	enemyTeamColor = Color(201, 110, 100);
	noneTeamColor = Color(227);

	match = 0;
	drawType = 0;

	bigFont = Font(30, Typeface::Bold);
	smallFont = Font(24, Typeface::Bold);

	viewerBox = RectF(SideAreaX * 0.96, MaxFieldY * TileSize * 0.70);

	test = Font((int32)(50 * 0.46));
	texLoser = Texture(Emoji(U"😵"));
	texEven = Texture(Emoji(U"🤔"));
	texWinner = Texture(Emoji(U"🙂"));

}



Procon30::GUI::~GUI()
{
}

std::shared_ptr<Procon30::Observer> Procon30::GUI::getObserver()
{
	return observer;
}
