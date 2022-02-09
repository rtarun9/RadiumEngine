#include "Camera.hpp"

#include <imgui.h>

namespace rad
{

	Camera::Camera()
		: m_WorldFront(dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)), m_WorldRight(dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),
		m_WorldUp(dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)), m_CameraFront(m_WorldFront), m_CameraRight(m_WorldRight),
		m_CameraUp(m_WorldUp), m_CameraTarget(dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)), m_CameraPosition(dx::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f)),
		m_Yaw(0.0f), m_Pitch(0.0f), m_ViewMatrix(dx::XMMatrixIdentity()), m_CameraRotationMatrix(dx::XMMatrixIdentity()),
		m_MovementSpeed(1000.0f), m_RotationSpeed(0.05f), m_KeyState{ false }
	{
	}

	void Camera::HandleInput(uint32_t keycode, bool isKeyDown)
	{
		// WARNING : Because we are not checking if keycode exist within keymap, any keycode not beloning in it will cause camera to move forward,
		// To disable this uncomment the code below. It is being left uncommented since this solution is likely not ideal, but can be annoying in some cases.
		if (INPUT_MAP.find(keycode) != INPUT_MAP.end())
		{
			m_KeyState[INPUT_MAP[keycode]] = isKeyDown;
		}
	}


	void Camera::Update(float deltaTime)
	{
		float movementSpeed = deltaTime * m_MovementSpeed / 10.0f;

		if (m_KeyState[Keys::W])
		{
			m_CameraPosition += m_CameraFront * movementSpeed;
		}
		else if (m_KeyState[Keys::S])
		{
			m_CameraPosition -= m_CameraFront * movementSpeed;
		}

		if (m_KeyState[Keys::A])
		{
			m_CameraPosition -= m_CameraRight * movementSpeed;
		}
		else if (m_KeyState[Keys::D])
		{
			m_CameraPosition += m_CameraRight * movementSpeed;
		}

		if (m_KeyState[Keys::AUp])
		{
			m_Pitch -= m_RotationSpeed;
		}
		else if (m_KeyState[Keys::ADown])
		{
			m_Pitch += m_RotationSpeed;
		}

		if (m_KeyState[Keys::ALeft])
		{
			m_Yaw -= m_RotationSpeed;
		}
		else if (m_KeyState[Keys::ARight])
		{
			m_Yaw += m_RotationSpeed;
		}

		m_CameraRotationMatrix = dx::XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);

		m_CameraTarget = dx::XMVector3TransformCoord(m_WorldFront, m_CameraRotationMatrix);
		m_CameraTarget = dx::XMVector3Normalize(m_CameraTarget);

		m_CameraRight = dx::XMVector3TransformCoord(m_WorldRight, m_CameraRotationMatrix);
		m_CameraFront = dx::XMVector3TransformCoord(m_WorldFront, m_CameraRotationMatrix);
		m_CameraUp = dx::XMVector3Cross(m_CameraFront, m_CameraRight);

		m_CameraTarget = m_CameraPosition + m_CameraTarget;

		m_ViewMatrix = dx::XMMatrixLookAtLH(m_CameraPosition, m_CameraTarget, m_CameraUp);
	}

	void Camera::UpdateControls()
	{
		ImGui::Begin("Camera Controls");
		ImGui::SliderFloat("Movement Speed", &m_MovementSpeed, 0.0f, 5000.0f);
		ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, 0.0f, 10.0f);
		ImGui::End();
	}

	dx::XMMATRIX Camera::GetViewMatrix() const
	{
		return m_ViewMatrix;
	}
}
