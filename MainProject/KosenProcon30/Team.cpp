#include "Team.hpp"

Procon30::Team::Team()
{
}


Procon30::Team::~Team()
{
}

void Procon30::Team::calcScore(Field& field)
{

	Array<Array<bool>> visit(MaxFieldY, Array<bool>(MaxFieldX, false));

	Array<Point> q;

	int fieldSizeX = 12;
	int fieldSizeY = 12;

	for (auto y : step(MaxFieldY)) {
		if (field.m_board.at(y, 0).exist == false) {
			fieldSizeY = y + 1;
			break;
		}
	}
	for (auto x : step(MaxFieldX)) {
		if (field.m_board.at(0, x).exist == false) {
			fieldSizeX = x + 1;
			break;
		}
	}
	for (auto y : step(fieldSizeY)) {
		q.push_back({ 0, y });
		q.push_back({ fieldSizeX - 1,y });
	}
	for (auto x : step(fieldSizeX)) {
		q.push_back({ x , 0 });
		q.push_back({ x , fieldSizeY - 1 });
	}

	while (!q.isEmpty()) {

		auto now = q.front();
		q.pop_front();

		if (0 <= now.y && now.y < fieldSizeY && 0 <= now.x && now.x < fieldSizeX) {

			if (visit.at(now.y).at(now.x))
				continue;
			visit.at(now.y).at(now.x) = true;

			if (field.m_board.at(now.y, now.x).color != this->color) {

				q.push_back({ now.x - 1	,now.y });
				q.push_back({ now.x		,now.y + 1 });
				q.push_back({ now.x + 1 ,now.y });
				q.push_back({ now.x		,now.y - 1 });

			}

		}

	}

	int resultScore = 0;
	int resultTileScore = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (this->color != field.m_board.at(y, x).color && visit.at(y).at(x) == false) {
				resultScore += abs(field.m_board.at(y, x).score);
			}
			if (field.m_board.at(y, x).color == this->color) {
				resultTileScore += field.m_board.at(y, x).score;
			}
		}
	}

	resultScore += resultTileScore;

	this->score = resultScore;
	this->tileScore = resultTileScore;

	return;
}
