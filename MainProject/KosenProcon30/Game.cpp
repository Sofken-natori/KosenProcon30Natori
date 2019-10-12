#include "Game.hpp"
#include "Observer.hpp"
#include "Algorithm.hpp"
#include "Algorithm/SuzukiAlgorithm.hpp"
#include "Algorithm/TakahashiAlgorithm.hpp"

bool Procon30::Game::dataReceived = false;
std::mutex Procon30::Game::HTTPWaitMtx;
std::condition_variable Procon30::Game::HTTPWaitCond;
std::mutex Procon30::Game::ReceiveMtx;

void Procon30::Game::HTTPReceived()
{
	{
		std::lock_guard<std::mutex> lock(HTTPWaitMtx);
		dataReceived = true;
	}
	HTTPWaitCond.notify_all();
}

int32 Procon30::Game::calculateScore(TeamColor color)
{
	Array<Array<bool>> visit(field.boardSize.y, Array<bool>(field.boardSize.x, false));

	Array<Point> q;

	const int& fieldSizeX = field.boardSize.x;
	const int& fieldSizeY = field.boardSize.y;

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

int32 Procon30::Game::calculateTileScore(TeamColor color)
{
	int32 fieldSizeX = -1;
	int32 fieldSizeY = -1;

	for (auto y : step(MaxFieldY)) {
		if (field.m_board.at(y, 0).exist == false) {
			fieldSizeY = y + 1;
			break;
		}
	}
	for (auto x : step(MaxFieldX)) {
		if (field.m_board.at(0, x).exist == false) {
			fieldSizeX = x + 1;
			break;
		}
	}

	assert(fieldSizeX > 0);
	assert(fieldSizeY > 0);
	int32 result = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (field.m_board.at(y, x).color == color) {
				result += field.m_board.at(y, x).score;
			}
		}
	}

	return result;
}

void Procon30::Game::updateData()
{
	//Wait部
	{
		std::unique_lock<std::mutex> lockWait(HTTPWaitMtx);
		HTTPWaitCond.wait(lockWait, [this]() {return dataReceived; });
	}
	if (programEnd->load()) {
		return;
	}
	//処理部@世界一雑.obj
	FilePath path = Format(U"json/", gameNum, U"/nowField.json");
	parseJson(path);

	//未受信変更部
	std::lock_guard<std::mutex> lockFlag(ReceiveMtx);
	dataReceived = false;

	observer->notify(gameNum, *this);
}

void Procon30::Game::sendToHTTP()
{
	FilePath path = Format(U"json/", gameNum, U"/post_", gameNum, U"_", turn, U".json");
	convertToJson(path);
	buffer->pushPath(path);
}

void Procon30::Game::Loop()
{
	while (true) {
		updateData();
		if (programEnd->load() == true)
			break;
		observer->notify(gameNum, *this);
		this->turnTimer.restart();
		//Algo実行
		{

			//CAUTION:許して
			if (fieldType == PublicField::NONE) {
				PublicFields publicFields;
				fieldType = publicFields.checkPublicField(*this);
				//ここでアルゴリズムの初期化をする。
				if (fieldType == PublicField::NON_MATCHING) {
					//Private
					std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> PBArray;
					for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++)
						PBArray[parallelNum] = std::unique_ptr<PruneBranchesAlgorithm>(new Procon30::YASAI::CompressBranch(1.8));

					FilePath parameterFilePath = U"parameters/parameter.ini";
					FilePath secondSearchParameterFilePath = U"parameters/parameter.ini";

					switch (this->teams.first.agentNum) {
					case 2:
						break;
					case 3:
						break;
					case 4:
						break;
					case 5:
						break;
					case 6:
						break;
					case 7:
						break;
					case 8:
						break;
					}

					std::array<std::unique_ptr<PruneBranchesAlgorithm>, parallelSize> secondPBArray;

					for (int32 parallelNum = 0; parallelNum < parallelSize; parallelNum++)
						secondPBArray[parallelNum] = std::unique_ptr<PruneBranchesAlgorithm>(new Procon30::YASAI::CompressBranch(1.8));

					algorithm.reset(new Procon30::ProconAlgorithm(parameterFilePath, std::move(PBArray), std::unique_ptr<Algorithm>(new Procon30::SUZUKI::SuzukiBeamSearchAlgorithm(secondSearchParameterFilePath ,std::move(secondPBArray)))));
				}
				else {
					//Public
					switch (fieldType)
					{
					case Procon30::PublicField::NONE:

						break;					
					case Procon30::PublicField::A_1:
						
						break;
					case Procon30::PublicField::A_2:
						break;
					case Procon30::PublicField::A_3:
						break;
					case Procon30::PublicField::A_4:
						break;
					case Procon30::PublicField::B_1:
						break;
					case Procon30::PublicField::B_2:
						break;
					case Procon30::PublicField::B_3:
						break;
					case Procon30::PublicField::C_1:
						break;
					case Procon30::PublicField::C_2:
						break;
					case Procon30::PublicField::D_1:
						break;
					case Procon30::PublicField::D_2:
						break;
					case Procon30::PublicField::E_1:
						break;
					case Procon30::PublicField::E_2:
						break;
					case Procon30::PublicField::F_1:
						break;
					case Procon30::PublicField::F_2:
						break;
					default:
						break;
					}
				}

			}

			assert(algorithm);

			algorithm->initilize(*this);

			auto result = algorithm->execute(*this);

			assert(result.code == Procon30::AlgorithmStateCode::None);

			const int32 selectedOrder = 0;// Random(result.orders.size() - 1);

			for (int32 i = 0; i < this->teams.first.agentNum; i++) {
				this->teams.first.agents[i].nextPosition = this->teams.first.agents[i].nowPosition + result.orders[selectedOrder][i].dir;
				this->teams.first.agents[i].action = result.orders[selectedOrder][i].action;
			}
		}
		observer->notify(gameNum, *this);
		sendToHTTP();
		observer->notify(gameNum, *this);
		if (programEnd->load() == true)
			break;
	}
	SafeConsole(U"Game Thread End");
	return;
}

