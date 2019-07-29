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
		std::unordered_map<int32, int32> matchesConversionTable;

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

		//�󂯎����Json�̃p�X
		FilePath receiveJsonPath;

		//code������
		std::future<CURLcode> future;

		//�ʐM�����𔻕�
		//perform������true,Update�Ŏ󂯎������false
		bool nowConnecting;

		//�ʐM���̎����ԍ�(0-indexed)
		int32 connectionMatchNumber;

		//���������擾������
		int32 gotMatchInfomationNum;

		//�ʐM���s�������
		ConnectionType connectionType;

		//async�̃X�e�[�^�X��ǂݎ��܂�
		CommunicationState getState() const;

		//Needs:getState() == Done
		ConnectionStatusCode getResult();

		//callback�֐�
		static size_t callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream);

		//filepath����s�������R�s�[���܂��B
		String getPostData(const FilePath& filePath);

		//async�œ�������Ԃ��`�F�b�N����Result���Ƃ��Ă��܂��B
		//true : Done
		//false: other
		//������
		bool checkResult();
	public:
		//���\�p�ɂɏ�������邯��notify�K���K������Ă�����...?
		std::shared_ptr<Observer> observer;

		//�ϊ��e�[�u���̐ݒ�
		void setConversionTable(const Array<int>& arr);

		//MatchHandle�̏����������܂��B
		//Need:MatchesConversionTable.size() > 0
		void initilizeAllMatchHandles();

		//Main����thread�ɓ�����Ƃ��͂���...?
		//�����X�V
		void update();

		// ping - failed������v���O�����𗎂Ƃ��B
		bool pingServerConnectionTest();

		// host/matches��get���܂��B
		bool getAllMatchesInfomation();

		// host/matches/N��get���܂�
		bool getMatchInfomation();

		//Form��Loop
		void initilizeFormLoop();

		// host/matches/N/action��post���܂�
		bool checkPostAction();

		HTTPCommunication();
		~HTTPCommunication();

		//TODO: operator=�̒��g����������
		//DONT USE:This function is not implement
		HTTPCommunication& operator=(const Procon30::HTTPCommunication& right);
	};

}