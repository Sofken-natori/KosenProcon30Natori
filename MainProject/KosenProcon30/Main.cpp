
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


	//TestCode
	{
		CURL* curl;
		CURLcode ret;

		curl = curl_easy_init();
		std::string chunk;

		if (curl == NULL) {
			Logger << U"curl_easy_init() failed";
			assert("curl_easy_init() failed");
			return;
		}

		curl_easy_setopt(curl, CURLOPT_URL, "https://kosyukai2019.azurewebsites.net/kosyukai2019.html");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callbackWrite);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
		ret = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (ret != CURLE_OK) {
			Logger << U"curl_easy_perform() failed.";
			assert("curl_easy_perform() failed.");
			return;
		}

		//debugでの表示まあ軽い
		Print << Unicode::Widen(chunk);
	}



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
	//2:HTTPCommunication
	//3:Game0
	//4:Game1
	//5:Game2
	//6:VirtualServer

	Procon30::GUI gui;

	Procon30::HTTPCommunication http;
	std::array<Procon30::Game,Procon30::MaxGameNumber> games;

	games[0].parseJson(U"example.json");

	while (System::Update())
	{
		gui.draw();

		Circle(Cursor::Pos(), 60).draw(ColorF(1, 0, 0, 0.5));
	}
}
