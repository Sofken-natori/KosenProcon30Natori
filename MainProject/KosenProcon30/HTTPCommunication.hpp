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
				//fieldInfo
				int32 height;
				int32 width;
				int32 turn;
				int32 startedAtUnixTime;

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

		//codeが入る
		std::future<CURLcode> future;

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
		//あとは随時書く
		bool checkResult();

		const FilePath jsonBuffer = U"json/buffer.json";
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

		//FormのLoop 実装はForm.cpp
		//Turn0のタイミングでのみ呼ばれることを想定して書きます。
		void initilizeFormLoop();

		// host/matches/N/actionにpostします
		bool checkPostAction();

		HTTPCommunication();
		~HTTPCommunication();

		//GUIがobserverからGameを手に入れる際にmatchSizeが必要なのでgeterを追加します。
		size_t getMatchNum() const;

		//TODO: operator=の中身を実装する
		//DONT USE:This function is not implement
		HTTPCommunication& operator=(const Procon30::HTTPCommunication& right);
	};

}