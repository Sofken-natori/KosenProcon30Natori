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
		//あれ
		String token;
		String host;
	public:
		InformationID MyTeamInformationID;
		Array<InformationID> EnemyTeamInformationIDs;
		
		std::shared_ptr<Observer> observer;

		// gameIDの変換テーブル

		// ping - failedしたらプログラムを落とす。
		void pingServerConnectionTest();


		// host/matchesをgetしてjsonを受け取ります。
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