#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "SendBuffer.hpp"
#include "InformatonID.hpp"

namespace Procon30 {

	class Observer;
	class Game;

	class HTTPCommunication
	{
	private:
		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;
		//����
		String token;
		String host;
	public:
		InformationID MyTeamInformationID;
		Array<InformationID> EnemyTeamInformationIDs;
		
		std::shared_ptr<Observer> observer;

		// gameID�̕ϊ��e�[�u��

		// ping - failed������v���O�����𗎂Ƃ��B
		void pingServerConnectionTest();


		// host/matches��get����json���󂯎��܂��B
		FilePath getAllMatchesInfomation();

		// 
		void initilizeFormLoop();
		
		//receive
		
		//DONT USE:This function is not implement
		void jsonDistribute();

		//DONT USE:This function is not implement
		void getServer();

		//DONT USE:This function is not implement
		void postServer();

		HTTPCommunication();
		~HTTPCommunication();


	};

}