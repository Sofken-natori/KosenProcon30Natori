#include "VirtualServer.hpp"
#include "GUI.hpp"
#include "SendBuffer.hpp"
#include "Observer.hpp"

Procon30::VirtualServer::VirtualServer()
{
	width = Random(10, MaxFieldX);
	height = Random(10, MaxFieldY);
	agent_count = (width * height + 39) / 50;
	putPoint(0);
	putAgent();
}

void Procon30::VirtualServer::writeJson(FilePath path)
{
	String s;
	s += U"{\n";
	s += U"\t\"width\": " + Format(width) + U",\n";
	s += U"\t\"height\": " + Format(height) + U",\n";
	//points
	s += U"\t\"points\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			if (j == width - 1) {
				s += Format(field.m_board[i][j].score);
			}
			else {
				s += Format(field.m_board[i][j].score) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	s += U"\t\"startedAtUnixTime\": 0,\n";
	s += U"\t\"turn\": 0,\n";
	//tiled
	s += U"\t\"tiled\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			if (j == width - 1) {
				s += Format(tile[i][j]);
			}
			else {
				s += Format(tile[i][j]) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	//teams
	s += U"\t\"teams\": [\n";
	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": 1,\n";
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.first.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.first.agents[i].nowPosition.x) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.first.agents[i].nowPosition.y) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	int32 sum = 0;
	for (int i = 0; i < agent_count; i++) {
		sum += field.m_board[teams.first.agents[i].nowPosition.y][teams.first.agents[i].nowPosition.x].score;
	}
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.first.color)) + U"\n";
	s += U"\t\t},\n";

	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": 11,\n";
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.second.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.second.agents[i].nowPosition.x) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.second.agents[i].nowPosition.y) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.second.color)) + U"\n";
	s += U"\t\t}\n";
	s += U"\t],\n";
	s += U"\t\"actions\": []\n";
	s += U"}\n";

	TextWriter tw(path);
	tw << s;
	tw.close();
}

