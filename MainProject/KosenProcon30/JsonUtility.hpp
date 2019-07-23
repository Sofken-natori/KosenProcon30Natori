#pragma once
#include "KosenProcon30.hpp"
#include "Field.hpp"
#include "Team.hpp"
#include "Agent.hpp"

namespace Procon30 {

	class JsonWriter
	{
		Array<Array<int32>> points;
		Array<Array<int32>> tiles;
		Array<Array<int32>> agents1;
		Array<Array<int32>> agents2;
		int32 agent_count = 8;
		int32 width = 20;
		int32 height = 20;
	public:
		VirtualServer();
		void putPoint();
		void putAgent();
		void writeJson(FilePath path);
	};
}
