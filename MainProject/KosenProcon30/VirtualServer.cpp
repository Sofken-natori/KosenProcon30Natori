#include "VirtualServer.hpp"


Procon30::VirtualServer::VirtualServer()
{
	width = Random(10, MaxFieldX);
	height = Random(10, MaxFieldY);
	agent_count = (width * height + 39) / 50;
	putPoint(0);
	putAgent();
}

void Procon30::VirtualServer::writeJson(FilePath path)
{
	String s;
	s += U"{\n";
	s += U"\t\"width\": " + Format(width) + U",\n";
	s += U"\t\"height\": " + Format(height) + U",\n";
	//points
	s += U"\t\"points\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			if (j == width - 1) {
				s += Format(field.m_board[i][j].score);
			}
			else {
				s += Format(field.m_board[i][j].score) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	s += U"\t\"startedAtUnixTime\": 0,\n";
	s += U"\t\"turn\": 0,\n";
	//tiled
	s += U"\t\"tiled\": [\n";
	for (int i = 0; i < height; i++) {
		s += U"\t\t[\n";
		s += U"\t\t\t";
		for (int j = 0; j < width; j++) {
			if (j == width - 1) {
				s += Format(tile[i][j]);
			}
			else {
				s += Format(tile[i][j]) + U",";
			}
		}
		if (i == height - 1) {
			s += U"\n\t\t]\n";
		}
		else {
			s += U"\n\t\t],\n";
		}
	}
	s += U"\t],\n";
	//teams
	s += U"\t\"teams\": [\n";
	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": 1,\n";
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.first.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.first.agents[i].nowPosition.x) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.first.agents[i].nowPosition.y) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	int32 sum = 0;
	for (int i = 0; i < agent_count; i++) {
		sum += field.m_board[teams.first.agents[i].nowPosition.y][teams.first.agents[i].nowPosition.x].score;
	}
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.first.color)) + U"\n";
	s += U"\t\t},\n";

	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": 11,\n";
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(teams.second.agents[i].agentID) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(teams.second.agents[i].nowPosition.x) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(teams.second.agents[i].nowPosition.y) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": " + Format(calculateScore(teams.second.color)) + U"\n";
	s += U"\t\t}\n";
	s += U"\t],\n";
	s += U"\t\"actions\": []\n";
	s += U"}\n";

	TextWriter tw(path);
	tw << s;
	tw.close();
}

