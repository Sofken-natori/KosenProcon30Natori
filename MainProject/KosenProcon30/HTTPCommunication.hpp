#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "SendBuffer.hpp"
#include "CommunicationData.hpp"

namespace Procon30 {

	class Observer;
	class Game;

	class HTTPCommunication
	{
	private:

		struct Form {

			struct school {
				//matchInfo
				String matchTo;
				int32 teamID;
				int32 turns;
				int32 id;
				int32 intervalMillis;
				int32 turnMillis;
				//fieldInfo
				int32 height;
				int32 width;
				int32 turn;
				uint64 startedAtUnixTime;

				bool checked;
			};

			Font infoFont;

			Array<school> schools;

			void draw() const;
			bool update();
		};

		CommunicationData comData;

		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;

		//�n���h��
		CURL* pingHandle;
		CURL* matchesInfoHandle;
		Array<CURL*> getMatchHandles;
		Array<CURL*> postActionHandles;

		//Header��list
		curl_slist* postList = NULL;
		curl_slist* otherList = NULL;

		//receiveRawData��mutex
		static std::mutex receiveRawMtx;

		//���e��Write�����ϐ�
		static String receiveRawData;

		//code������
		std::future<CURLcode> future;

		//async�̃X�e�[�^�X��ǂݎ��܂�
		[[nodiscard]] CommunicationState getState() const;

		//Needs:getState() == Done
		[[nodiscard]] ConnectionStatusCode getResult();

		//callback�֐�
		static size_t callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream);

		//filepath����s�������R�s�[���܂��B
		String getPostData(const FilePath& filePath);

		//async�œ�������Ԃ��`�F�b�N����Result���Ƃ��Ă��܂��B
		//true : Done
		//false: other
		//���Ƃ͐�������
		[[nodiscard]] bool checkResult();

		const FilePath jsonBuffer = U"json/buffer.json";

		bool isFormLoop;

		std::thread thisThread;
	public:
		//���\�p�ɂɏ�������邯��notify�K���K������Ă�����...?
		std::shared_ptr<Observer> observer;

		//�ϊ��e�[�u���̐ݒ�
		void setConversionTable(const Array<int>& arr);

		//MatchHandle�̏����������܂��B
		//Need:MatchesConversionTable.size() > 0
		void initilizeAllMatchHandles();

		//�����X�V
		void update();

		//thread�ɓ��������
		void Loop();

		//Main����Ăяo������
		void ThreadRun();

		// ping - failed������v���O�����𗎂Ƃ��B
		bool pingServerConnectionTest();

		// host/matches��get���܂��B
		bool getAllMatchesInfomation();

		// host/matches/N��get���܂�
		bool getMatchInfomation();

		//Form��Loop ������Form.cpp
		//Turn0�̃^�C�~���O�ł̂݌Ă΂�邱�Ƃ�z�肵�ď����܂��B
		[[nodiscard]] Array<Form::school> initilizeFormLoop();

		// host/matches/N/action��post���܂�
		bool checkPostAction();

		HTTPCommunication();
		~HTTPCommunication();

		//GUI��observer����Game����ɓ����ۂ�matchSize���K�v�Ȃ̂�geter��ǉ����܂��B
		[[nodiscard]] size_t getMatchNum() const;

		[[nodiscard]] CommunicationData getComData() const;

		[[nodiscard]] std::shared_ptr<SendBuffer> getBufferPtr();

		[[nodiscard]] int32 getGameIDfromGameNum(const int32& num);

		//TODO: operator=�̒��g����������
		//DONT USE:This function is not implement
		HTTPCommunication& operator=(const Procon30::HTTPCommunication& right);
	};

}