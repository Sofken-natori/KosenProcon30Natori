#pragma once
#include "KosenProcon30.hpp"

namespace Procon30 {
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

	struct CommunicationData {
		//token
		String token;

		//URL
		String host;

		//試合の数
		size_t matchNum;

		// gameIDの変換テーブル
		std::unordered_map<int32, int32> matchesConversionTable;

		//受け取ったJsonのパス
		FilePath receiveJsonPath;

		//通信中かを判別
		//performしたらtrue,Updateで受け取ったらfalse
		bool nowConnecting;

		//通信中の試合番号(0-indexed)
		int32 connectionMatchNumber;

		//試合情報を取得した数
		int32 gotMatchInfomationNum;

		//通信を行った種別
		ConnectionType connectionType;

		//通信結果
		ConnectionStatusCode connectionCode;
	};
}
