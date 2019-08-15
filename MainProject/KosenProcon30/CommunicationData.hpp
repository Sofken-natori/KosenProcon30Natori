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

		//�����̐�
		size_t matchNum;

		// gameID�̕ϊ��e�[�u��
		std::unordered_map<int32, int32> matchesConversionTable;

		//�󂯎����Json�̃p�X
		FilePath receiveJsonPath;

		//�ʐM�����𔻕�
		//perform������true,Update�Ŏ󂯎������false
		bool nowConnecting;

		//�ʐM���̎����ԍ�(0-indexed)
		int32 connectionMatchNumber;

		//���������擾������
		int32 gotMatchInfomationNum;

		//�ʐM���s�������
		ConnectionType connectionType;

		//�ʐM����
		ConnectionStatusCode connectionCode;
	};
}