void Procon30::VirtualServer::negativePercent(int32 percent, int32 fieldType)
{
	bool isNegativeBorad = false;
	if (percent > 50) {
		isNegativeBorad = true;
		percent = 100 - percent;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				field.m_board[i][j].score *= -1;
			}
		}
	}
	int32 posX = 0, posY = 0;
	if (fieldType == 0) {
		int32 cnt = (((height * width) * percent) / 100) / 4;
		while (cnt > 0) {
			posX = Random(0, (width - 1) / 2);
			posY = Random(0, (height - 1) / 2);
			if (isNegativeBorad == false && field.m_board[posY][posX].score < 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score > 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}
	else if (fieldType == 1) {
		int32 cnt = (((height * width) * percent) / 100) / 2;
		while (cnt > 0) {
			posX = Random(0, (width - 1) / 2);
			posY = Random(0, height - 1);
			if (isNegativeBorad == false && field.m_board[posY][posX].score < 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score > 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}
	else if (fieldType == 2) {
		int32 cnt = (((height * width) * percent) / 100) / 2;
		while (cnt > 0) {
			posX = Random(0, width - 1);
			posY = Random(0, (height - 1) / 2);
			if (isNegativeBorad == false && field.m_board[posY][posX].score < 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
			if (isNegativeBorad == true && field.m_board[posY][posX].score > 0) {
				field.m_board[posY][posX].score *= -1;
				cnt--;
			}
		}
	}

}

void Procon30::VirtualServer::putPoint(int32 fieldType)
{
	field.boardSize.y = height;
	field.boardSize.x = width;
	if (fieldType == 0) {
		//上下左右対称
		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (field.boardSize.x + 1) / 2; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		negativePercent(20, 0);

		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[i][field.boardSize.x - j - 1] = field.m_board[i][j];
				field.m_board[field.boardSize.y - i - 1][j] = field.m_board[i][j];
				field.m_board[field.boardSize.y - i - 1][field.boardSize.x - j - 1] = field.m_board[i][j];
			}
		}
	}
	else if (fieldType == 1) {
		//左右対称
		for (int i = 0; i < field.boardSize.y; i++) {
			for (int j = 0; j < (field.boardSize.x + 1) / 2; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		negativePercent(20, 0);

		for (int i = 0; i < field.boardSize.y; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[i][field.boardSize.x - j - 1] = field.m_board[i][j];
			}
		}
	}
	else if (fieldType == 2) {
		//上下対称
		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < field.boardSize.x; j++) {
				field.m_board[i][j].score = abs(Random(-16, 16));
				field.m_board[i][j].exist = true;
			}
		}

		negativePercent(20, 2);

		for (int i = 0; i < (field.boardSize.y + 1) / 2; i++) {
			for (int j = 0; j < (width + 1) / 2; j++) {
				field.m_board[field.boardSize.y - i - 1][j] = field.m_board[i][j];
			}
		}
	}


	/*points.resize(height);
	for (int i = 0; i < height; i++) {
		points[i].resize(width);
	}
	for (int i = 0; i < (height + 1) / 2; i++) {
		for (int j = 0; j < (width + 1) / 2; j++) {
			points[i][j] = Random(-20, 20);
		}
	}
	for (int i = 0; i < (height + 1) / 2; i++) {
		for (int j = 0; j < (width + 1) / 2; j++) {
			points[i][width - j - 1] = points[i][j];
			points[height - i - 1][j] = points[i][j];
			points[height - i - 1][width - j - 1] = points[i][j];
		}
	}*/
	return;
}

void Procon30::VirtualServer::putAgent()
{
	//エージェントの数
	teams.first.agentNum = agent_count;
	teams.second.agentNum = agent_count;
	teams.first.agents.resize(teams.first.agentNum);
	teams.second.agents.resize(teams.second.agentNum);
	//チームidの設定
	teams.first.teamID = 1;
	teams.second.teamID = 11;
	//x,yを-1に設定
	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].nowPosition.x = -1;
		teams.first.agents[i].nowPosition.y = -1;
		teams.second.agents[i].nowPosition.x = -1;
		teams.second.agents[i].nowPosition.y = -1;
	}
	//エージェントidの設定
	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].agentID = i + 2;
		teams.second.agents[i].agentID = i + 12;
	}
	//tileの設定
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			tile[i][j] = 0;
		}
	}

	for (int i = 0; i < teams.first.agentNum; i++) {
		teams.first.agents[i].nowPosition.y = Random(0, height - 1);
		teams.second.agents[i].nowPosition.y = teams.first.agents[i].nowPosition.y;
	}
	for (int i = 0; i < teams.first.agentNum; i++) {
		bool isLoop = true;
		while (isLoop) {
			isLoop = false;
			teams.first.agents[i].nowPosition.x = Random(0, (width - 1) / 2);
			for (int j = 0; j < teams.first.agentNum; j++) {
				if (i != j) {
					if (teams.first.agents[i].nowPosition.x == teams.first.agents[j].nowPosition.x && teams.first.agents[i].nowPosition.y == teams.first.agents[j].nowPosition.y) {
						isLoop = true;
					}
				}
			}
		}
		teams.second.agents[i].nowPosition.x = width - teams.first.agents[i].nowPosition.x - 1;
		tile[teams.first.agents[i].nowPosition.y][teams.first.agents[i].nowPosition.x] = teams.first.teamID;
		tile[teams.second.agents[i].nowPosition.y][teams.second.agents[i].nowPosition.x] = teams.second.teamID;
		//	tiles[agents1[i][2]][agents1[i][1]] = agents1[i][0];
	//	tiles[agents2[i][2]][agents2[i][1]] = agents2[i][0];
	}
	//agents1.resize(agent_count);
	//agents2.resize(agent_count);
	///*agentsの0番目がチームid
	//1番目がx座標2番目がy座標
	//3番目がエージェントid
	//*/
	//for (int i = 0; i < agent_count; i++) {
	//	agents1[i].resize(4);
	//	agents2[i].resize(4);
	//	agents1[i][0] = 1;
	//	agents2[i][0] = 11;
	//	agents1[i][1] = -1;
	//	agents1[i][2] = -1;
	//	agents2[i][1] = -1;
	//	agents2[i][2] = -1;
	//	agents1[i][3] = 2 + i;
	//	agents2[i][3] = 12 + i;
	//}
	//tiles.resize(height);
	//for (int i = 0; i < height; i++) {
	//	tiles[i].resize(width);
	//}
	///*for (int i = 0; i < height;i++) {
	//	for (int j = 0; j < width;j++) {
	//		tiles[i][j] = 0;
	//	}
	//}*/
	//for (int i = 0; i < agent_count; i++) {
	//	agents1[i][2] = Random(0, height - 1);
	//	agents2[i][2] = agents1[i][2];
	//}
	//for (int i = 0; i < agent_count; i++) {
	//	bool isLoop = false;
	//	while (isLoop == false) {
	//		isLoop = false;
	//		agents1[i][1] = Random(0, (width - 1) / 2);
	//		for (int j = 0; j < agent_count; j++) {
	//			if (i != j) {
	//				if (agents1[i][2] != agents1[j][2] && agents1[i][1] != agents1[j][1]) {
	//					isLoop = true;
	//				}
	//			}
	//		}
	//	}
	//	agents2[i][1] = width - agents1[i][1] - 1;
	//	tiles[agents1[i][2]][agents1[i][1]] = agents1[i][0];
	//	tiles[agents2[i][2]][agents2[i][1]] = agents2[i][0];
	//}
	return;
}

int32 Procon30::VirtualServer::calculateScore(TeamColor color)
{
	Array<Array<bool>> visit(MaxFieldY, Array<bool>(MaxFieldX, false));

	Array<Point> q;

	int fieldSizeX = field.boardSize.x;
	int fieldSizeY = field.boardSize.y;

	assert(fieldSizeX > 0);
	assert(fieldSizeY > 0);

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

			if (field.m_board.at(now.y, now.x).color != color) {

				q.push_back({ now.x - 1	,now.y });
				q.push_back({ now.x		,now.y + 1 });
				q.push_back({ now.x + 1 ,now.y });
				q.push_back({ now.x		,now.y - 1 });

			}

		}

	}

	int32 result = 0;

	for (auto y : step(fieldSizeY)) {
		for (auto x : step(fieldSizeX)) {
			if (color != field.m_board.at(y, x).color && visit.at(y).at(x) == false) {
				result += abs(field.m_board.at(y, x).score);
			}
			if (field.m_board.at(y, x).color == color) {
				result += field.m_board.at(y, x).score;
			}
		}
	}

	return result;
}
