
# include "KosenProcon30.hpp"
# include "GUI.hpp"
# include "Game.hpp"
# include "VirtualServer.hpp"

void Main()
{
	if (Procon30::virtualServerMode) {
		//VirtualServerMode

		INIData data;
		data.load(U"json/VirtualServer/config.ini");

		int32 battleNum = ParseInt<int32>(data.getGlobalVaue(U"BattleNum"));

		for (int i = 0; i < battleNum; i++) {
			Procon30::VirtualServer::VirtualServerMain();
		}
		return;
	}

	Window::Resize(Procon30::WindowSize);
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
	Scene::SetScaleMode(ScaleMode::AspectFit);
	Window::SetStyle(WindowStyle::Sizable);
	//(Debug)
	//VirtualServer

	//incetance genarate order
	//1st GUI
	//- Observer instance
	//2nd HTTPCommunication
	//- SendBuffer instance
	//- Observer shared_ptr Substitution
	//3rd Game
	//- Observer shared_ptr Substitution
	//- SendBuffer shared_ptr Substitution
	
	//thread
	//1:GUI - main,Observer
	//2:HTTPCommunication　-sendBuffer
	//3:Game0
	//4:Game1
	//5:Game2
	//6:VirtualServer
	std::shared_ptr<std::atomic<bool>> ProgramEnd(new std::atomic<bool>);
	ProgramEnd->store(false);

	Procon30::GUI gui;

	Procon30::HTTPCommunication http;


	http.observer = gui.getObserver();
	http.programEnd = ProgramEnd;

	auto schools = http.initilizeFormLoop();

	std::array<Procon30::Game,Procon30::MaxGameNumber> games;

	for (size_t i = 0; i < http.getMatchNum(); i++) {
		games[i].observer = gui.getObserver();
		games[i].buffer = http.getBufferPtr();
		games[i].gameNum = i;
		games[i].gameID = schools[http.getGameIDfromGameNum(i)].id;
		games[i].startedAtUnixTime = schools[http.getGameIDfromGameNum(i)].startedAtUnixTime;
		games[i].matchTo = schools[http.getGameIDfromGameNum(i)].matchTo;
		games[i].MaxTurn = schools[http.getGameIDfromGameNum(i)].turns;
		games[i].turnMillis = schools[http.getGameIDfromGameNum(i)].turnMillis;
		games[i].intervalMillis = schools[http.getGameIDfromGameNum(i)].intervalMillis;
		games[i].teams.first.teamID = schools[http.getGameIDfromGameNum(i)].teamID;
		games[i].programEnd = ProgramEnd;
		http.baseUnixTime = Min(http.baseUnixTime, schools[http.getGameIDfromGameNum(i)].startedAtUnixTime);
		http.baseIntervalMillis = Min(http.baseIntervalMillis, static_cast<uint64>(schools[http.getGameIDfromGameNum(i)].intervalMillis));
		http.baseTurnMillis = Min(http.baseTurnMillis, static_cast<uint64>(schools[http.getGameIDfromGameNum(i)].turnMillis));
	}
	http.baseUnixTime *= (uint64)1000;
	//games[0].parseJson(U"example.json");
	Scene::SetBackground(Color(128));

	//これがここでいいのかわかんないです
	

	http.ThreadRun();
	for (size_t i = 0; i < http.getMatchNum(); i++) {
		games[i].ThreadRun();
	}
	gui.dataUpdate();
	while (System::Update())
	{
		
		gui.draw();

		Circle(Cursor::Pos(), 30).draw(ColorF(1, 0, 0, 0.5));
		if (ProgramEnd->load()) {
			break;
		}
	}

	//std::terminateが出ます。許して
	
	ProgramEnd->store(true);
	return;
}




