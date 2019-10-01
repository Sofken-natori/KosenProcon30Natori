#include "GUI.hpp"



void Procon30::GUI::draw() {
	//ゲームが信仰していなときに死ぬ or 初期状態で0番目のゲーム参照になる
	SimpleGUI::RadioButtons(match, viewerStrings, Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.01));
	SimpleGUI::RadioButtons(drawType, { U"タイル + エージェント",U"タイル + 点数" }, Vec2(MaxFieldX * TileSize * 1.2, MaxFieldY * TileSize * 0.01));

	if (observer->getUpdateFlag((int32)match)) {
		dataUpdate();
	}

	//WAGNI:Gameのコピーコンストラクタを作ってほしいかも。なくていい
	Game game;
	game = observer->getStock((int32)match);

	//盤面描画
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
			//点数描画
			if (drawType == 1) {
				scoreFont[match](game.field.m_board[y][x].score).drawAt(Vec2((x + 0.5) * correctedTileSize[match], (y + 0.5) * correctedTileSize[match]), Palette::Black);
			}
		}
	}


	//エージェントの描画
	if (drawType == 0) {
		//相手
		for (Agent agent : game.teams.second.agents) {
			Vec2 pos = { (agent.nowPosition.x + 0.5) * correctedTileSize[match],(agent.nowPosition.y + 0.5) * correctedTileSize[match] };
			Shape2D::NStar(8, correctedTileSize[match] * 0.45, correctedTileSize[match] * 0.35, pos).draw();
			Shape2D::NStar(8, correctedTileSize[match] * 0.375, correctedTileSize[match] * 0.275, pos).draw(enemyTeamColor);
		}
		//自分
		for (Agent agent : game.teams.first.agents) {
			Vec2 pos = { (agent.nowPosition.x + 0.5) * correctedTileSize[match],(agent.nowPosition.y + 0.5) * correctedTileSize[match] };
			Vec2 nxpos = { (agent.nextPosition.x + 0.5) * correctedTileSize[match],(agent.nextPosition.y + 0.5) * correctedTileSize[match] };

			Line(pos, nxpos).drawArrow(correctedTileSize[match] * 0.066, Vec2(correctedTileSize[match] * 0.133, correctedTileSize[match] * 0.133), Palette::Yellow);
			Circle(pos, correctedTileSize[match] / 2 * 0.8).draw();
			Circle(pos, correctedTileSize[match] / 2 * 0.675).draw(myTeamColor);
			if (agent.action == Action::Move) {
				scoreFont[match](U"M").drawAt(pos);
			}
			else if (agent.action == Action::Remove) {
				scoreFont[match](U"R").drawAt(pos);
			}
			else {
				scoreFont[match](U"S").drawAt(pos);
			}
		}
	}


	bigFont(U"現在のターン : ", game.turn).draw(Vec2(MaxFieldX * TileSize * 1.01, MaxFieldY * TileSize * 0.15), Palette::White);

	//絵文字表示 +50 or -50 にて表情変化 
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
	double y = MaxFieldY * TileSize * 0.30;
	bigFont(U"MyTeam (ID : {})"_fmt(game.teams.first.teamID)).draw(Vec2(myInfoX, y), myTeamColor);
	bigFont(U"Score : {}"_fmt(game.teams.first.score)).draw(Vec2(myInfoX, y += (bigFont.fontSize() * 1.5)), myTeamColor);
	smallFont(U"  TileScore  : {}"_fmt(game.teams.first.tileScore)).draw(Vec2(myInfoX, y += (bigFont.fontSize() * 1.5)), myTeamColor);
	smallFont(U"  AreaScore : {}"_fmt(game.teams.first.areaScore)).draw(Vec2(myInfoX, y += (smallFont.fontSize() * 1.5)), myTeamColor);

	bigFont(U"Agents").draw(Vec2(myInfoX, y += (bigFont.fontSize() * 1.5)), myTeamColor);
	y += (smallFont.fontSize() / 2);
	for (int i : step(game.teams.first.agentNum)) {
		smallFont(U"  pos {0} → {1} "_fmt(
			game.teams.first.agents[i].nowPosition,
			game.teams.first.agents[i].nextPosition)).draw(Vec2(myInfoX, y += (smallFont.fontSize() * 1.5)), myTeamColor);
	}


	//相手チーム情報
	y = MaxFieldY * TileSize * 0.30;
	bigFont(U"EnemyTeam (ID : {})"_fmt(game.teams.second.teamID)).draw(Vec2(enemyInfoX, y), enemyTeamColor);
	bigFont(U"Score : {}"_fmt(game.teams.second.score)).draw(Vec2(enemyInfoX, y += (bigFont.fontSize() * 1.5)), enemyTeamColor);
	smallFont(U"  TileScore : {} "_fmt(game.teams.second.tileScore)).draw(Vec2(enemyInfoX, y += (bigFont.fontSize() * 1.5)), enemyTeamColor);
	smallFont(U"  AreaScore : {}"_fmt(game.teams.second.areaScore)).draw(Vec2(enemyInfoX, y += (smallFont.fontSize() * 1.5)), enemyTeamColor);

	bigFont(U"Agents").draw(Vec2(enemyInfoX, y += (bigFont.fontSize() * 1.5)), enemyTeamColor);
	y += (smallFont.fontSize() / 2);
	for (int i : step(game.teams.second.agentNum)) {
		smallFont(U"  pos {0}"_fmt(
			game.teams.second.agents[i].nowPosition)).draw(Vec2(enemyInfoX, y += (smallFont.fontSize() * 1.5)), enemyTeamColor);
	}
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

		//viewerStrings.push_back(U"match_{}"_fmt(i));
		viewerStrings.push_back(U"{}"_fmt(observer->getStock(i).gameID));
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
