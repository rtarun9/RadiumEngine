#include "Pch.hpp"

enum Keys
{
	W,
	A,
	S,
	D,
	AUp,
	ADown,
	ALeft,
	ARight,
	TotalKeyCount
};

class Camera
{
public:
	Camera();

	void HandleInput(uint32_t keycode, bool isKeyDown);
	void Update(float deltaTime);

	dx::XMMATRIX GetViewMatrix() const; 

public:
	dx::XMVECTOR m_WorldFront;
	dx::XMVECTOR m_WorldRight;
	dx::XMVECTOR m_WorldUp;

	dx::XMVECTOR m_CameraFront;
	dx::XMVECTOR m_CameraRight;
	dx::XMVECTOR m_CameraUp;

	dx::XMVECTOR m_CameraTarget;
	dx::XMVECTOR m_CameraPosition;

	dx::XMMATRIX m_CameraRotationMatrix;

	dx::XMMATRIX m_ViewMatrix;
 
	// Movement speed is for WASD
	// Rotation speed is for orientation
	float m_MovementSpeed;
	float m_RotationSpeed;

	float m_Yaw;
	float m_Pitch;

	bool m_KeyState[Keys::TotalKeyCount];
};