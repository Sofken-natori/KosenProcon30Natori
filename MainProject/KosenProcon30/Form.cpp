#include "HTTPCommunication.hpp"

void Procon30::HTTPCommunication::Form::draw() const
{
}

void Procon30::HTTPCommunication::Form::update()
{
}

void Procon30::HTTPCommunication::initilizeFormLoop()
{

	Form form;

	while (System::Update()) {
		form.update();
		form.draw();
	}

	return;
}