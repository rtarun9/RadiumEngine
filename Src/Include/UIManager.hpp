#include "Pch.hpp"

#include "imgui.h"

namespace rad
{
	class UIManager
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

		void FrameBegin();

		void Render();

		void Close();

		// From here : https://github.com/ocornut/imgui/issues/707	
		void SetCustomDarkTheme();

	public:
		bool m_DisplayUI{true};
	};
}

