#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"

namespace Procon30 {

	class Observer;

	class HTTPCommunication
	{
	public:
		

		
		s3d::Array<int32> agentsID;
		std::shared_ptr<Observer> observer;
		
		//receive
		
		void jsonDistribute();

		//send


		HTTPCommunication();
		~HTTPCommunication();

		


	};

}