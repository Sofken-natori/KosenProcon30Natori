
# include "KosenProcon30.hpp"
# include "GUI.hpp"

void Main()
{
	Window::Resize(Procon30::WindowSize);
	Graphics::SetBackground(ColorF(0.8, 0.9, 1.0));

	Procon30::GUI gui;

	while (System::Update())
	{
		gui.draw();

		Circle(Cursor::Pos(), 60).draw(ColorF(1, 0, 0, 0.5));
	}
}
