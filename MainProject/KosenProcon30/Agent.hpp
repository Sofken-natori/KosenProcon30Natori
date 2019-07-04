#pragma once
#include "KosenProcon30.hpp"
#include <Siv3D.hpp>

namespace Procon30 {

	class Agent
	{
	public:
		Action action;
		Point nextPosition;
		Point nowPosition;
		int32 agentID;

		Agent();
		~Agent();
	};

}