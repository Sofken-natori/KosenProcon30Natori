#pragma once
#include "KosenProcon30.hpp"
#include "Field.hpp"
#include "Team.hpp"
#include "Agent.hpp"
namespace Procon30 {
	class VirtualServer
	{
	private:
		int32 player_count = 8;
		Array<Array<int32>> points;
		int32 random_number();
		void write_json();
		void input_point(Array<Array<int32>> &points);
	};
}
