#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"
#include "SendBuffer.hpp"

namespace Procon30 {

	class Observer;

	class Game
	{
	private:
		static std::mutex HTTPWaitMtx;
		static std::condition_variable HTTPWaitCond;
		static bool dataReceived;
		static std::mutex ReceiveMtx;

		void parseAgentsData(Team& team, JSONValue object);
		void parseTeamsData(JSONValue object);
		void parseActionsData(JSONValue object);

	public:
		//NEED:HTTPCommunication����Ăяo��
		//����v�����񂾂��ǌʂɋN�������ق������ɂɂ����ėǂ���������Ȃ��̂�.jp
		static void HTTPReceived();

		Field field;

		//first: MyTeam
		//second: EnemyTeam
		std::pair<Team, Team> teams;

		bool isSearchFinished;

		//nasatame
		int32 calculateScore(TeamColor color);
		int32 calculateTileScore(TeamColor color);

		int32 turn;
		int32 MaxTurn;
		//�C�ӂɌ��܂邠��
		int32 gameID;
		//0,1,2...
		int32 gameNum;
		int32 startedAtUnixTime;
		String matchTo;
		int32 turnMillis;

		//YASAI
		void updateData();
		void sendToHTTP();

		//nasatame
		//TODO : AgentID�Ȃǂ�O�����ĕ����Ă��Ȃ��̂œK�������A��ŏ���������K�v����
		void parseJson(FilePath fileName);

		//action�f�[�^��json�Ƀp�[�X���邾���̋@�\���������Ȃ��B
		void convertToJson(FilePath fileName);

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;
		std::shared_ptr<SendBuffer> buffer;

		//TODO:����������teamID�ݒ肵�Ă����B
		Game();
		~Game();

		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);
	};

}