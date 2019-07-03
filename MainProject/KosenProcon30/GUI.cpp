#include "GUI.hpp"



void Procon30::GUI::draw() {
}

void Procon30::GUI::dataUpdate()
{
}


Procon30::GUI::GUI()
{
	observer = std::shared_ptr<Observer>(new Observer());
}


Procon30::GUI::~GUI()
{
}

std::shared_ptr<Procon30::Observer> Procon30::GUI::getObserver()
{
	return observer;
}
