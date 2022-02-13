#include "Application.hpp"
								
#include "../SandBox/SandBox.hpp"

int WINAPI wWinMain(HINSTANCE instance, [[maybe_unused]] HINSTANCE prevInstance, [[maybe_unused]] LPWSTR commandLine, [[maybe_unused]] INT commandShow)		
{																																							
	std::shared_ptr<rad::SandBox> engine = std::make_shared<rad::SandBox>(L"RadiumEngine", 1920, 1080);
	return rad::Application::Run(engine, instance);																											
}																																							
