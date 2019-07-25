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

		//試合の数
		size_t matchNum;

		// gameIDの変換テーブル
		std::unordered_map<int, int> matchesConversionTable;

		//ハンドル
		CURL* pingHandle;
		CURL* matchesInfoHandle;
		Array<CURL*> getMatchHandles;
		Array<CURL*> postActionHandles;

		//Headerのlist
		curl_slist* postList = NULL;
		curl_slist* otherList = NULL;

		//receiveRawDataのmutex
		std::mutex receiveRawMtx;

		//内容がWriteされる変数
		String receiveRawData;

		//受け取ったJsonのパス
		FilePath receiveJsonPath;

		//codeが入る
		std::future<CURLcode> future;

		//通信中かを判別
		//performしたらtrue,Updateで受け取ったらfalse
		bool nowConnecting;

		//通信中の試合番号(0-indexed)
		int32 connectionMatchNumber;

		//通信を行った種別
		ConnectionType connectionType;

		//asyncのステータスを読み取ります
		CommunicationState getState() const;

		//Needs:getState() == Done
		ConnectionStatusCode getResult();

		//callback関数
		size_t callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream);

		//filepathから行動情報をコピーします。
		String getPostData(const FilePath& filePath);

	public:
		//結構頻繁に書き換わるけどnotifyガンガンやっていいの...?
		std::shared_ptr<Observer> observer;

		//変換テーブルの設定
		//未定義
		void setConversionTable(/* 任意の型 */);

		//MatchHandleの初期化をします。
		//Need:MatchesConversionTable.size() > 0
		//未定義
		void initilizeAllMatchHandles();

		// ping - failedしたらプログラムを落とす。
		void pingServerConnectionTest();

		//Mainからthreadに投げるときはこれ...?
		//随時更新
		void update();

		// host/matchesをgetします。
		//未定義
		void getAllMatchesInfomation();

		//FormのLoop
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