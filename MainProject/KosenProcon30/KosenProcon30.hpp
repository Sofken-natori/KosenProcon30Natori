#pragma once

#include <Siv3D.hpp>
#include <memory>
#include <thread>
#include <array>
#include <utility>
#include <deque>

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

	
}