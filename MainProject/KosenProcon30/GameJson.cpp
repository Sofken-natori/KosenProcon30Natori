#include "Game.hpp"

void Procon30::Game::parseAgentsData(Team& team, JSONValue object)
{
	if (team.agentNum == 0) {
		team.agentNum = object.arrayCount();
		team.agents = Array< Agent >(team.agentNum);

		for (int32 i = 0; i < object.arrayCount(); i++)
		{
			const JSONValue& agent = object.arrayView()[i];
			team.agents.at(i).agentID = agent[U"agentID"].get<int32>();
			team.agents.at(i).nowPosition.x = agent[U"x"].get<int32>();
			team.agents.at(i).nowPosition.y = agent[U"y"].get<int32>();
		}
	}
	else
	{
		for (int32 i = 0; i < object.arrayCount(); i++)
		{
			const JSONValue& agent = object.arrayView()[i];
			bool ok = false;
			for (size_t k = 0; k < team.agentNum; k++) {
				if (team.agents.at(k).agentID == agent[U"agentID"].get<int32>()) {
					team.agents.at(k).nowPosition.x = agent[U"x"].get<int32>();
					team.agents.at(k).nowPosition.y = agent[U"y"].get<int32>();
					ok = true;
					break;
				}
			}
			assert(ok);
		}
	}
}

