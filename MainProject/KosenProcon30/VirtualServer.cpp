#include "VirtualServer.hpp"


Procon30::VirtualServer::VirtualServer()
{
	putPoint();
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
				s += Format(points[i][j]);
			}
			else {
				s += Format(points[i][j]) + U",";
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
				s += Format(tiles[i][j]);
			}
			else {
				s += Format(tiles[i][j]) + U",";
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
	for (int i = 0; i < agent_count;i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(agents1[i][3]) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(agents1[i][1]) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(agents1[i][2]) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	int32 sum = 0;
	for (int i = 0; i < agent_count;i++) {
		sum += points[agents1[i][2]][agents1[i][1]];
	}
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": 0\n";
	s += U"\t\t},\n";

	s += U"\t\t{\n";
	s += U"\t\t\t\"teamID\": 11,\n";
	s += U"\t\t\t\"agents\": [\n";
	for (int i = 0; i < agent_count; i++) {
		s += U"\t\t\t\t{\n";
		s += U"\t\t\t\t\t\"agentID\": " + Format(agents2[i][3]) + U",\n";
		s += U"\t\t\t\t\t\"x\": " + Format(agents2[i][1]) + U",\n";
		s += U"\t\t\t\t\t\"y\": " + Format(agents2[i][2]) + U"\n";
		if (i == agent_count - 1) {
			s += U"\t\t\t\t}\n";
		}
		else {
			s += U"\t\t\t\t},\n";
		}
	}
	s += U"\t\t\t],\n";
	s += U"\t\t\t\"tilePoint\": " + Format(sum) + U",\n";
	s += U"\t\t\t\"areaPoint\": 0\n";
	s += U"\t\t}\n";
	s += U"\t],\n";
	s += U"\t\"actions\": []\n";
	s += U"}\n";

	TextWriter tw(path);
	tw << s;
	tw.close();
}

void Procon30::VirtualServer::putPoint()
{
	points.resize(height);
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
	}
	return;
}

void Procon30::VirtualServer::putAgent()
{
	agents1.resize(agent_count);
	agents2.resize(agent_count);
	/*agentsの0番目がチームid
	1番目がx座標2番目がy座標
	3番目がエージェントid
	*/
	for (int i = 0; i < agent_count; i++) {
		agents1[i].resize(4);
		agents2[i].resize(4);
		agents1[i][0] = 1;
		agents2[i][0] = 11;
		agents1[i][1] = -1;
		agents1[i][2] = -1;
		agents2[i][1] = -1;
		agents2[i][2] = -1;
		agents1[i][3] = 2 + i;
		agents2[i][3] = 12 + i;
	}
	tiles.resize(height);
	for (int i = 0; i < height; i++) {
		tiles[i].resize(width);
	}
	/*for (int i = 0; i < height;i++) {
		for (int j = 0; j < width;j++) {
			tiles[i][j] = 0;
		}
	}*/
	for (int i = 0; i < agent_count; i++) {
		agents1[i][2] = Random(0, height - 1);
		agents2[i][2] = agents1[i][2];
	}
	for (int i = 0; i < agent_count; i++) {
		bool isLoop = false;
		while (isLoop == false) {
			isLoop = false;
			agents1[i][1] = Random(0, (width - 1) / 2);
			for (int j = 0; j < agent_count; j++) {
				if (i != j) {
					if (agents1[i][2] != agents1[j][2] && agents1[i][1] != agents1[j][1]) {
						isLoop = true;
					}
				}
			}
		}
		agents2[i][1] = width - agents1[i][1] - 1;
		tiles[agents1[i][2]][agents1[i][1]] = agents1[i][0];
		tiles[agents2[i][2]][agents2[i][1]] = agents2[i][0];
	}
	return;
}
