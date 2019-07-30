#pragma once
#include "KosenProcon30.hpp"
#include "Field.hpp"
#include "Team.hpp"
#include "Agent.hpp"

namespace Procon30 {

	class JsonUtility
	{

	private:
		static void parseJsonToAgentsData(Team& team, JSONValue object);
		static void parseJsonToTeamsData(std::pair<Team,Team>& teams,JSONValue object);
		static void parseJsonToActionsData();

	public:
	};
}