void Procon30::Game::parseTeamsData(JSONValue object)
{
	{
		const auto& jsonFirstTeamID = (*object.arrayView().begin())[U"teamID"].get<int32>();

		if (this->teams.second.teamID == -1) {
			if (jsonFirstTeamID == this->teams.first.teamID) {
				{
					const auto& team = *object.arrayView().begin();
					//this->teams.first.teamID = team[U"teamID"].get<int32>();
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
			else {
				{
					const auto& team = *object.arrayView().begin();
					this->teams.second.teamID = team[U"teamID"].get<int32>();
					this->teams.second.tileScore = team[U"tilePoint"].get<int32>();
					this->teams.second.areaScore = team[U"areaPoint"].get<int32>();
					this->teams.second.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
					parseAgentsData(this->teams.second, team[U"agents"]);
				}
				{
					const auto& team = *(++object.arrayView().begin());
					//UNDONE:本番環境では入れること
					assert(this->teams.first.teamID == team[U"teamID"].get<int32>());
					this->teams.first.teamID = team[U"teamID"].get<int32>();
					this->teams.first.tileScore = team[U"tilePoint"].get<int32>();
					this->teams.first.areaScore = team[U"areaPoint"].get<int32>();
					this->teams.first.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
					parseAgentsData(this->teams.first, team[U"agents"]);
				}
			}
		}
		else if (jsonFirstTeamID == this->teams.first.teamID) {
			{
				const auto& team = *object.arrayView().begin();
				//this->teams.first.teamID = team[U"teamID"].get<int32>();
				this->teams.first.tileScore = team[U"tilePoint"].get<int32>();
				this->teams.first.areaScore = team[U"areaPoint"].get<int32>();
				this->teams.first.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
				parseAgentsData(this->teams.first, team[U"agents"]);
			}
			{
				const auto& team = *(++object.arrayView().begin());
				assert(this->teams.second.teamID == team[U"teamID"].get<int32>());
				this->teams.second.teamID = team[U"teamID"].get<int32>();
				this->teams.second.tileScore = team[U"tilePoint"].get<int32>();
				this->teams.second.areaScore = team[U"areaPoint"].get<int32>();
				this->teams.second.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
				parseAgentsData(this->teams.second, team[U"agents"]);
			}
		}
		else if (jsonFirstTeamID == this->teams.second.teamID) {
			{
				const auto& team = *object.arrayView().begin();
				//this->teams.second.teamID = team[U"teamID"].get<int32>();
				this->teams.second.tileScore = team[U"tilePoint"].get<int32>();
				this->teams.second.areaScore = team[U"areaPoint"].get<int32>();
				this->teams.second.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
				parseAgentsData(this->teams.second, team[U"agents"]);
			}
			{
				const auto& team = *(++object.arrayView().begin());
				assert(this->teams.first.teamID == team[U"teamID"].get<int32>());
				this->teams.first.teamID = team[U"teamID"].get<int32>();
				this->teams.first.tileScore = team[U"tilePoint"].get<int32>();
				this->teams.first.areaScore = team[U"areaPoint"].get<int32>();
				this->teams.first.score = team[U"tilePoint"].get<int32>() + team[U"areaPoint"].get<int32>();
				parseAgentsData(this->teams.first, team[U"agents"]);
			}
		}
		else {
			assert(jsonFirstTeamID == this->teams.first.teamID);
		}

	}
}

void Procon30::Game::parseActionsData(JSONValue object)
{
	{
		for (int32 i = 0; i < object.arrayCount(); i++)
		{
			//TODO: agentIDに合わせた保存先を選択するように変更この時ターンも確認すること。
			const JSONValue& action = object.arrayView()[i];
			const auto agentID = action[U"agentID"].get<int32>();
			const auto type = action[U"type"].getString();
			const auto dx = action[U"dx"].get<int32>();
			const auto dy = action[U"dy"].get<int32>();

			bool ok = false;
			{
				//TODO:このタイミングで整合性チェック

				auto& team = this->teams.first;
				for (auto& agent : team.agents) {
					if (agent.agentID == agentID) {
						ok = true;
						break;
					}
				}
			}
			{
				auto& team = this->teams.second;
				for (auto& agent : team.agents) {
					if (agent.agentID == agentID) {
						ok = true;
						break;
					}
				}
			}

			assert(ok);
		}
	}
}

void Procon30::Game::parseJson(FilePath fileName)
{
	s3d::JSONReader reader(fileName);

	assert(reader);

	{
		this->turn = reader[U"turn"].get<int32>();
		this->startedAtUnixTime = reader[U"startedAtUnixTime"].get<int32>();

		if (field.boardSize.x == 0 && field.boardSize.y == 0) {
			field.boardSize.y = reader[U"height"].get<int32>();
			field.boardSize.x = reader[U"width"].get<int32>();

			{
				for (int32 y = 0; y < reader[U"points"].arrayCount(); y++) {
					for (int32 x = 0; x < reader[U"points"].arrayView()[y].arrayCount(); x++) {
						field.m_board.at(y, x).score = reader[U"points"].arrayView()[y].arrayView()[x].get<int32>();
					}
				}
			}

			parseTeamsData(reader[U"teams"]);
			parseActionsData(reader[U"actions"]);
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
		else {
			assert(field.boardSize.y == reader[U"height"].get<int32>());
			assert(field.boardSize.x == reader[U"width"].get<int32>());


			{
				for (int32 y = 0; y < reader[U"points"].arrayCount(); y++) {
					for (int32 x = 0; x < reader[U"points"].arrayView()[y].arrayCount(); x++) {
						assert(field.m_board.at(y, x).score == reader[U"points"].arrayView()[y].arrayView()[x].get<int32>());
					}
				}
			}

			parseTeamsData(reader[U"teams"]);
			parseActionsData(reader[U"actions"]);

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

}

String encodeString(const Procon30::Team& team)
{
	/*
{
	"actions": [
		{
			"agentID": 9,
			"type": "move",
			"dx": 1,
			"dy": 1
		},
		{
			"agentID": 10,
			"type": "move",
			"dx": -1,
			"dy": -1
		}
	]
}
	*/

	String result;

	result += U"{\n";
	result += U"\t\"actions\": [\n";
	for (int32 i = 0; i < team.agentNum; i++) {
		result += U"\t\t{\n";
		result += U"\t\t\t\"agentID\":" + Format(team.agents.at(i).agentID) + U"\n";

		const String type = (Array<String>{ U"stay", U"move", U"remove" })[(int32)team.agents.at(i).action];
		result += U"\t\t\t\"type\":" + type + U"\n";
		result += U"\t\t\t\"dx\":" + Format(team.agents.at(i).nextPosition.x - team.agents.at(i).nowPosition.x) + U"\n";
		result += U"\t\t\t\"dy\":" + Format(team.agents.at(i).nextPosition.y - team.agents.at(i).nowPosition.y) + U"\n";

		result += U"\t\t}" + (i == team.agentNum - 1) ? U"\n" : U",\n";
	}

	result += U"\t]\n";
	result += U"}\n";

	return result;
}



void Procon30::Game::convertToJson(FilePath fileName)
{

	TextWriter tw(fileName);

	tw << encodeString(this->teams.first);

	tw.close();

	return;
}