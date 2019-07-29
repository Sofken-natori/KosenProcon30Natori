#include "SendBuffer.hpp"



void Procon30::SendBuffer::pushPath(FilePath path)
{
	{
		std::unique_lock<std::mutex> lock(queueMtx);
		cond.wait(lock, [this]() {return queueBuffer.size() < QueueMaxSize; });
		queueBuffer.push_back(path);
	}
	cond.notify_all();
}

FilePath Procon30::SendBuffer::getPath()
{
	FilePath ret;
	{
		std::unique_lock<std::mutex> lock(queueMtx);
		cond.wait(lock, [this]() {return queueBuffer.size() > 0; });
		ret = queueBuffer.back();
		queueBuffer.pop_back();
	}
	cond.notify_all();
	return ret;
}

inline const size_t Procon30::SendBuffer::size()
{
	std::lock_guard<std::mutex> lock(queueMtx);
	return queueBuffer.size();
}

Procon30::SendBuffer::SendBuffer()
{
}


Procon30::SendBuffer::~SendBuffer()
{
}
