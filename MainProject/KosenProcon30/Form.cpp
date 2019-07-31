#include "HTTPCommunication.hpp"

void Procon30::HTTPCommunication::Form::draw() const
{


}

bool Procon30::HTTPCommunication::Form::update()
{

	for (int i = 0; i < this->schools.size(); i++) {

		//Selector
		auto& school = schools[i];
		SimpleGUI::CheckBox(school.checked, school.name, Vec2(100, i * 200 + 100));

		//Infos
		infoFont(U"teamID:{},turns:{},id:{}\nstartedAtUnixTime:{},height:{},width:{}"_fmt(school.teamID, school.turns, school.id,
			school.startedAtUnixTime, school.height, school.width)).draw(300, i * 200 + 100);
	}

	//Start Button
	if (SimpleGUI::Button(U"Start", Vec2(1700, 900)))
	{
		return false;
	}

	return true;
}

void Procon30::HTTPCommunication::initilizeFormLoop()
{

	Form form;

	form.infoFont = Font(30);


	{
		JSONReader jsonReader(U"json/AllMatchesInfo.json");

		for (const auto& itr : jsonReader.arrayView()) {
			form.schools.push_back({});
			form.schools.back().name = itr[U"matchTo"].getString();
			form.schools.back().turns = itr[U"turns"].get<int32>();
			form.schools.back().teamID = itr[U"teamID"].get<int32>();
			form.schools.back().id = itr[U"id"].get<int32>();

		}

	}

	assert(gotMatchInfomationNum == form.schools.size());

	for (int i = 0; i < gotMatchInfomationNum; i++) {
		JSONReader jsonReader;

		jsonReader.open(U"json/" + Format(i) + U"/field_" + Format(i) + U"_" + Format(0) + U".json");

		if (!jsonReader) {
			assert("Error : Can't open field json file in form");
		}


		form.schools.back().startedAtUnixTime = jsonReader[U"startedAtUnixTime"].get<int32>();
		form.schools.back().width = jsonReader[U"width"].get<int32>();
		form.schools.back().height = jsonReader[U"height"].get<int32>();
		form.schools.back().turn = jsonReader[U"turn"].get<int32>();
	}

	bool loop = true;

	while (System::Update() && loop) {
		loop = form.update();
		form.draw();
		if (loop) {
			Array<int> arr;
			for (int i = 0; i < gotMatchInfomationNum; i++) {
				if (form.schools[i].checked)
					arr << i;
			}
			setConversionTable(arr);
		}
	}

	return;
}