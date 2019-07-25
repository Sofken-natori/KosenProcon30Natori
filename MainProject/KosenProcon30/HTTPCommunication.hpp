#pragma once
#include "KosenProcon30.hpp"
#include "Team.hpp"
#include "Field.hpp"
#include "SendBuffer.hpp"

namespace Procon30 {

	class Observer;
	class Game;

	enum class CommunicationState {
		Null,
		Done,
		Connecting,
	};

	enum class ConnectionType {
		Null,
		Ping,
		AllMatchesInfo,
		MatchInfomation,
		PostAction,
	};

	class HTTPCommunication
	{
	private:
		//DONT DELETE
		std::shared_ptr<SendBuffer> buffer;

		//token
		String token;

		//URL
		String host;

		//�����̐�
		size_t matchNum;

		// gameID�̕ϊ��e�[�u��
		std::unordered_map<int, int> matchesConversionTable;

		//�n���h��
		CURL* pingHandle;
		CURL* matchesInfoHandle;
		Array<CURL*> getMatchHandles;
		Array<CURL*> postActionHandles;

		//Header��list
		curl_slist* postList = NULL;
		curl_slist* otherList = NULL;

		//receiveRawData��mutex
		std::mutex receiveRawMtx;

		//���e��Write�����ϐ�
		String receiveRawData;

		//�󂯎����Json�̃p�X
		FilePath receiveJsonPath;

		//code������
		std::future<CURLcode> future;

		//�ʐM�����𔻕�
		//perform������true,Update�Ŏ󂯎������false
		bool nowConnecting;

		//�ʐM���̎����ԍ�(0-indexed)
		int32 connectionMatchNumber;

		//�ʐM���s�������
		ConnectionType connectionType;

		//async�̃X�e�[�^�X��ǂݎ��܂�
		CommunicationState getState() const;

		//Needs:getState() == Done
		ConnectionStatusCode getResult();

		//callback�֐�
		size_t callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream);

		//filepath����s�������R�s�[���܂��B
		String getPostData(const FilePath& filePath);

	public:
		//���\�p�ɂɏ�������邯��notify�K���K������Ă�����...?
		std::shared_ptr<Observer> observer;

		//�ϊ��e�[�u���̐ݒ�
		//����`
		void setConversionTable(/* �C�ӂ̌^ */);

		//MatchHandle�̏����������܂��B
		//Need:MatchesConversionTable.size() > 0
		//����`
		void initilizeAllMatchHandles();

		// ping - failed������v���O�����𗎂Ƃ��B
		void pingServerConnectionTest();

		//Main����thread�ɓ�����Ƃ��͂���...?
		//�����X�V
		void update();

		// host/matches��get���܂��B
		//����`
		void getAllMatchesInfomation();

		//Form��Loop
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