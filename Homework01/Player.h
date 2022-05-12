#pragma once

#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
private:
	CCamera* m_pCamera = NULL;

private:
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

private:
	XMFLOAT3 m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

private:
	float m_fFriction = 125.0f;
	float m_fPitch = 0.0f;
	float m_fYaw = 0.0f;
	float m_fRoll = 0.0f;

private:
	int m_nRailIndex = 0;
	int m_nRailPos = 1;
	bool m_bMove = false;
	int m_nLeftRail = 0;

protected:
	std::vector<DIR> m_vRail;

public:
	CPlayer();
	virtual ~CPlayer();

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetMove(bool bMove) { m_bMove = bMove; }

	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void Move(DWORD dwDirection, float fDistance);
	void Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
	void SetCameraOffset(XMFLOAT3& xmf3CameraOffset);

	void Update(float fTimeElapsed = 0.016f);

	void GetRailFromFile();
	void FollowRail();

public:
	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() { return m_pCamera; }

public:
	XMFLOAT3& GetPos() { return m_xmf3Position; }
	XMFLOAT3& GetRight() { return m_xmf3Right; }
	XMFLOAT3& GetUp() { return m_xmf3Up; }
	XMFLOAT3& GetLook() { return m_xmf3Look; }
	XMFLOAT3& GetCameraOffset() { return m_xmf3CameraOffset; }
	const XMFLOAT3& GetVelocity() { return m_xmf3Velocity; }
	const bool GetMove() { return m_bMove; }
};

class CRollerCoasterPlayer : public CPlayer
{
public:
	CRollerCoasterPlayer();
	virtual ~CRollerCoasterPlayer();

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);


};
