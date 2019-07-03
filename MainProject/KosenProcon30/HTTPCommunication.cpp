#include "HTTPCommunication.hpp"
#include "Observer.hpp"
#include "Game.hpp"

void Procon30::HTTPCommunication::jsonDistribute()
{
	//




	Procon30::Game::HTTPReceived();
}

Procon30::HTTPCommunication::HTTPCommunication()
{
}

Procon30::HTTPCommunication::~HTTPCommunication()
{
}

Procon30::HTTPCommunication& Procon30::HTTPCommunication::operator=(const Procon30::HTTPCommunication& right)
{
	this->MyTeamInformationID = right.MyTeamInformationID;
	return (*this);
}

Procon30::InformationID::InformationID()
{
}

inline Procon30::InformationID::InformationID(int32 teamid, Array<int32> agentids) : teamID(teamid), AgentIDs(agentids) {};
