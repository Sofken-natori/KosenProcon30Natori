#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"

namespace Procon30 {

	class HTTPCommunication
	{
	public:
		int32 teamID;
		s3d::Array<int32> agentsID;

		HTTPCommunication();
		~HTTPCommunication();

		void parseJson(String fileName,Field &field,Team &team);


	};

}