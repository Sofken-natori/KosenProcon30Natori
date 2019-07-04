#include "Game.hpp"

void Procon30::Game::parseAgentsData(Team& team, JSONValue object)
{
	team.agentNum = object.arrayCount();
	team.agents = Array< Agent >(team.agentNum);
	{
		for (int32 i = 0; i < object.arrayCount(); i++)
		{
			const JSONValue& agent = object.arrayView()[i];
			team.agents[i].agentID = agent[U"agentID"].get<int32>();
			team.agents[i].nowPosition.x = agent[U"x"].get<int32>();
			team.agents[i].nowPosition.y = agent[U"y"].get<int32>();
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
		parseAgentsData(this->teams.first, team[U"agents"]);
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

void Procon30::Game::parseJson(String fileName)
{
	s3d::JSONReader reader(fileName);

	assert(reader);

	{
		field.boardSize.y = reader[U"height"].get<int32>();
		field.boardSize.x = reader[U"width"].get<int32>();
		{
			for(int32 y = 0; y < reader[U"points"].arrayCount(); y++){
				for (int32 x = 0; x < reader[U"points"].arrayView()[y].arrayCount(); x++) {
					field.m_board[y][x].score = reader[U"points"].arrayView()[y].arrayView()[x].get<int32>();
				}
			}
		}
		this->startedAtUnixTime = reader[U"startedAtUnixTime"].get<int32>();

		parseTeamsData(reader[U"teams"]);
		parseActionsData(reader[U"actions"]);


		this->turn = reader[U"turn"].get<int32>();
		{
			for (int32 y = 0; y < reader[U"tiled"].arrayCount(); y++) {
				for (int32 x = 0; x < reader[U"tiled"].arrayView()[y].arrayCount(); x++) {
					int32 tileColor = reader[U"tiled"].arrayView()[y].arrayView()[x].get<int32>();
					field.m_board[y][x].color = (tileColor == this->teams.first.teamID) ? (TeamColor::Blue)
						: tileColor == this->teams.second.teamID ? (TeamColor::Red)
						: TeamColor::None;
				}
			}
		}
	}

}

void Procon30::Game::convertToJson(String fileName)
{
}