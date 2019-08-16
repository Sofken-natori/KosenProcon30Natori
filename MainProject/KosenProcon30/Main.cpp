
# include "KosenProcon30.hpp"
# include "GUI.hpp"
# include "Game.hpp"
# include "VirtualServer.hpp"

void Main()
{
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

	Procon30::ProglamEnd.store(false);

	Procon30::GUI gui;

	Procon30::HTTPCommunication http;

	http.observer = gui.getObserver();

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
	}

	//games[0].parseJson(U"example.json");
	Scene::SetBackground(Color(128));

	//これがここでいいのかわかんないです
	

	http.ThreadRun();
	for (size_t i = 0; i < http.getMatchNum(); i++) {
		games[i].ThreadRun();
	}
	gui.dataUpdate();
	//あとでthreadGuardにします。
	while (System::Update() || Procon30::ProglamEnd.load())
	{
		
		gui.draw();

		Circle(Cursor::Pos(), 60).draw(ColorF(1, 0, 0, 0.5));
	}

	//std::terminateが出ます。許して
	
	Procon30::ProglamEnd.store(true);
	return;
}




