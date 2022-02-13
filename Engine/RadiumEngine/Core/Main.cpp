#include "Application.hpp"
								
#include "../SandBox/DeferredShading.hpp"

int WINAPI wWinMain(HINSTANCE instance, [[maybe_unused]] HINSTANCE prevInstance, [[maybe_unused]] LPWSTR commandLine, [[maybe_unused]] INT commandShow)		
{																																							
	std::shared_ptr<rad::DeferredShading> engine = std::make_shared<rad::DeferredShading>(L"RadiumEngine", 1920, 1080);
	return rad::Application::Run(engine, instance);																											
}																																							
