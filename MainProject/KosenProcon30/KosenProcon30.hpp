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

	constexpr bool virtualServerMode = true;

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
		//���ł��Ȃ����
		Null,

		//Code:200,201(post)
		//����
		OK,

		//Code:400
		//�����̊J�n�O�ɃA�N�Z�X�����ꍇ
		//UnixTime�����Ă���̂ŗv�m�F
		TooEarly,

		//Code:400
		//�C���^�[�o�����Ȃ�TooEarly�ȊO�ŉ񓚂̎�t���s���Ă��Ȃ����ԂɃA�N�Z�X�����ꍇ
		//�����A���ꂪ�o����O�ɑ��M���������Ă��Ȃ�����Stay�s���̉\���������B
		//UnixTime�����Ă���̂ŗv�m�F
		UnacceptableTime,

		//Code:400
		//�Q�����Ă��Ȃ������ɑ΂��郊�N�G�X�g
		//�[���ȃG���[�A�����I���������߂���B
		InvailedMatches,

		//Code:401
		//�g�[�N�����Ԉ���Ă���������͑��݂��Ȃ��ꍇ
		//�[���ȃG���[�A�����I���������߂���B
		InvaildToken,

		//curl_easy_perform() is failed.
		//���Ԃ�I�������^�C���A�E�g�A�đ����ĘA������悤�������狭���I���������߂���B
		ConnectionLost,

		//Unknown Error
		//Code��200,201,400,401�ȊO�̎�
		//����o���炾�������󂯎����json�ɂȂ񂩂��珑���Ă���
		//json/buffer.json��ǂ�
		//�[���ȃG���[�A�����I���������߂���B
		UnknownError
	};
	
	template <class... Args, std::enable_if_t<s3d::detail::format_validation<Args...>::value>* = nullptr>
	inline void SafeConsole(const Args & ... args) {
		static std::mutex consoleMtx;
		std::lock_guard<std::mutex> lock(consoleMtx);
		Console << Format(args...);
	}
}