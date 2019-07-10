#include "VirtualServer.hpp"

int32 Procon30::VirtualServer::random_number()
{
		return Random(1, 6);
}

void Procon30::VirtualServer::write_json()
{

}

void Procon30::VirtualServer::input_point(Array<Array<int32>> &points)
{
	points.resize(20);
	for (int i = 0; i < 20;i++) {
		points[i].resize(20);
	}
	for (int i = 0; i < 10;i++) {
		for (int j = 0; j < 10;j++) {
			points[i][j] = random_number();
		}
	}
}