void Procon30::VirtualServer::negativePercent(int32 percent, int32 fieldType)
{
	bool isNegativeBorad = false;
	if (percent > 50) {
		isNegativeBorad = true;
		percent = 100 - percent;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				field.m_board[i][j].score *= -1;
			}
		}
	}
	int32 posX = 0, posY = 0;
	if (fieldType == 0) {
		int32 cnt = (((height * width) * percent) / 100) / 4;
		while (cnt > 0) {
			posX = Random(0, (width - 1) / 2);
			posY = Random(0, (height - 1) / 2);
			if (isNegativeBorad == false && field.m_board[posY][posX].score >= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score <= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}
	else if (fieldType == 1) {
		int32 cnt = (((height * width) * percent) / 100) / 2;
		while (cnt > 0) {
			posX = Random(0, (width - 1) / 2);
			posY = Random(0, height - 1);
			if (isNegativeBorad == false && field.m_board[posY][posX].score >= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score <= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}
	else if (fieldType == 2) {
		int32 cnt = (((height * width) * percent) / 100) / 2;
		while (cnt > 0) {
			posX = Random(0, width - 1);
			posY = Random(0, (height - 1) / 2);
			if (isNegativeBorad == false && field.m_board[posY][posX].score >= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score <= 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}

}

void Procon30::VirtualServer::putPoint(int32 fieldType)
{
	field.boardSize.y = height;
	field.boardSize.x = width;
	if (fieldType == 0) {
		//上下左右対称
		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (field.boardSize.x + 1) / 2; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		//negativePercent(20, 0);

		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[i][field.boardSize.x - j - 1] = field.m_board[i][j];
				field.m_board[field.boardSize.y - i - 1][j] = field.m_board[i][j];
				field.m_board[field.boardSize.y - i - 1][field.boardSize.x - j - 1] = field.m_board[i][j];
			}
		}
	}
	else if (fieldType == 1) {
		//左右対称
		for (int i = 0; i < field.boardSize.y; i++) {
			for (int j = 0; j < (field.boardSize.x + 1) / 2; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		negativePercent(20, 0);

		for (int i = 0; i < field.boardSize.y; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[i][field.boardSize.x - j - 1] = field.m_board[i][j];
			}
		}
	}
	else if (fieldType == 2) {
		//上下対称
		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < field.boardSize.x; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		negativePercent(20, 2);

		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[field.boardSize.y - i - 1][j] = field.m_board[i][j];
			}
		}
	}


	/*points.resize(height);
	for (int i = 0; i < height; i++) {
		points[i].resize(width);
	}
	for (int i = 0; i < (height + 1) / 2; i++) {
		for (int j = 0; j < (width + 1) / 2; j++) {
			points[i][j] = Random(-20, 20);
		}
	}
	for (int i = 0; i < (height + 1) / 2; i++) {
		for (int j = 0; j < (width + 1) / 2; j++) {
			points[i][width - j - 1] = points[i][j];
			points[height - i - 1][j] = points[i][j];
			points[height - i - 1][width - j - 1] = points[i][j];
		}
	}*/
	return;
}

void Procon30::VirtualServer::putAgent()
{
	//エージェントの数
	teams.first.agentNum = agent_count;
	teams.second.agentNum = agent_count;
	teams.first.agents.resize(teams.first.agentNum);
	teams.second.agents.resize(teams.second.agentNum);
	//チームidの設定
	teams.first.teamID = 1;
	teams.second.teamID = 11;
	//x,yを-1に設定
	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].nowPosition.x = -1;
		teams.first.agents[i].nowPosition.y = -1;
		teams.second.agents[i].nowPosition.x = -1;
		teams.second.agents[i].nowPosition.y = -1;
	}
	//エージェントidの設定
	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].agentID = i + 2;
		teams.second.agents[i].agentID = i + 12;
	}
	//tileの設定
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			tile[i][j] = 0;
		}
	}

	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].nowPosition.y = Random(0, height - 1);
		teams.second.agents[i].nowPosition.y = teams.first.agents[i].nowPosition.y;
	}
	for (int i = 0; i < teams.first.agentNum; i++) {
		bool isLoop = true;
		while (isLoop) {
			isLoop = false;
			teams.first.agents[i].nowPosition.x = Random(0, (width - 1) / 2);
			for (int j = 0; j < teams.first.agentNum; j++) {
				if (i != j) {
					if (teams.first.agents[i].nowPosition.x == teams.first.agents[j].nowPosition.x && teams.first.agents[i].nowPosition.y == teams.first.agents[j].nowPosition.y) {
						isLoop = true;
					}
				}
			}
		}
		teams.second.agents[i].nowPosition.x = width - teams.first.agents[i].nowPosition.x - 1;
		tile[teams.first.agents[i].nowPosition.y][teams.first.agents[i].nowPosition.x] = teams.first.teamID;
		tile[teams.second.agents[i].nowPosition.y][teams.second.agents[i].nowPosition.x] = teams.second.teamID;
		//	tiles[agents1[i][2]][agents1[i][1]] = agents1[i][0];
	//	tiles[agents2[i][2]][agents2[i][1]] = agents2[i][0];
	}
	//agents1.resize(agent_count);
	//agents2.resize(agent_count);
	///*agentsの0番目がチームid
	//1番目がx座標2番目がy座標
	//3番目がエージェントid
	//*/
	//for (int i = 0; i < agent_count; i++) {
	//	agents1[i].resize(4);
	//	agents2[i].resize(4);
	//	agents1[i][0] = 1;
	//	agents2[i][0] = 11;
	//	agents1[i][1] = -1;
	//	agents1[i][2] = -1;
	//	agents2[i][1] = -1;
	//	agents2[i][2] = -1;
	//	agents1[i][3] = 2 + i;
	//	agents2[i][3] = 12 + i;
	//}
	//tiles.resize(height);
	//for (int i = 0; i < height; i++) {
	//	tiles[i].resize(width);
	//}
	///*for (int i = 0; i < height;i++) {
	//	for (int j = 0; j < width;j++) {
	//		tiles[i][j] = 0;
	//	}
	//}*/
	//for (int i = 0; i < agent_count; i++) {
	//	agents1[i][2] = Random(0, height - 1);
	//	agents2[i][2] = agents1[i][2];
	//}
	//for (int i = 0; i < agent_count; i++) {
	//	bool isLoop = false;
	//	while (isLoop == false) {
	//		isLoop = false;
	//		agents1[i][1] = Random(0, (width - 1) / 2);
	//		for (int j = 0; j < agent_count; j++) {
	//			if (i != j) {
	//				if (agents1[i][2] != agents1[j][2] && agents1[i][1] != agents1[j][1]) {
	//					isLoop = true;
	//				}
	//			}
	//		}
	//	}
	//	agents2[i][1] = width - agents1[i][1] - 1;
	//	tiles[agents1[i][2]][agents1[i][1]] = agents1[i][0];
	//	tiles[agents2[i][2]][agents2[i][1]] = agents2[i][0];
	//}
	return;
}

