#pragma once
#include "KosenProcon30.hpp"

namespace Procon30 {
	class SendBuffer
	{
	private:
		std::mutex queueMtx;
		std::condition_variable cond;

		std::deque<FilePath> queueBuffer;
		const size_t QueueMaxSize = 5;
	public:

		void pushPath(FilePath path);

		FilePath getPath();

		SendBuffer();
		~SendBuffer();
	};
}
