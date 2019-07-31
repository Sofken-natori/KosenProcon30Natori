#pragma once

#include <Siv3D.hpp>
#include <mutex>
#include <condition_variable>
#include <curl/curl.h>
#include <future>

namespace Procon30 {

	constexpr int32 MaxFieldX = 20;
	constexpr int32 MaxFieldY = 20;
	constexpr int32 TileSize = 50;
	constexpr int32 SideAreaX = 400;
	constexpr Size WindowSize{ MaxFieldX * TileSize + SideAreaX * 2, MaxFieldY * TileSize };
	constexpr size_t MaxGameNumber = 3;

	enum class Action {
		Stay,
		Move,
		Remove
	};

	enum class TeamColor {
		None,
		Blue,
		Red
	};

	enum class ConnectionStatusCode : uint32 {
		//何でもない状態
		Null,

		//Code:200
		//正常
		OK,

		//Code:400
		//試合の開始前にアクセスした場合
		//UnixTimeも来ているので要確認
		TooEarly,

		//Code:400
		//インターバル中などTooEarly以外で回答の受付を行っていない時間にアクセスした場合
		//もし、これが出たら前に送信が完了していない限りStay行動の可能性が高い。
		//UnixTimeも来ているので要確認
		UnacceptableTime,

		//Code:400
		//参加していない試合に対するリクエスト
		//深刻なエラー、強制終了をお勧めする。
		InvailedMatches,

		//Code:401
		//トークンが間違っているもしくは存在しない場合
		//深刻なエラー、強制終了をお勧めする。
		InvaildToken,

		//curl_easy_perform() is failed.
		//たぶん鯖落ちかタイムアウト、再送して連続するようだったら強制終了をお勧めする。
		ConnectionLost
	};
}