#include "pch.h"
#include "Player.h"

CPlayer::CPlayer()
{
	GetRailFromFile();
}

CPlayer::~CPlayer()
{
	if (m_pCamera)
		delete m_pCamera;
}

void CPlayer::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x, y, z);

	CGameObject::SetPosition(x, y, z);
}

void CPlayer::SetRotation(float x, float y, float z)
{
}

void CPlayer::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4View._11, xmf4x4View._21, xmf4x4View._31));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4View._12, xmf4x4View._22, xmf4x4View._32));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4View._13, xmf4x4View._23, xmf4x4View._33));
}

void CPlayer::Move(DWORD dwDirection, float fDistance)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, true);
	}
}

void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(xmf3Shift, m_xmf3Position);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(float x, float y, float z)
{
	Move((XMFLOAT3&)(x, y, z), false);
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
	m_pCamera->Rotate(fPitch, fYaw, fRoll);
	if (fPitch != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
	m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}

void CPlayer::SetCameraOffset(XMFLOAT3& xmf3CameraOffset)
{
	m_xmf3CameraOffset = xmf3CameraOffset;
	m_pCamera->SetLookAt((XMFLOAT3&)Vector3::Add(m_xmf3Position, m_xmf3CameraOffset), m_xmf3Position, m_xmf3Up);
	m_pCamera->GenerateViewMatrix();

}

void CPlayer::Update(float fTimeElapsed)
{
	Move(m_xmf3Velocity, false);

	m_pCamera->Update(this, m_xmf3Position, fTimeElapsed);
	m_pCamera->GenerateViewMatrix();

	XMFLOAT3 xmf3Deceleration = Vector3::Normalize(Vector3::ScalarProduct(m_xmf3Velocity, -1.0f));
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > fLength)
		fDeceleration = fLength;

	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Deceleration, fDeceleration);
}

void CPlayer::GetRailFromFile()
{
	std::ifstream in("Rail.txt");
	int temp;

	while (!in.eof()) {
		in >> temp;
		m_vRail.push_back((DIR)temp);
	}
}

void CPlayer::FollowRail()
{		
	if (m_vRail[m_nRailIndex] == DIR::FORWARD) {
		if (m_nRailIndex < m_vRail.size() - 1) {
			if (m_vRail[m_nRailIndex + 1] == DIR::UP) {
				Rotate(-2.4f, 0.0f, 0.0f);
				m_nRailPos += 1;
			}
			else if (m_vRail[m_nRailIndex + 1] == DIR::DOWN) {
				Rotate(2.4f, 0.0f, 0.0f);
				Move(DIR_FORWARD, 0.16f);
				m_nRailPos += 1;
			}
			else if (m_vRail[m_nRailIndex + 1] == DIR::LEFT) {
				Rotate(0.0f, -1.8f, 0.0f);
				Move(DIR_FORWARD, 0.22f);
				m_nRailPos += 1;
			}
			else if (m_vRail[m_nRailIndex + 1] == DIR::RIGHT) {

			}
			else {
				Move(DIR_FORWARD, 0.16f);
				m_nRailPos += 1;
				if (m_nLeftRail)
					Move(DIR_LEFT, 0.002f);
			}
		}
		else {
			Move(DIR_FORWARD, 0.16f);
			m_nRailPos += 1;
		}

		if (m_nRailPos % 26 == 0) {
			m_nRailIndex += 1;
			m_nRailPos = 1;
		}
	}
	else if (m_vRail[m_nRailIndex] == DIR::UP) {
		if (m_nRailIndex < m_vRail.size() - 1) {
			if (m_vRail[m_nRailIndex + 1] == DIR::FORWARD) {
				Rotate(1.4f, 0.0f, 0.0f);
				Move(DIR_FORWARD, 0.15f);
				m_nRailPos += 1;
				if(m_nLeftRail)
					Move(DIR_LEFT, 0.002f);
			}
			else {
				Move(DIR_FORWARD, 0.1f);
				m_nRailPos += 1;
			}
		}
		else {
			Move(DIR_FORWARD, 0.1f);
			m_nRailPos += 1;
		}

		if (m_nRailPos % 41 == 0) {
			m_nRailIndex += 1;
			m_nRailPos = 1;
			std::cout << m_nRailIndex << std::endl;
		}
	}
	else if (m_vRail[m_nRailIndex] == DIR::DOWN) {
		if (m_nRailIndex < m_vRail.size() - 1) {
			if (m_vRail[m_nRailIndex + 1] == DIR::FORWARD) {
				Rotate(1.4f, 0.0f, 0.0f);
				Move(DIR_FORWARD, 0.1f);
				m_nRailPos += 1;
			}
			else {
				Move(DIR_FORWARD, 0.5f);
				m_nRailPos += 1;
			}
		}
		else {
			Move(DIR_FORWARD, 0.5f);
			m_nRailPos += 1;
		}

		if (m_nRailPos % 9 == 0) {
			m_nRailIndex += 1;
			m_nRailPos = 1;
			std::cout << m_nRailIndex << std::endl;
		}
	}
	else if (m_vRail[m_nRailIndex] == DIR::LEFT) {
		Move(DIR_RIGHT, 0.1f);
		m_nRailIndex += 1;
		m_nLeftRail += 1;
	}
	else if (m_vRail[m_nRailIndex] == DIR::RIGHT) {

	}
}

void CPlayer::OnUpdateTransform()
{
	XMFLOAT4X4 xmf4x4World = Matrix4x4::Identity();
	xmf4x4World._11 = m_xmf3Right.x;	xmf4x4World._12 = m_xmf3Right.y;	xmf4x4World._13 = m_xmf3Right.z;
	xmf4x4World._21 = m_xmf3Up.x;		xmf4x4World._22 = m_xmf3Up.y;		xmf4x4World._23 = m_xmf3Up.z;
	xmf4x4World._31 = m_xmf3Look.x;		xmf4x4World._32 = m_xmf3Look.y;		xmf4x4World._33 = m_xmf3Look.z;
	xmf4x4World._41 = m_xmf3Position.x; xmf4x4World._42 = m_xmf3Position.y; xmf4x4World._43 = m_xmf3Position.z;

	SetWorldMatrix(xmf4x4World);
}

void CPlayer::Animate(float fElapsedTime)
{
	OnUpdateTransform();

	CGameObject::Animate(fElapsedTime);
}

void CPlayer::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, pCamera);
}

CRollerCoasterPlayer::CRollerCoasterPlayer()
{
}

CRollerCoasterPlayer::~CRollerCoasterPlayer()
{
}

void CRollerCoasterPlayer::OnUpdateTransform()
{
	CPlayer::OnUpdateTransform();

	XMMATRIX XMRollPitchYaw = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	XMFLOAT4X4 xmf4x4world = GetWorldMatrix();

	SetWorldMatrix(Matrix4x4::Multiply(XMRollPitchYaw, xmf4x4world));	
}

void CRollerCoasterPlayer::Animate(float fElapsedTime)
{
	CPlayer::Animate(fElapsedTime);
}

void CRollerCoasterPlayer::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CPlayer::Render(hDCFrameBuffer, pCamera);
}