int32 Procon30::VirtualServer::calculateScore(TeamColor color)
{
	Array<Array<bool>> visit(MaxFieldY, Array<bool>(MaxFieldX, false));

	Array<Point> q;

	int fieldSizeX = field.boardSize.x;
	int fieldSizeY = field.boardSize.y;

	assert(fieldSizeX > 0);
	assert(fieldSizeY > 0);

	for (auto y : step(fieldSizeY)) {
		q.push_back({ 0, y });
		q.push_back({ fieldSizeX - 1,y });
	}
	for (auto x : step(fieldSizeX)) {
		q.push_back({ x , 0 });
		q.push_back({ x , fieldSizeY - 1 });
	}

	while (!q.isEmpty()) {

		auto now = q.front();
		q.pop_front();

		if (0 <= now.y && now.y < fieldSizeY && 0 <= now.x && now.x < fieldSizeX) {

			if (visit.at(now.y).at(now.x))
				continue;
			visit.at(now.y).at(now.x) = true;

			if (field.m_board.at(now.y, now.x).color != color) {

				q.push_back({ now.x - 1	,now.y });
				q.push_back({ now.x		,now.y + 1 });
				q.push_back({ now.x + 1 ,now.y });
				q.push_back({ now.x		,now.y - 1 });

			}

		}

	}

	int32 result = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (color != field.m_board.at(y, x).color && visit.at(y).at(x) == false) {
				result += abs(field.m_board.at(y, x).score);
			}
			if (field.m_board.at(y, x).color == color) {
				result += field.m_board.at(y, x).score;
			}
		}
	}

	return result;
}

