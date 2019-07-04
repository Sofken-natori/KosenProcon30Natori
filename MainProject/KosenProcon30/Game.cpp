#include "Game.hpp"

bool Procon30::Game::dataReceived = false;
std::mutex Procon30::Game::HTTPWaitMtx;
std::condition_variable Procon30::Game::HttpWaitCond;
std::mutex Procon30::Game::ReceiveMtx;

void Procon30::Game::parseAgentsData(Team& team, JSONValue object)
{
	team.agentNum = object.arrayCount();
	team.agents = Array< Agent >(team.agentNum);
	{
		int num = 0;
		for (const auto& agent : object.arrayView()) {
			team.agents[num].agentID = agent[U"agentID"].get<int32>();
			team.agents[num].nowPosition.x = agent[U"x"].get<int32>();
			team.agents[num].nowPosition.y = agent[U"y"].get<int32>();
			num++;
		}
	}
}

void Procon30::Game::parseTeamsData(JSONValue object)
{
	{
		const auto& team = *object.arrayView().begin();
		this->teams.first.teamID = team[U"teamID"].get<int32>();
		this->teams.first.tileScore = team[U"tilePoint"].get<int32>();
		this->teams.first.areaScore = team[U"areaPoint"].get<int32>();
		this->teams.first.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
		parseAgentsData(this->teams.first,team[U"agents"]);
	}
	{
		const auto& team = *(++object.arrayView().begin());
		this->teams.second.teamID = team[U"teamID"].get<int32>();
		this->teams.second.tileScore = team[U"tilePoint"].get<int32>();
		this->teams.second.areaScore = team[U"areaPoint"].get<int32>();
		this->teams.second.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
		parseAgentsData(this->teams.second, team[U"agents"]);
	}
}

void Procon30::Game::parseActionsData(JSONValue object)
{
}

void Procon30::Game::HTTPReceived()
{
	std::lock_guard<std::mutex> lock(HTTPWaitMtx);
	dataReceived = true;
	HttpWaitCond.notify_all();
}

int32 Procon30::Game::calculateScore(TeamColor color)
{
	Array<Array<bool>> visit(MaxFieldY, Array<bool>(MaxFieldX, false));

	Array<Point> q;

	int fieldSizeX = -1;
	int fieldSizeY = -1;

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

void Procon30::Game::dataUpdate()
{
	//Waitïî
	std::unique_lock<std::mutex> lockWait(HTTPWaitMtx);
	while (!dataReceived) {
		HttpWaitCond.wait(lockWait);
	}

	//èàóùïî

	//ñ¢éÛêMïœçXïî
	std::lock_guard<std::mutex> lockFlag(ReceiveMtx);
	dataReceived = false;
}

void Procon30::Game::parseJson(String fileName)
{
	s3d::JSONReader reader(fileName);

	assert(reader);

	{
		field.boardSize.y = reader[U"height"].get<int32>();
		field.boardSize.x = reader[U"width"].get<int32>();
		{
			int32 y = 0;
			for (const auto& row : reader[U"points"].arrayView()) {
				int32 x = 0;
				for (const auto& point : row.arrayView()) {
					field.m_board[y][x].score = point.get<int>();
					x++;
				}
				y++;
			}
		}
		this->startedAtUnixTime = reader[U"startedAtUnixTime"].get<int32>();

		parseTeamsData(reader[U"teams"]);
		parseActionsData(reader[U"actions"]);


		this->turn = reader[U"turn"].get<int32>();
		{
			int32 y = 0;
			for (const auto& row : reader[U"tiled"].arrayView()) {
				int32 x = 0;
				for (const auto& tile : row.arrayView()) {
					field.m_board[y][x].color = (tile.get<int>() == this->teams.first.teamID) ? (TeamColor::Blue)
						: tile.get<int>() == this->teams.second.teamID ? (TeamColor::Red)
						: TeamColor::None;
					x++;
				}
				y++;
			}
		}
	}

}

void Procon30::Game::convertToJson(String fileName)
{
}

Procon30::Game::Game()
{

}


Procon30::Game::~Game()
{
}

Procon30::Game& Procon30::Game::operator=(const Procon30::Game& right)
{
	this->field = right.field;
	this->gameTimer = right.gameTimer;
	this->turn = right.turn;
	this->Maxturn = right.Maxturn;
	this->turnTimer = right.turnTimer;
	this->isSearchFinished = right.isSearchFinished;

	return (*this);
}
