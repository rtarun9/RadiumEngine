#pragma once

#include "Pch.hpp"

#include "Graphics/API/RenderTarget.hpp"
#include "imgui.h"

namespace rad
{
	class UIManager
	{
	public:
		void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int width, int height);

		void FrameBegin();

		void Render();

		void Close();

		// From here : https://github.com/ocornut/imgui/issues/707	
		void SetCustomDarkTheme();

		void ShowMetrics();

		void DisplayRenderTarget(const std::wstring& rtName, ID3D11ShaderResourceView* srv);


	public:
		static constexpr int IMAGE_DIMENSIONS = 500;

		int m_Width{};
		int m_Height{};

		ImGuiViewport m_Viewport{};

		bool m_DisplayUI{true};
	};
}

