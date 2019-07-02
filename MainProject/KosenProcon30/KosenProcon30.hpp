#pragma once

#include <Siv3D.hpp>

namespace Procon30 {

	constexpr int32 MaxFieldX = 20;
	constexpr int32 MaxFieldY = 20;
	constexpr int32 TileSize = 50;
	constexpr int32 SideAreaX = 400;
	constexpr Size WindowSize{ MaxFieldX * TileSize + SideAreaX * 2, MaxFieldY * TileSize };

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