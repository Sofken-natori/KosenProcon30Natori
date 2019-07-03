#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"

namespace Procon30 {

	class Observer;
	class Game;

	struct InformationID {
		int32 teamID;
		Array<int32> AgentIDs;
		InformationID();
		InformationID(int32 teamid, Array<int32> agentids);
	};

	class HTTPCommunication
	{
	public:
		

		InformationID MyTeamInformationID;
		
		
		std::shared_ptr<Observer> observer;
		
		//receive
		
		//DONT USE:This function is not implement
		void jsonDistribute();

		//send

		HTTPCommunication();
		~HTTPCommunication();

		//UNDONE: When member variable changed, You must check this function. 
		HTTPCommunication& operator=(const Procon30::HTTPCommunication& right);


	};

}