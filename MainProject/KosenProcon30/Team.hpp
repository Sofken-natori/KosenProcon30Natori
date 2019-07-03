#pragma once
#include "KosenProcon30.hpp"
#include "Agent.hpp"

namespace Procon30 {

	class Team
	{
	public:
		TeamColor color;
		int32 score;
		int32 agentNum;
		Array<Agent> agents;
		Team();
		~Team();
	};

}