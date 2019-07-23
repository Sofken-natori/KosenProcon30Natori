#include "HTTPCommunication.hpp"
#include "Observer.hpp"
#include "Game.hpp"

void Procon30::HTTPCommunication::jsonDistribute()
{

}

void Procon30::HTTPCommunication::getServer()
{


	Procon30::Game::HTTPReceived();
}

void Procon30::HTTPCommunication::postServer()
{
	FilePath path = buffer->getPath();


}

Procon30::HTTPCommunication::HTTPCommunication()
	:buffer(new SendBuffer())
{
	
}

Procon30::HTTPCommunication::~HTTPCommunication()
{
}

Procon30::InformationID::InformationID()
{
}

inline Procon30::InformationID::InformationID(int32 teamid, Array<int32> agentids) : teamID(teamid), AgentIDs(agentids) {};