void Procon30::VirtualServer::VirtualServerMain()
{

	Window::Resize(Procon30::WindowSize);
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
	Scene::SetScaleMode(ScaleMode::AspectFit);
	Window::SetStyle(WindowStyle::Sizable);


	//incetance genarate order
	//1st GUI
	//- Observer instance
	//2nd HTTPCommunication
	//- SendBuffer instance
	//- Observer shared_ptr Substitution
	//3rd Game
	//- Observer shared_ptr Substitution
	//- SendBuffer shared_ptr Substitution

	//thread
	//1:GUI - main,Observer
	//2:HTTPCommunication　-sendBuffer
	//3:Game0
	//4:Game1
	//5:Game2
	//6:VirtualServer
	std::shared_ptr<std::atomic<bool>> ProgramEnd(new std::atomic<bool>);
	ProgramEnd->store(false);


	Procon30::GUI gui;

	Procon30::VirtualServer server;

	server.buffer.reset(new SendBuffer());

	std::array<Procon30::Game, Procon30::MaxGameNumber> games;

	server.observer = gui.getObserver();
	server.programEnd = ProgramEnd;

	for (size_t i = 0; i < server.getMatchNum(); i++) {
		games[i].observer = gui.getObserver();
		games[i].buffer = server.getBufferPtr();
		games[i].gameNum = i;
		//gameIDは、先攻が1、後攻が2固定。また、公式配布のIDと同じとする
		games[i].gameID = server.getGameIDfromGameNum(i);
		//その内現在時間を入れる。
		games[i].startedAtUnixTime = 0;
		games[i].matchTo = (i == 0 ? U"A" : U"B");
		games[i].MaxTurn = 60;
		games[i].turnMillis = v_turnMillis;
		games[i].intervalMillis = v_intervalMillis;
		//games[0]が自分1,相手2、games[1]が自分2,相手1
		games[i].teams.first.teamID = (i == 0 ? 1 : 2);
		games[i].programEnd = ProgramEnd;
	}

	Scene::SetBackground(Color(128));

	for (size_t i = 0; i < server.getMatchNum(); i++) {
		games[i].ThreadRun();
	}
	//ごくまれに間に合ってないでスレッド行っちゃった事例が発生するために、時間遅延。
	server.ThreadRun();

	gui.dataUpdate();
	//TODO:あとでthreadGuardにします。
	while (System::Update() || ProgramEnd->load())
	{

		gui.draw();

		Circle(Cursor::Pos(), 60).draw(ColorF(1, 0, 0, 0.5));
	}

	ProgramEnd->store(true);

	//Procon30::Game::HTTPReceived();

	server.thisThread.join();

	return;
}

const size_t Procon30::VirtualServer::getMatchNum() const
{
	return 2;
}

const size_t Procon30::VirtualServer::getGameIDfromGameNum(const int32& num) const
{
	size_t table[] = { 1, 2 };
	return table[num];
}

std::shared_ptr<Procon30::SendBuffer> Procon30::VirtualServer::getBufferPtr()
{
	return buffer;
}


void Procon30::VirtualServer::Loop()
{
	//サーバーからデータ取得
	//その代わりにデータをセットしておく
	//games[0]が自分1,相手2、games[1]が自分2,相手1
	teams.first.teamID = 1;
	teams.second.teamID = 2;

	FilePath matchField = Format(U"json/VirtualServer/matchField.json");

	//今後翔君の関数を使って自動生成する場合はここに

	constexpr bool isGeneratedField = false;

	/*
	while (comData.gotMatchInfomationNum != comData.matchNum) {
		getMatchInfomation();
		while (true)
		{
			if (checkResult())break;
		}
		std::this_thread::sleep_for(500ms);
	}
	*/

	initMatch(matchField);

	FileSystem::Copy(matchField, Format(U"json/", 0, U"/nowField.json"), CopyOption::OverwriteExisting);
	FileSystem::Copy(matchField, Format(U"json/", 1, U"/nowField.json"), CopyOption::OverwriteExisting);
	Print << U"gotMatchInfoof:" << 0;
	Print << U"gotMatchInfoof:" << 0;

	std::this_thread::sleep_for(500ms);
	Procon30::Game::HTTPReceived();
	//observer->notify(*this);

	gameTimer.start();
	turnTimer.start();

	turn = 0;

	isStrategyStep = true;
	posted[1] = posted[0] = false;

	while (true) {
		update();
		if (programEnd->load() == true) {
			Procon30::Game::HTTPReceived();
			break;
		}
	}
	Logger << U"VirtualServer Thread End";
	return;
}

void Procon30::VirtualServer::ThreadRun()
{
	//observer->notify(*this);
	std::thread th(&VirtualServer::Loop, this);
	this->thisThread = std::move(th);
	return;
}

