#include "Camera.hpp"

Camera::Camera()
{
	m_WorldFront = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_WorldRight = dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	m_WorldUp = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_CameraFront = m_WorldFront;
	m_CameraRight = m_WorldRight;
	m_CameraUp = m_WorldUp;

	m_CameraTarget = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	m_CameraPosition = dx::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);

	m_Yaw = 0.0f;
	m_Pitch = 0.0f;

	m_MovementSpeed = 500.0f;

	m_RotationSpeed = 0.05f;

	for (size_t i = 0; i < Keys::TotalKeyCount; i++)
	{
		m_KeyState[i] = false;
	}
}

void Camera::HandleInput(uint32_t keycode, bool isKeyDown)
{
	if (isKeyDown)
	{
		switch (keycode)
		{
		case 'W':
		{
			m_KeyState[Keys::W] = true;
		}break;

		case 'S':
		{
			m_KeyState[Keys::S] = true;
		}break;

		case 'A':
		{
			m_KeyState[Keys::A] = true;
		}break;

		case 'D':
		{
			m_KeyState[Keys::D] = true;
		}break;

		case VK_UP:
		{
			m_KeyState[Keys::AUp] = true;
		}break;

		case VK_DOWN:
		{
			m_KeyState[Keys::ADown] = true;
		}break;

		case VK_RIGHT:
		{
			m_KeyState[Keys::ARight] = true;
		}break;

		case VK_LEFT:
		{
			m_KeyState[Keys::ALeft] = true;
		}break;
		}
	}
	else
	{
		switch (keycode)
		{
		case 'W':
		{
			m_KeyState[Keys::W] = false;
		}break;

		case 'S':
		{
			m_KeyState[Keys::S] = false;
		}break;

		case 'A':
		{
			m_KeyState[Keys::A] = false;
		}break;

		case 'D':
		{
			m_KeyState[Keys::D] = false;
		}break;

		case VK_UP:
		{
			m_KeyState[Keys::AUp] = false;
		}break;

		case VK_DOWN:
		{
			m_KeyState[Keys::ADown] = false;
		}break;

		case VK_RIGHT:
		{
			m_KeyState[Keys::ARight] = false;
		}break;

		case VK_LEFT:
		{
			m_KeyState[Keys::ALeft] = false;
		}break;
		}
	}
}


void Camera::Update(float deltaTime)
{
	float movementSpeed = deltaTime * m_MovementSpeed / 10.0f;
	
	if (m_KeyState[Keys::W])
	{
		m_CameraPosition +=  m_CameraFront * movementSpeed;
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

dx::XMMATRIX Camera::GetViewMatrix() const
{
	return m_ViewMatrix;
}