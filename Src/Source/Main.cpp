#include "Pch.hpp"

#include "Application.hpp"
#include "Engine.hpp"

int WINAPI wWinMain(HINSTANCE instance, [[maybe_unused]] HINSTANCE prevInstance, [[maybe_unused]] LPWSTR commandLine, [[maybe_unused]] INT commandShow)
{
	Engine *engine = new Engine(L"Radium Engine", 1080, 720);
	return Application::Run(engine, instance);
}