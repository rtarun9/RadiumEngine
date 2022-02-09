#include "Application.hpp"
#include "../SandBox/PBRTest.hpp"

int WINAPI wWinMain(HINSTANCE instance, [[maybe_unused]] HINSTANCE prevInstance, [[maybe_unused]] LPWSTR commandLine, [[maybe_unused]] INT commandShow)
{
	std::shared_ptr<rad::PBRTest> engine = std::make_shared<rad::PBRTest>(L"Radium Engine", 1920, 1080);
	return rad::Application::Run(engine, instance);
}