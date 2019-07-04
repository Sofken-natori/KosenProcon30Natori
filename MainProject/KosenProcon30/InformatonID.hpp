#pragma once
#include "KosenProcon30.hpp"

namespace Procon30 {
	struct InformationID {
		int32 teamID;
		Array<int32> AgentIDs;
		InformationID();
		InformationID(int32 teamid, Array<int32> agentids);
	};
}