void Procon30::Game::ThreadRun()
{
	std::thread th(&Game::Loop, this);
	this->thisThread = std::move(th);
	return;
}

Procon30::Game::Game()
{
	isSearchFinished = false;
	MaxTurn = 60;
	turn = 0;
	startedAtUnixTime = 0;

	algorithm.reset(new BeamSearchAlgorithm(100));
	//algorithm.reset(new RandAlgorithm());
}

Procon30::Game::~Game()
{
	if (thisThread.joinable()) {
		thisThread.join();
	}
}

Procon30::Game& Procon30::Game::operator=(const Procon30::Game& right)
{
	this->field = right.field;
	this->gameTimer = right.gameTimer;
	this->turnTimer = right.turnTimer;
	this->turn = right.turn;
	this->MaxTurn = right.MaxTurn;
	this->gameID = right.gameID;
	this->gameNum = right.gameNum;
	this->matchTo = right.matchTo;
	this->turnMillis = right.turnMillis;
	this->intervalMillis = right.intervalMillis;
	this->startedAtUnixTime = right.startedAtUnixTime;
	this->teams = right.teams;
	this->isSearchFinished = right.isSearchFinished;
	this->fieldType = this->fieldType;

	return (*this);
}

Procon30::PublicFields::PublicFields() {
	//許して
	std::array<String, 15> fileNames = {
		U"A-1",U"A-2",U"A-3",U"A-4",
		U"B-1",U"B-2",U"B-3",
		U"C-1",U"C-2",
		U"D-1",U"D-2",
		U"E-1",U"E-2",
		U"F-1",U"F-2" };

	FilePath filePath = U"json/PublicField/";
	for (int i = 0; i < 15; i++) {
		FilePath path = Format(filePath, fileNames[i], U".json");

		s3d::JSONReader reader(path);

		fields[i].boardSize.y = reader[U"height"].get<int32>();
		fields[i].boardSize.x = reader[U"width"].get<int32>();

		{
			for (int32 y = 0; y < reader[U"points"].arrayCount(); y++) {
				for (int32 x = 0; x < reader[U"points"].arrayView()[y].arrayCount(); x++) {
					fields[i].m_board.at(y, x).score = reader[U"points"].arrayView()[y].arrayView()[x].get<int32>();
					fields[i].m_board.at(y, x).exist = true;
				}
			}
		}

	}
}

void Procon30::PublicFields::read() {
	
}

Procon30::PublicField Procon30::PublicFields::checkPublicField(const Game& game) {
	bool worng = false;
	for (int i = 0; i < 15; i++, worng = false) {
		if (game.field.boardSize.x != fields[i].boardSize.x)continue;
		if (game.field.boardSize.y != fields[i].boardSize.y)continue;
		for (int y : step(game.field.boardSize.y)) {
			for (int x : step(game.field.boardSize.x)) {
				if (game.field.m_board[y][x].score != fields[i].m_board[y][x].score) {
					worng = true;
					break;
				}
			}
			if (worng)break;
		}
		if (worng)continue;
		return PublicField(i + 2);
	}
	return PublicField(1);
}