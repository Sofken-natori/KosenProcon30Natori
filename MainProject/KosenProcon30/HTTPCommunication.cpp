#include "HTTPCommunication.hpp"
#include "Observer.hpp"
#include "Game.hpp"
Procon30::CommunicationState Procon30::HTTPCommunication::getState() const
{
	if (!this->future.valid())
		return CommunicationState::Null;
	if (this->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		return CommunicationState::Done;
	return CommunicationState::Connecting;
}

//Needs:getState() == Done
Procon30::ConnectionStatusCode Procon30::HTTPCommunication::getResult()
{
	assert(this->future.valid());
	if (this->future.valid()) {
		CURLcode result = this->future.get();
		uint32 code;
		if (result != CURLE_OK) {
			Logger << U"curl_easy_perform() failed.\nUnixTime(Milli):"
				<< Time::GetMillisecSinceEpoch();
			return ConnectionStatusCode::ConnectionLost;
		}
		{
			std::lock_guard<std::mutex> lock(receiveRawMtx);
			size_t ofset = Min(receiveRawData.indexOf('['), receiveRawData.indexOf('{'));
			code = Parse<uint32>(receiveRawData.substr(9, 3));
			TextWriter writer(receiveJsonPath);
			writer << receiveRawData.substr(ofset);
			writer.close();
		}
		if (code != 200) {
			Logger << U"Status Error:" << code;
			if (code == 401) {
				return ConnectionStatusCode::InvaildToken;
			}
			JSONReader reader(receiveJsonPath);
			String status = reader[U"status"].get<String>();
			uint64 startAtUnixTime = reader[U"startAtUnixTime"].get<uint64>();
			Logger << U"status:" << status << U"\nstartAtUnixTime" << startAtUnixTime;
			if (U"InvalidMatches" == status) {

				return ConnectionStatusCode::InvailedMatches;
			}
			if (U"TooEarly" == status) {

				return ConnectionStatusCode::TooEarly;
			}
			if (U"UnacceptableTime" == status) {

				return ConnectionStatusCode::UnacceptableTime;
			}
		}
		return ConnectionStatusCode::OK;
	}
	return ConnectionStatusCode::Null;
}

size_t Procon30::HTTPCommunication::callbackWrite(char* ptr, size_t size, size_t nmemb, String* stream) {
	std::string buf;
	size_t dataLength = size * nmemb;
	buf.append(ptr, dataLength);
	{
		std::lock_guard<std::mutex> lock(receiveRawMtx);
		stream->append(Unicode::Widen(buf));
	}
	return dataLength;
}

inline String Procon30::HTTPCommunication::getPostData(const FilePath& filePath) {
	TextReader reader(filePath);
	if (!reader) {
		assert("send_json_file_openError");
		exit(1);
	}
	String send = reader.readAll();
	reader.close();
	return send;
}

void Procon30::HTTPCommunication::pingServerConnectionTest()
{
	nowConnecting = true;
	future = std::async(std::launch::async, [&]() {
		return curl_easy_perform(pingHandle);
	});
}

void Procon30::HTTPCommunication::update()
{
	ConnectionStatusCode CSC;
	if (nowConnecting) {
		switch (getState())
		{
		case CommunicationState::Done:
			switch (CSC)
			{
			case Procon30::ConnectionStatusCode::OK:
				switch (connectionType)
				{
				case Procon30::ConnectionType::Ping:
					Logger << U"PingTest:OK";
					break;
				case Procon30::ConnectionType::AllMatchesInfo:

					Procon30::Game::HTTPReceived();
					break;
				case Procon30::ConnectionType::MatchInfomation:
					break;
				case Procon30::ConnectionType::PostAction:
					break;
				case Procon30::ConnectionType::Null:
					break;
				}
				break;
			case Procon30::ConnectionStatusCode::TooEarly:
				break;
			case Procon30::ConnectionStatusCode::UnacceptableTime:
				break;
			case Procon30::ConnectionStatusCode::InvailedMatches:
				break;
			case Procon30::ConnectionStatusCode::InvaildToken:
				break;
			case Procon30::ConnectionStatusCode::ConnectionLost:
				break;
			case Procon30::ConnectionStatusCode::Null:
				break;
			}

			nowConnecting = false;
			break;
		case CommunicationState::Null:

			break;
		case CommunicationState::Connecting:

			break;
		}
	}
}

void Procon30::HTTPCommunication::jsonDistribute()
{

}

void Procon30::HTTPCommunication::getServer()
{


	
}

void Procon30::HTTPCommunication::postServer()
{
	FilePath path = buffer->getPath();


}

Procon30::HTTPCommunication::HTTPCommunication()
	:buffer(new SendBuffer())
{
	//Setting Header
	postList = NULL;
	otherList = NULL;
	otherList = curl_slist_append(otherList, (U"Authorization:" + token).narrow().c_str());
	postList = curl_slist_append(postList, "Authorization:procon30_example_token");
	postList = curl_slist_append(postList, "Content-Type: application/json");

	//Handle-initilize
	pingHandle = curl_easy_init();
	curl_easy_setopt(pingHandle, CURLOPT_URL, (U"http://" + host + U"/ping").narrow().c_str());
	curl_easy_setopt(pingHandle, CURLOPT_HTTPHEADER, otherList);
	curl_easy_setopt(pingHandle, CURLOPT_HEADER, 1L);
	curl_easy_setopt(pingHandle, CURLOPT_WRITEFUNCTION, callbackWrite);
	curl_easy_setopt(pingHandle, CURLOPT_WRITEDATA, &receiveRawData);

	matchesInfoHandle = curl_easy_init();
	curl_easy_setopt(matchesInfoHandle, CURLOPT_URL, (U"http://" + host + U"/matches").narrow().c_str());
	curl_easy_setopt(matchesInfoHandle, CURLOPT_HTTPHEADER, otherList);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_HEADER, 1L);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_WRITEFUNCTION, callbackWrite);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_WRITEDATA, &receiveRawData);
}

Procon30::HTTPCommunication::~HTTPCommunication()
{
}

//Procon30::InformationID::InformationID()
//{
//}

//inline Procon30::InformationID::InformationID(int32 teamid, Array<int32> agentids) : teamID(teamid), AgentIDs(agentids) {};
