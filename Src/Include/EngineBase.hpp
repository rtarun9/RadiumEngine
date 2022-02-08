#pragma once

#include "Pch.hpp"

#include "Camera.hpp"
#include "UIManager.hpp"

#include "Graphics/DepthStencil.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Model.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/InputLayout.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/TextureSampler.hpp"
#include "Graphics/Lights.hpp"
#include "Graphics/RenderTarget.hpp"
#include "Graphics/SkyBox.hpp"

namespace rad
{
	class EngineBase
	{
	public:
		EngineBase(const std::wstring& title, uint32_t width, uint32_t height) : m_Title(title), m_Width(width), m_Height(height) {}
		~EngineBase() = default;

		virtual void OnInit() = 0;
		virtual void OnUpdate(float deltaTime) = 0;
		virtual void OnRender() = 0;
		virtual void OnDestroy() = 0;

		virtual void OnKeyDown(uint32_t keycode) = 0;
		virtual void OnKeyUp(uint32_t keycode)  = 0;

		void LoadContent() {};

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; };
		std::wstring GetTitle() const { return m_Title; };

	private:
		uint32_t m_Width;
		uint32_t m_Height;
		std::wstring m_Title;
	};
}
