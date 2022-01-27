#include "Pch.hpp"

#include "imgui.h"

class UIManager
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
	
	void FrameBegin();

	void Render();

	void Close();

private:
	// NOTE : Custom DearImGui style taken from https://github.com/GraphicsProgramming/dear-imgui-styles.
	// Original style : https://github.com/simongeilfus/Cinder-ImGui
	void CustomCinderStyle();
};