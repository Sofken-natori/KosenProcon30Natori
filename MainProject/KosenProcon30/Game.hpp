#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "HTTPCommunication.hpp"
#include "SendBuffer.hpp"
#include "Algorithm.hpp"

namespace Procon30 {

	class Observer;
	class Algorithm;

	class Game
	{
	private:
		static std::mutex HTTPWaitMtx;
		static std::condition_variable HTTPWaitCond;
		static bool dataReceived;
		static std::mutex ReceiveMtx;

		//����3�������`�F�b�N���K�v���c�_����
		void parseAgentsData(Team& team, JSONValue object);
		void parseTeamsData(JSONValue object);
		void parseActionsData(JSONValue object);
		std::thread thisThread;
		
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
		uint64 startedAtUnixTime;
		String matchTo;
		int32 turnMillis;
		int32 intervalMillis;

		//YASAI
		void updateData();
		void sendToHTTP();
		//thread�ɓ��������
		void Loop();

		//Main����Ăяo������
		void ThreadRun();

		void parseJson(FilePath fileName);

		//action�f�[�^��json�Ƀp�[�X���邾���̋@�\���������Ȃ��B
		void convertToJson(FilePath fileName);

		Stopwatch turnTimer;
		Stopwatch gameTimer;

		std::shared_ptr<Observer> observer;
		std::shared_ptr<SendBuffer> buffer;
		std::shared_ptr<std::atomic<bool>> programEnd;

		//����������teamID�ݒ肵�Ă����B =L Team�̃R���X�g���N�^�ł���Ă�B
		Game();
		~Game();

		//UNDONE: When member variable changed, You must check this function. 
		Game& operator=(const Procon30::Game& right);

		std::unique_ptr<Algorithm> algorithm;

		PublicField fieldType;
	};

}