void Procon30::VirtualServer::update()
{
	//結果が帰ってきてないか確認。多分いらない
	//bool gotResult = checkResult();


	if (isStrategyStep && turnTimer.ms() > v_turnMillis) {
		turnTimer.restart();
		isStrategyStep = false;

		Logger << U"gameNum:0 {} posted"_fmt(posted[0]);
		Logger << U"gameNum:1 {} posted"_fmt(posted[1]);

		posted[1] = posted[0] = false;

		//アクションデータの解析。
		parseActionData(U"json/VirtualServer/post_{}_{}.json"_fmt(0, turn));
		parseActionData(U"json/VirtualServer/post_{}_{}.json"_fmt(1, turn));

		//シミュレーションを行う。
		simulation();
	}

	bool gotResult = false;

	if (!isStrategyStep && turnTimer.ms() > v_intervalMillis) {


		//シミュレーションを行った結果をGameに送る
		FilePath v_nowField = U"json/VirtualServer/field_{}.json"_fmt(turn);

		//サーバー側のデータのconvertToJson
		//WAGNI:startedAtUnixTimeは仮置き。

		turn++;

		writeFieldJson(v_nowField);

		FileSystem::Copy(v_nowField, Format(U"json/", 0, U"/nowField.json"), CopyOption::OverwriteExisting);
		FileSystem::Copy(v_nowField, Format(U"json/", 1, U"/nowField.json"), CopyOption::OverwriteExisting);

		turnTimer.restart();
		isStrategyStep = true;

		gotResult = true;

		//シミュレーションを行ったらフラグを立てて置いてここでGameを起こすようにする
		Procon30::Game::HTTPReceived();
	}

	//post処理は何よりも優先されるべき
	//答えをポスト
	//SendBuffer等を用いて答えが投稿されてきたらどこかにコピーしておくようにする。
	bool postNow = checkPostAction();

	/*
	if (comData.gotMatchInfomationNum == comData.matchNum) {
		Procon30::Game::HTTPReceived();
		comData.gotMatchInfomationNum = 0;
	}
	*/
	//データの更新は一斉だからいらないかな
	/*
	if (comData.gotMatchInfomationNum != 0) {
		getMatchInfomation();
	}
	*/
	//時間に合わせて取得処理を書きますが...(今はかけない(公式の回答待ち))
	//ここはいらないかな

	if (gotResult || postNow) {
		//observer->notify(*this);
	}

}


bool Procon30::VirtualServer::checkPostAction()
{
	if (buffer->size() == 0) return false;

	FilePath path = buffer->getPath();
	auto splittedPath = path.split('_');
	int32 gameNum = Parse<int32>(splittedPath[1]);
	String send = getPostData(path);

	TextWriter tw(U"json/VirtualServer/post_{}_{}.json"_fmt(gameNum, turn));
	tw << send;
	tw.close();

	posted[gameNum] = true;

	return true;
}

String Procon30::VirtualServer::getPostData(const FilePath& filePath) {
	TextReader reader(filePath);
	if (!reader) {
		assert("send_json_file_openError");
		exit(1);
	}
	String send = reader.readAll();
	reader.close();
	return send;
}

