#pragma once
#include "KosenProcon30.hpp"
#include <Siv3D.hpp>
class Agent
{
public:
	Action action;
	Point nextPosition;
	Point nowPosition;
	Agent();
	~Agent();
};

