#include "Game.hpp"
#include "Observer.hpp"

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
	//Wait��
	std::unique_lock<std::mutex> lockWait(HTTPWaitMtx);
	if (!dataReceived) {
		HTTPWaitCond.wait(lockWait);
	}

	//������@���E��G.obj
	FilePath path = Format(U"json/", gameNum, U"/nowField.json");
	parseJson(path);

	//����M�ύX��
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

Procon30::Game::Game()
{
	isSearchFinished = false;
	MaxTurn = 60;
	turn = 0;
	startedAtUnixTime = -1;
}

Procon30::Game::~Game()
{
}

Procon30::Game& Procon30::Game::operator=(const Procon30::Game& right)
{
	this->field = right.field;
	this->gameTimer = right.gameTimer;
	this->turn = right.turn;
	this->MaxTurn = right.MaxTurn;
	this->turnTimer = right.turnTimer;
	this->isSearchFinished = right.isSearchFinished;

	return (*this);
}