bool Procon30::VirtualServer::initMatch(const FilePath& filePath)
{
	//parseJsonを参考に埋める。一応埋めた。

	s3d::JSONReader reader(filePath);

	assert(reader);

	{
		this->turn = reader[U"turn"].get<int32>();
		//WAGNI:startedAtUnixTimeの実装
		//this->startedAtUnixTime = reader[U"startedAtUnixTime"].get<int32>();

		{
			field.boardSize.y = reader[U"height"].get<int32>();
			field.boardSize.x = reader[U"width"].get<int32>();

			{
				for (int32 y = 0; y < reader[U"points"].arrayCount(); y++) {
					for (int32 x = 0; x < reader[U"points"].arrayView()[y].arrayCount(); x++) {
						field.m_board.at(y, x).score = reader[U"points"].arrayView()[y].arrayView()[x].get<int32>();
						field.m_board.at(y, x).exist = true;
					}
				}
			}


			{
				const auto& teamJsonData = *reader[U"teams"].arrayView().begin();
				//this->teams.first.teamID = team[U"teamID"].get<int32>();
				this->teams.first.tileScore = teamJsonData[U"tilePoint"].get<int32>();
				this->teams.first.areaScore = teamJsonData[U"areaPoint"].get<int32>();
				this->teams.first.score = teamJsonData[U"tilePoint"].get<int32>() + teamJsonData[U"areaPoint"].get<int32>();
				this->teams.first.color = TeamColor::Blue;

				this->teams.first.agentNum = teamJsonData[U"agents"].arrayCount();
				this->teams.first.agents = Array< Agent >(this->teams.first.agentNum);

				for (int32 i = 0; i < teamJsonData[U"agents"].arrayCount(); i++)
				{
					const JSONValue& agent = teamJsonData[U"agents"].arrayView()[i];
					this->teams.first.agents.at(i).agentID = agent[U"agentID"].get<int32>();
					this->teams.first.agents.at(i).nowPosition.x = agent[U"x"].get<int32>() - 1;
					this->teams.first.agents.at(i).nowPosition.y = agent[U"y"].get<int32>() - 1;
				}
			}
			{
				const auto& teamJsonData = *(++reader[U"teams"].arrayView().begin());
				//this->teams.first.teamID = team[U"teamID"].get<int32>();
				this->teams.second.tileScore = teamJsonData[U"tilePoint"].get<int32>();
				this->teams.second.areaScore = teamJsonData[U"areaPoint"].get<int32>();
				this->teams.second.score = teamJsonData[U"tilePoint"].get<int32>() + teamJsonData[U"areaPoint"].get<int32>();
				this->teams.second.color = TeamColor::Red;

				this->teams.second.agentNum = teamJsonData[U"agents"].arrayCount();
				this->teams.second.agents = Array< Agent >(this->teams.second.agentNum);

				for (int32 i = 0; i < teamJsonData[U"agents"].arrayCount(); i++)
				{
					const JSONValue& agent = teamJsonData[U"agents"].arrayView()[i];
					this->teams.second.agents.at(i).agentID = agent[U"agentID"].get<int32>();
					this->teams.second.agents.at(i).nowPosition.x = agent[U"x"].get<int32>() - 1;
					this->teams.second.agents.at(i).nowPosition.y = agent[U"y"].get<int32>() - 1;
				}
			}

			{
				for (int32 y = 0; y < reader[U"tiled"].arrayCount(); y++) {
					for (int32 x = 0; x < reader[U"tiled"].arrayView()[y].arrayCount(); x++) {
						const int32 tileColor = reader[U"tiled"].arrayView()[y].arrayView()[x].get<int32>();
						field.m_board.at(y, x).color = (tileColor == this->teams.first.teamID) ? (TeamColor::Blue)
							: tileColor == this->teams.second.teamID ? (TeamColor::Red)
							: TeamColor::None;
					}
				}
			}

		}
	}

	return true;
}

bool Procon30::VirtualServer::parseActionData(const FilePath& filePath)
{

	s3d::JSONReader reader(filePath);

	const auto& object = reader[U"actions"];

	{
		for (int32 i = 0; i < object.arrayCount(); i++)
		{
			const JSONValue& action = object.arrayView()[i];
			const auto agentID = action[U"agentID"].get<int32>();
			const auto type = action[U"type"].getString();
			auto dx = action[U"dx"].get<int32>();
			auto dy = action[U"dy"].get<int32>();

			bool ok = false;
			{
				auto& team = this->teams.first;
				for (auto& agent : team.agents) {
					if (agent.agentID == agentID) {
						ok = true;
						if (type == U"move") {
							agent.action = Action::Move;
						}
						else if (type == U"remove") {
							agent.action = Action::Remove;
						}
						else if (type == U"stay") {
							agent.action = Action::Stay;
							dx = dy = 0;
						}
						if (!(-1 <= dx && dx <= 1 && -1 <= dy && dy <= 1)) {
							agent.action = Action::Stay;
							dx = dy = 0;
						}
						agent.nextPosition = agent.nowPosition + s3d::Point(dx, dy);
						if (agent.nextPosition.x >= field.boardSize.x || agent.nextPosition.y >= field.boardSize.y || agent.nextPosition.x < 0 || agent.nextPosition.y < 0) {
							agent.action = Action::Stay;
							dx = dy = 0;
							agent.nextPosition = agent.nowPosition + s3d::Point(dx, dy);
						}
						break;
					}
				}
			}
			{
				auto& team = this->teams.second;
				for (auto& agent : team.agents) {
					if (agent.agentID == agentID) {
						ok = true;
						if (type == U"move") {
							agent.action = Action::Move;
						}
						else if (type == U"remove") {
							agent.action = Action::Remove;
						}
						else if (type == U"stay") {
							agent.action = Action::Stay;
							dx = dy = 0;
						}
						if (!(-1 <= dx && dx <= 1 && -1 <= dy && dy <= 1)) {
							agent.action = Action::Stay;
							dx = dy = 0;
						}
						agent.nextPosition = agent.nowPosition + s3d::Point(dx, dy);
						if (agent.nextPosition.x >= field.boardSize.x || agent.nextPosition.y >= field.boardSize.y || agent.nextPosition.x < 0 || agent.nextPosition.y < 0) {
							agent.action = Action::Stay;
							dx = dy = 0;
							agent.nextPosition = agent.nowPosition + s3d::Point(dx, dy);
						}
						break;
					}
				}
			}

			assert(ok);
		}

	}
	return true;
}

