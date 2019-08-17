#include <ctime>
#include "HTTPCommunication.hpp"

void Procon30::HTTPCommunication::Form::draw() const
{

	const std::time_t t = std::time(0);
	infoFont(U"now time:{}"_fmt(t)).draw(Vec2(1200.0, 900.0), Palette::Red);

	for (size_t i = 0; i < this->schools.size(); i++) {
		auto& school = schools[i];

		//Infos

		if (t - (int64)school.startedAtUnixTime < 0) {
			//開始前

			const int sec = abs(t - (int64)school.startedAtUnixTime) % 60;
			const int min = (abs(t - (int64)school.startedAtUnixTime) / 60) % 60;
			const int hour = (abs(t - (int64)school.startedAtUnixTime) / (60 * 60)) % 24;
			const int64 day = (abs(t - (int64)school.startedAtUnixTime) / ((int64)60 * 60 * 24));

			infoFont(U"teamID:{},turns:{},id:{}\nstartedAtUnixTime:{},height:{},width:{}"_fmt(school.teamID, school.turns, school.id,
				school.startedAtUnixTime, school.height, school.width)).draw(400.0, i * 200.0 + 100);
			infoFont(U"開始前：開始まで{}日{}時間{}分{}秒"_fmt(day, hour, min, sec)).draw(400.0, i * 200.0 + 200);
		}
		else {
			//開始後

			const int sec = abs(t - (int64)school.startedAtUnixTime) % 60;
			const int min = (abs(t - (int64)school.startedAtUnixTime) / 60) % 60;
			const int hour = (abs(t - (int64)school.startedAtUnixTime) / (60 * 60)) % 24;
			const int64 day = (abs(t - (int64)school.startedAtUnixTime) / ((int64)60 * 60 * 24));

			infoFont(U"teamID:{},turns:{},id:{}\nstartedAtUnixTime:{},height:{},width:{}"_fmt(school.teamID, school.turns, school.id,
				school.startedAtUnixTime, school.height, school.width)).draw(400.0, i * 200.0 + 100);
			infoFont(U"開始後：開始してから{}日{}時間{}分{}秒"_fmt(day, hour, min, sec)).draw(400.0, i * 200.0 + 200);
		}

	}

}

bool Procon30::HTTPCommunication::Form::update()
{

	bool canStart = false;

	for (size_t i = 0; i < this->schools.size(); i++) {
		auto& school = schools[i];

		//Selector
		SimpleGUI::CheckBox(school.checked, school.matchTo, Vec2(100, i * 200 + 100));

		if (school.checked == true) {
			canStart = true;
		}

	}

	//Start Button
	if (SimpleGUI::Button(U"Start", Vec2(1600, 900),Optional<double>(150),canStart))
	{
		return false;
	}

	return true;
}

Array<Procon30::HTTPCommunication::Form::school> Procon30::HTTPCommunication::initilizeFormLoop()
{

	Form form;

	form.infoFont = Font(30);

	this->getAllMatchesInfomation();
	while (System::Update()) {
		if (checkResult())break;
	}


	{
		//これは通ることを想定しています。
		JSONReader jsonReader(U"json/AllMatchesInfo.json");

		for (const auto& itr : jsonReader.arrayView()) {
			form.schools.push_back({});
			form.schools.back().matchTo = itr[U"matchTo"].getString();
			form.schools.back().turns = itr[U"turns"].get<int32>();
			form.schools.back().teamID = itr[U"teamID"].get<int32>();
			form.schools.back().id = itr[U"id"].get<int32>();
			form.schools.back().intervalMillis = itr[U"intervalMillis"].get<int32>();
			form.schools.back().turnMillis = itr[U"turnMillis"].get<int32>();
		}

	}

	//assert(comData.gotMatchInfomationNum == form.schools.size());
	CURL* TempHandle;
	TempHandle = curl_easy_init();
	curl_easy_setopt(TempHandle, CURLOPT_HTTPHEADER, otherList);
	curl_easy_setopt(TempHandle, CURLOPT_HEADER, 1L);
	curl_easy_setopt(TempHandle, CURLOPT_WRITEFUNCTION, callbackWrite);
	curl_easy_setopt(TempHandle, CURLOPT_WRITEDATA, &receiveRawData);
	for (size_t i = 0; i < form.schools.size(); i++) {
		curl_easy_setopt(TempHandle, CURLOPT_URL, Format(U"http://", comData.host, U"/matches/", form.schools[i].id).narrow().c_str());
		comData.nowConnecting = true;
		comData.connectionType = ConnectionType::MatchInfomation;
		comData.connectionMatchNumber = i;

		future = std::async(std::launch::async, [&]() {
			return curl_easy_perform(TempHandle);
			});
		while (System::Update()) {
			if (checkResult())break;
		}

		JSONReader jsonReader;

		//これは通ることを想定しています。
		jsonReader.open(comData.receiveJsonPath);

		if (!jsonReader) {
			assert("Error : Can't open field json file in form");
		}
		else {
			switch (comData.connectionCode)
			{
			case ConnectionStatusCode::TooEarly:
				//おそらくTooEarlyでアクセスするため
				form.schools[i].startedAtUnixTime = jsonReader[U"startAtUnixTime"].get<uint64>();
				break;
			default:
				//TooEarlyではないので
				form.schools[i].startedAtUnixTime = jsonReader[U"startedAtUnixTime"].get<uint64>();
				form.schools[i].width = jsonReader[U"width"].get<int32>();
				form.schools[i].height = jsonReader[U"height"].get<int32>();
				form.schools[i].turn = jsonReader[U"turn"].get<int32>();
				break;
			}
		}
	}
	curl_easy_cleanup(TempHandle);
	bool loop = true;

	while (System::Update() && loop) {
		loop = form.update();
		form.draw();
	}

	{
		Array<int> arr;
		for (size_t i = 0; i < form.schools.size(); i++) {
			if (form.schools[i].checked)
				arr << form.schools[i].id;

		}
		setConversionTable(arr);
	}
	initilizeAllMatchHandles();
	comData.gotMatchInfomationNum = 0;
	isFormLoop = false;
	return form.schools;
}