#pragma once
#include "KosenProcon30.hpp"
#include "Observer.hpp"

namespace Procon30 {

	class GUI
	{
	private:
		//DON'T DELETE
		std::shared_ptr<Observer> observer;

		//�������ƂɃT�C�Y���ς�����肷��ϐ��B
		std::array<RectF, MaxGameNumber> teamTile;
		std::array<double, MaxGameNumber> correctedTileSize;
		std::array<Font, MaxGameNumber>  scoreFont;
		Array<String> viewerStrings;

		//�ȉ��S������ʂ��ċ��ʂ̂���

		//��Ƀ^�C���̐F�Ŏg�p
		Color myTeamColor;
		Color enemyTeamColor;
		Color noneTeamColor;

		//�\�����ɑI�ʂ��邽�߂̕ϐ�
		size_t match;
		size_t drawType;

		//�����`��ɕK�v�ȕϐ�
		Font font;

		//�\���̍ۂɂ��������ɂȂ锠
		Rect viewerBox;


		Font test;
	public:

		void draw();
		//�J�Ò��̎������ύX���ꂽ�Ƃ�
		//�V�K�Ɏ������J�n�����ۂɏ����f�[�^�𐶐��������̂ŌĂ�ł����Ă�������
		void dataUpdate();

		GUI();
		~GUI();

		std::shared_ptr<Observer> getObserver();
	};

}