void Procon30::VirtualServer::simulation()
{

	//ok == 0 or 1
	//ng == 2
	//wait == 3 
	for (int loop = 0; loop < 20; loop++) {

		int flag[2][10] = {};

		{//check
			auto& team = this->teams.first;
			int i = 0;
			for (auto& agent : team.agents) {

				if (agent.action == Action::Stay) {
					flag[0][i] = 1;
				}
				else if (agent.action == Action::Move || agent.action == Action::Remove) {
					auto& f_team = this->teams.first;
					for (auto& f_agent : f_team.agents) {
						if (agent.nowPosition == f_agent.nowPosition) {
							continue;
						}
						if (agent.nextPosition == f_agent.nextPosition) {
							flag[0][i] = 2;
						}
						else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
							if (flag[0][i] != 2)
								flag[0][i] = 3;
						}
						else {
							if (flag[0][i] == 0)
								flag[0][i] = 1;
						}
					}
					auto& s_team = this->teams.second;
					for (auto& s_agent : s_team.agents) {
						if (agent.nextPosition == s_agent.nextPosition) {
							flag[0][i] = 2;
						}
						else if (agent.nextPosition == s_agent.nowPosition && s_agent.action == Action::Move) {
							if (flag[0][i] != 2)
								flag[0][i] = 3;
						}
						else {
							if (flag[0][i] == 0)
								flag[0][i] = 1;
						}
					}
				}
				i++;
			}
		}
		{//check
			auto& team = this->teams.second;
			int i = 0;
			for (auto& agent : team.agents) {

				if (agent.action == Action::Stay) {
					flag[1][i] = 1;
				}
				else if (agent.action == Action::Move || agent.action == Action::Remove) {
					auto& f_team = this->teams.first;
					for (auto& f_agent : f_team.agents) {
						if (agent.nextPosition == f_agent.nextPosition) {
							flag[1][i] = 2;
						}
						else if (agent.nextPosition == f_agent.nowPosition && f_agent.action == Action::Move) {
							if (flag[1][i] != 2)
								flag[1][i] = 3;
						}
						else {
							if (flag[1][i] == 0)
								flag[1][i] = 1;
						}
					}
					auto& s_team = this->teams.second;
					for (auto& s_agent : s_team.agents) {
						if (agent.nowPosition == s_agent.nowPosition) {
							continue;
						}
						if (agent.nextPosition == s_agent.nextPosition) {
							flag[1][i] = 2;
						}
						else if (agent.nextPosition == s_agent.nowPosition && s_agent.action == Action::Move) {
							if (flag[1][i] != 2)
								flag[1][i] = 3;
						}
						else {
							if (flag[1][i] == 0)
								flag[1][i] = 1;
						}
					}
				}
				i++;
			}
		}
		{//update
			int i = 0;
			auto& team = this->teams.first;
			for (auto& agent : team.agents) {
				if (flag[0][i] == 0 || flag[0][i] == 1) {

					switch (agent.action) {
					case Action::Move:
						if (field.m_board.at(agent.nextPosition).color != TeamColor::Red) {
							agent.nowPosition = agent.nextPosition;
							field.m_board.at(agent.nextPosition).color = TeamColor::Blue;
						}
						break;
					case Action::Remove:
						field.m_board.at(agent.nextPosition).color = TeamColor::None;
						break;
					case Action::Stay:
						break;
					}
				}
				if (flag[0][i] == 0 || flag[0][i] == 1 || flag[0][i] == 2) {
					agent.action = Action::Stay;
					agent.nextPosition = agent.nowPosition;
				}
				i++;
			}
		}
		{//update
			int i = 0;
			auto& team = this->teams.second;
			for (auto& agent : team.agents) {
				if (flag[1][i] == 0 || flag[1][i] == 1) {

					switch (agent.action) {
					case Action::Move:
						if (field.m_board.at(agent.nextPosition).color != TeamColor::Blue) {
							agent.nowPosition = agent.nextPosition;
							field.m_board.at(agent.nextPosition).color = TeamColor::Red;
						}
						break;
					case Action::Remove:
						field.m_board.at(agent.nextPosition).color = TeamColor::None;
						break;
					case Action::Stay:
						break;
					}
				}
				if (flag[1][i] == 0 || flag[1][i] == 1 || flag[1][i] == 2) {
					agent.action = Action::Stay;
					agent.nextPosition = agent.nowPosition;
				}
				i++;
			}
		}

	}

	return;
}


