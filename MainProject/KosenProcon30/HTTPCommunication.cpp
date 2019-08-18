#include "HTTPCommunication.hpp"
#include "Observer.hpp"
#include "Game.hpp"

std::mutex Procon30::HTTPCommunication::receiveRawMtx;
String Procon30::HTTPCommunication::receiveRawData;

Procon30::CommunicationState Procon30::HTTPCommunication::getState() const
{
	if (!this->future.valid())
		return CommunicationState::Null;
	if (this->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		return CommunicationState::Done;
	return CommunicationState::Connecting;
}

Procon30::ConnectionStatusCode Procon30::HTTPCommunication::getResult()
{
	assert(this->future.valid());
	if (this->future.valid()) {
		CURLcode result = this->future.get();
		uint32 code;
		if (result != CURLE_OK) {
			Logger << U"curl_easy_perform() failed.\nUnixTime(Milli):"
				<< Time::GetMillisecSinceEpoch();
			Print << U"curl_easy_perform() failed.";
			return ConnectionStatusCode::ConnectionLost;
		}
		{
			std::lock_guard<std::mutex> lock(receiveRawMtx);
			size_t ofset = Min(receiveRawData.indexOf('['), receiveRawData.indexOf('{'));
			code = Parse<uint32>(receiveRawData.substr(9, 3));
			TextWriter writer(jsonBuffer);
			writer.clear();
			writer << receiveRawData.substr(ofset);
			writer.close();
			receiveRawData.clear();
		}
		if (code != 200 && code != 201) {
			Logger << U"Status Error:" << code;
			if (code == 401) {
				return ConnectionStatusCode::InvaildToken;
			}
			JSONReader reader(jsonBuffer);
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
			return ConnectionStatusCode::UnknownError;
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
		stream->append(Unicode::FromUTF8(buf));
	}
	return dataLength;
}

String Procon30::HTTPCommunication::getPostData(const FilePath& filePath) {
	TextReader reader(filePath);
	if (!reader) {
		assert("send_json_file_openError");
		exit(1);
	}
	String send = reader.readAll();
	reader.close();
	return send;
}

bool Procon30::HTTPCommunication::checkResult()
{

	JSONReader jsonReader;
	int32 thisTurn;
	if (comData.nowConnecting) {
		switch (getState())
		{
		case CommunicationState::Done:
			comData.nowConnecting = false;
			if (!FileSystem::Exists(Format(U"json/", comData.gotMatchInfomationNum, U"/"))) {
				FileSystem::CreateDirectories(Format(U"json/", comData.gotMatchInfomationNum, U"/"));
			}
			comData.connectionCode = getResult();
			switch (comData.connectionCode)
			{
			case Procon30::ConnectionStatusCode::OK:
				switch (comData.connectionType)
				{
				case Procon30::ConnectionType::Ping:
					comData.receiveJsonPath = U"json/ping.json";
					FileSystem::Copy(jsonBuffer, comData.receiveJsonPath, CopyOption::OverwriteExisting);
					Print << U"PingTest:OK";
					break;
				case Procon30::ConnectionType::AllMatchesInfo:
					comData.receiveJsonPath = U"json/AllMatchesInfo.json";
					FileSystem::Copy(jsonBuffer, comData.receiveJsonPath, CopyOption::OverwriteExisting);
					Print << U"gotAllInfo";
					break;
				case Procon30::ConnectionType::MatchInfomation:
					jsonReader.open(jsonBuffer);
					thisTurn = jsonReader[U"turn"].get<int32>();
					comData.receiveJsonPath = Format(U"json/", comData.gotMatchInfomationNum, U"/field_", comData.gotMatchInfomationNum, U"_", thisTurn, U".json");
					jsonReader.close();
					FileSystem::Copy(jsonBuffer, comData.receiveJsonPath, CopyOption::OverwriteExisting);
					FileSystem::Copy(jsonBuffer, Format(U"json/",comData.gotMatchInfomationNum, U"/nowfield.json"), CopyOption::OverwriteExisting);
					Print << U"gotMatchInfoof:" << comData.gotMatchInfomationNum;
					if (thisTurn > comData.gotMatchInfoOfTurn[comData.gotMatchInfomationNum] && !isFormLoop) {
						comData.gotMatchInfoOfTurn[comData.gotMatchInfomationNum] = thisTurn;
						comData.gotMatchInfomationNum++;
					}
					else if (isFormLoop) {
						comData.gotMatchInfomationNum++;
					}
					break;
				case Procon30::ConnectionType::PostAction:
					jsonReader.open(jsonBuffer);
					comData.receiveJsonPath = Format(U"json/", comData.connectionMatchNumber, U"/postReceive_", comData.connectionMatchNumber, U"_", jsonReader[U"turn"].get<int32>(), U".json");
					jsonReader.close();
					FileSystem::Copy(jsonBuffer, comData.receiveJsonPath, CopyOption::OverwriteExisting);
					FileSystem::Copy(jsonBuffer, Format(U"json/", comData.connectionMatchNumber, U"/nowField.json"), CopyOption::OverwriteExisting);
					Print << U"postActionof:" << comData.connectionMatchNumber;
					break;
				case Procon30::ConnectionType::Null:
					break;
				}
				break;
			case Procon30::ConnectionStatusCode::TooEarly:
				jsonReader.open(jsonBuffer);
				comData.receiveJsonPath = Format(U"json/", comData.gotMatchInfomationNum, U"/field_", comData.gotMatchInfomationNum, U"_TE.json");
				jsonReader.close();
				FileSystem::Copy(jsonBuffer, comData.receiveJsonPath, CopyOption::OverwriteExisting);
				Print << U"gotMatchInfoof:" << comData.gotMatchInfomationNum << U"::TooEarly";
				if (isFormLoop) {
					comData.gotMatchInfomationNum++;
				}

				break;
			case Procon30::ConnectionStatusCode::UnacceptableTime:
				break;
			case Procon30::ConnectionStatusCode::InvailedMatches:
				break;
			case Procon30::ConnectionStatusCode::InvaildToken:
				break;
			case Procon30::ConnectionStatusCode::ConnectionLost:
				break;
			case Procon30::ConnectionStatusCode::UnknownError:
				break;
			case Procon30::ConnectionStatusCode::Null:
				break;
			}


			return true;
			break;
		case CommunicationState::Null:

			break;
		case CommunicationState::Connecting:

			break;
		}
	}
	return false;
}

void Procon30::HTTPCommunication::setConversionTable(const Array<int>& arr)
{
	comData.matchNum = arr.size();
	for (int32 i = 0; i < comData.matchNum; i++) {
		comData.matchesConversionTable[i] = arr[i];
	}
}

void Procon30::HTTPCommunication::initilizeAllMatchHandles()
{
	getMatchHandles.resize(comData.matchNum);
	postActionHandles.resize(comData.matchNum);
	for (int32 i = 0; i < comData.matchNum; i++) {
		getMatchHandles[i] = curl_easy_init();
		curl_easy_setopt(getMatchHandles[i], CURLOPT_URL, Format(U"http://", comData.host, U"/matches/", comData.matchesConversionTable[i]).narrow().c_str());
		curl_easy_setopt(getMatchHandles[i], CURLOPT_HTTPHEADER, otherList);
		curl_easy_setopt(getMatchHandles[i], CURLOPT_HEADER, 1L);
		curl_easy_setopt(getMatchHandles[i], CURLOPT_WRITEFUNCTION, callbackWrite);
		curl_easy_setopt(getMatchHandles[i], CURLOPT_WRITEDATA, &receiveRawData);

		postActionHandles[i] = curl_easy_init();
		curl_easy_setopt(postActionHandles[i], CURLOPT_URL, Format(U"http://", comData.host, U"/matches/", comData.matchesConversionTable[i], U"/action").narrow().c_str());
		curl_easy_setopt(postActionHandles[i], CURLOPT_HTTPHEADER, postList);
		curl_easy_setopt(postActionHandles[i], CURLOPT_HEADER, 1L);
		curl_easy_setopt(postActionHandles[i], CURLOPT_POST, 1L);
		curl_easy_setopt(postActionHandles[i], CURLOPT_WRITEFUNCTION, callbackWrite);
		curl_easy_setopt(postActionHandles[i], CURLOPT_WRITEDATA, &receiveRawData);
	}
}

void Procon30::HTTPCommunication::update()
{
	bool gotResult = checkResult();
	//postˆ—‚Í‰½‚æ‚è‚à—Dæ‚³‚ê‚é‚×‚«
	bool postNow = checkPostAction();
	if (comData.gotMatchInfomationNum == comData.matchNum) {
		Procon30::Game::HTTPReceived();
		comData.gotMatchInfomationNum = 0;
	}
	if (comData.gotMatchInfomationNum != 0) {
		getMatchInfomation();
	}
	//ŽžŠÔ‚É‡‚í‚¹‚ÄŽæ“¾ˆ—‚ð‘‚«‚Ü‚·‚ª...(¡‚Í‚©‚¯‚È‚¢(ŒöŽ®‚Ì‰ñ“š‘Ò‚¿))

	if (gotResult || postNow) {
		observer->notify(*this);
	}
}

void Procon30::HTTPCommunication::Loop()
{
	while (comData.gotMatchInfomationNum != comData.matchNum) {
		getMatchInfomation();
		while (true)
		{
			if (checkResult())break;
		}
		std::this_thread::sleep_for(500ms);
	}
	Procon30::Game::HTTPReceived();
	observer->notify(*this);
	while (true) {
		update();
		if (ProglamEnd.load() == true)
			break;
	}
	Logger << U"HTTPCommunication Thread End";
	return;
}

void Procon30::HTTPCommunication::ThreadRun()
{
	observer->notify(*this);
	std::thread th(&HTTPCommunication::Loop, this);
	this->thisThread = std::move(th);
	return;
}

bool Procon30::HTTPCommunication::pingServerConnectionTest()
{
	if (comData.nowConnecting) return false;
	comData.nowConnecting = true;
	comData.connectionType = ConnectionType::Ping;
	future = std::async(std::launch::async, [&]() {
		return curl_easy_perform(pingHandle);
		});
	return true;
}

bool Procon30::HTTPCommunication::getAllMatchesInfomation()
{
	if (comData.nowConnecting) return false;
	comData.nowConnecting = true;
	comData.connectionType = ConnectionType::AllMatchesInfo;
	future = std::async(std::launch::async, [&]() {
		return curl_easy_perform(matchesInfoHandle);
		});
	return true;
}

bool Procon30::HTTPCommunication::getMatchInfomation()
{
	if (comData.nowConnecting) return false;
	if (comData.gotMatchInfomationNum >= comData.matchNum && !isFormLoop) return false;
	comData.nowConnecting = true;
	comData.connectionType = ConnectionType::MatchInfomation;
	comData.connectionMatchNumber = comData.gotMatchInfomationNum;

	future = std::async(std::launch::async, [&]() {
		return curl_easy_perform(getMatchHandles[comData.connectionMatchNumber]);
		});
	return true;
}

bool Procon30::HTTPCommunication::checkPostAction()
{
	if (comData.nowConnecting) return false;
	if (buffer->size() == 0) return false;
	comData.nowConnecting = true;
	comData.connectionType = ConnectionType::PostAction;
	FilePath path = buffer->getPath();
	auto splittedPath = path.split('_');
	int32 gameNum = Parse<int32>(splittedPath[1]);
	comData.connectionMatchNumber = gameNum;
	String send = getPostData(path);
	tempPostString = send.toUTF8();
	curl_easy_setopt(postActionHandles[comData.connectionMatchNumber], CURLOPT_POSTFIELDSIZE, (long)tempPostString.size());
	curl_easy_setopt(postActionHandles[comData.connectionMatchNumber], CURLOPT_POSTFIELDS, tempPostString.c_str());
	future = std::async(std::launch::async, [&]() {
		return curl_easy_perform(postActionHandles[comData.connectionMatchNumber]);
		});
	return true;
}

Procon30::HTTPCommunication::HTTPCommunication()
	:buffer(new SendBuffer())
{
	comData.connectionMatchNumber = -1;
	comData.connectionType = ConnectionType::Null;
	comData.gotMatchInfomationNum = 0;
	comData.nowConnecting = false;
	isFormLoop = true;
	{
		JSONReader reader(U"AccessInfo.json");
		comData.token = reader[U"Token"].getString();
		comData.host = reader[U"Host"].getString();
	}
	//comData.host = U"127.0.0.1:8081";
	//comData.token = U"procon30_example_token";
	for (int32& i : comData.gotMatchInfoOfTurn) {
		i = -1;
	}


	//Setting Header
	postList = NULL;
	otherList = NULL;
	otherList = curl_slist_append(otherList, (U"Authorization:" + comData.token).narrow().c_str());
	postList = curl_slist_append(postList, (U"Authorization:" + comData.token).narrow().c_str());
	postList = curl_slist_append(postList, "Content-Type: application/json");

	//Handle-initilize
	pingHandle = curl_easy_init();
	curl_easy_setopt(pingHandle, CURLOPT_URL, (U"http://" + comData.host + U"/ping").narrow().c_str());
	curl_easy_setopt(pingHandle, CURLOPT_HTTPHEADER, otherList);
	curl_easy_setopt(pingHandle, CURLOPT_HEADER, 1L);
	curl_easy_setopt(pingHandle, CURLOPT_WRITEFUNCTION, callbackWrite);
	curl_easy_setopt(pingHandle, CURLOPT_WRITEDATA, &receiveRawData);

	matchesInfoHandle = curl_easy_init();
	curl_easy_setopt(matchesInfoHandle, CURLOPT_URL, (U"http://" + comData.host + U"/matches").narrow().c_str());
	curl_easy_setopt(matchesInfoHandle, CURLOPT_HTTPHEADER, otherList);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_HEADER, 1L);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_WRITEFUNCTION, callbackWrite);
	curl_easy_setopt(matchesInfoHandle, CURLOPT_WRITEDATA, &receiveRawData);
}

Procon30::HTTPCommunication::~HTTPCommunication()
{
	if (thisThread.joinable()) {
		//thisThread.detach();
	}
}

size_t Procon30::HTTPCommunication::getMatchNum() const
{
	return this->comData.matchNum;
}

Procon30::CommunicationData Procon30::HTTPCommunication::getComData() const
{
	return comData;
}

std::shared_ptr<Procon30::SendBuffer> Procon30::HTTPCommunication::getBufferPtr()
{
	return buffer;
}

int32 Procon30::HTTPCommunication::getGameIDfromGameNum(const int32& num)
{
	if (num < 0 || comData.matchesConversionTable.count(num) == 0)return -1;
	return (*comData.matchesConversionTable.find(num)).first;
}

Procon30::HTTPCommunication& Procon30::HTTPCommunication::operator=(const Procon30::HTTPCommunication& right)
{
	this->comData = right.comData;
	return (*this);
}