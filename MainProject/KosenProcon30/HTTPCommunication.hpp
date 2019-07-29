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
		std::unordered_map<int32, int32> matchesConversionTable;

		//ハンドル
		CURL* pingHandle;
		CURL* matchesInfoHandle;
		Array<CURL*> getMatchHandles;
		Array<CURL*> postActionHandles;

		//Headerのlist
		curl_slist* postList = NULL;
		curl_slist* otherList = NULL;

		//receiveRawDataのmutex
		static std::mutex receiveRawMtx;

		//内容がWriteされる変数
		static String receiveRawData;

		//受け取ったJsonのパス
		FilePath receiveJsonPath;

		//codeが入る
		std::future<CURLcode> future;

		//通信中かを判別
		//performしたらtrue,Updateで受け取ったらfalse
		bool nowConnecting;

		//通信中の試合番号(0-indexed)
		int32 connectionMatchNumber;

		//試合情報を取得した数
		int32 gotMatchInfomationNum;

		//通信を行った種別
		ConnectionType connectionType;

		//asyncのステータスを読み取ります
		CommunicationState getState() const;

		//Needs:getState() == Done
		ConnectionStatusCode getResult();

		//callback関数
		static size_t callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream);

		//filepathから行動情報をコピーします。
		String getPostData(const FilePath& filePath);

		//asyncで投げた状態をチェックしてResultをとってきます。
		//true : Done
		//false: other
		//未完成
		bool checkResult();
	public:
		//結構頻繁に書き換わるけどnotifyガンガンやっていいの...?
		std::shared_ptr<Observer> observer;

		//変換テーブルの設定
		void setConversionTable(const Array<int>& arr);

		//MatchHandleの初期化をします。
		//Need:MatchesConversionTable.size() > 0
		void initilizeAllMatchHandles();

		//Mainからthreadに投げるときはこれ...?
		//随時更新
		void update();

		// ping - failedしたらプログラムを落とす。
		bool pingServerConnectionTest();

		// host/matchesをgetします。
		bool getAllMatchesInfomation();

		// host/matches/Nをgetします
		bool getMatchInfomation();

		//FormのLoop
		void initilizeFormLoop();

		// host/matches/N/actionにpostします
		bool checkPostAction();

		HTTPCommunication();
		~HTTPCommunication();

		//TODO: operator=の中身を実装する
		//DONT USE:This function is not implement
		HTTPCommunication& operator=(const Procon30::HTTPCommunication& right);
	};

}