void Procon30::VirtualServer::writeFieldJson(FilePath path)
{
	String s;
	s += U"{\n";
	s += U"\t\"width\": " + Format(field.boardSize.x) + U",\n";
	s += U"\t\"height\": " + Format(field.boardSize.y) + U",\n";

	height = field.boardSize.y;
	width = field.boardSize.x;

	//points
	s += U"\t\"points\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			if (j == width - 1) {
				s += Format(field.m_board[i][j].score);
			}
			else {
				s += Format(field.m_board[i][j].score) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	s += U"\t\"startedAtUnixTime\": 0,\n";
	s += U"\t\"turn\": {},\n"_fmt(turn);
	//tiled
	s += U"\t\"tiled\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			int tile_num = 0;
			if (field.m_board[i][j].color == TeamColor::Blue) {
				tile_num = teams.first.teamID;
			}
			else if (field.m_board[i][j].color == TeamColor::Red) {
				tile_num = teams.second.teamID;
			}
			if (j == width - 1) {
				s += Format(tile_num);
			}
			else {
				s += Format(tile_num) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	//teams

	int agent_count = teams.first.agentNum;

	s += U"\t\"teams\": [\n";
	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": {},\n"_fmt(teams.first.teamID);
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.first.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.first.agents[i].nowPosition.x + 1) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.first.agents[i].nowPosition.y + 1) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	int32 sum = 0;
	for (const auto& tile : field.m_board) {
		if (teams.first.color == tile.color) {
			sum += tile.score;
		}
	}
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.first.color) - sum) + U"\n";
	s += U"\t\t},\n";

	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": {},\n"_fmt(teams.second.teamID);
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.second.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.second.agents[i].nowPosition.x + 1) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.second.agents[i].nowPosition.y + 1) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	sum = 0;
	for (const auto& tile : field.m_board) {
		if (teams.second.color == tile.color) {
			sum += tile.score;
		}
	}
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.second.color) - sum) + U"\n";
	s += U"\t\t}\n";
	s += U"\t],\n";
	s += U"\t\"actions\": []\n";
	s += U"}\n";

	TextWriter tw(path);
	tw.writelnUTF8(s.toUTF8());
	tw.close();
}
