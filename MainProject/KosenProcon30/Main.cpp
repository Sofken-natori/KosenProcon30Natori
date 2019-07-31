
# include "KosenProcon30.hpp"
# include "GUI.hpp"
# include "Game.hpp"
# include "VirtualServer.hpp"

//TestCode
size_t callbackWrite(char* ptr, size_t size, size_t nmemb, std::string* stream) {
	size_t dataLength = size * nmemb;
	stream->append(ptr, dataLength);
	return dataLength;
}

void Main()
{
	Window::Resize(Procon30::WindowSize);
	Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
	//(Debug)
	//VirtualServer

	//feature
	//Initilize Form

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

	Procon30::GUI gui;

	Procon30::HTTPCommunication http;

	http.initilizeFormLoop();

	std::array<Procon30::Game,Procon30::MaxGameNumber> games;

	games[0].parseJson(U"example.json");
	Scene::SetBackground(Color(128));

	while (System::Update())
	{
		gui.draw();

		Circle(Cursor::Pos(), 60).draw(ColorF(1, 0, 0, 0.5));
	}
}
