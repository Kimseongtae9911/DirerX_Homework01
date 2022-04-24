#pragma once
#include "Mesh.h"
#include "Camera.h"

class CGameObject
{
protected:
	CGameObject* m_pObjectCollided = NULL;
	CMesh* m_pMesh = NULL;

	DWORD m_dwColor = RGB(255, 0, 0);	

protected:
	XMFLOAT4X4 m_xmf4x4World = Matrix4x4::Identity();
	XMFLOAT3 m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);

	BoundingOrientedBox	m_xmOOBB = BoundingOrientedBox();

private:
	float m_fMovingSpeed = 0.0f;
	float m_fMovingRange = 0.0f;
	float m_fRotationSpeed = 0.0f;

public:
	CGameObject() {}
	virtual ~CGameObject();

public:
	void SetMesh(CMesh* pMesh) { m_pMesh = pMesh; if (pMesh) pMesh->AddRef(); }

	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }

	void SetRotationTransform(std::unique_ptr<XMFLOAT4X4> pmxf4x4Transform);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);

	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void SetRotationAxis(XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3& vDirection, float fSpeed);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);

	void LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up);
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void UpdateBoundingBox();

	void Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh);

	virtual void OnUpdateTransform() { }
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	int PickObjectByRayIntersection(XMVECTOR& xmPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance);

public:
	XMFLOAT3& GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	CGameObject*& GetObjectCollided() { return m_pObjectCollided; }
	CGameObject* GetObjectCollided() const { return m_pObjectCollided; }

	BoundingOrientedBox& GetBOBox() { return m_xmOOBB; }

	float GetMovingSpeed() const { return m_fMovingSpeed; }
	XMFLOAT3 GetMovingDirection() const { return m_xmf3MovingDirection; }
	XMFLOAT3& GetMovingDirection() { return m_xmf3MovingDirection; }
		
	void SetWorldMatrix(XMFLOAT4X4 xmf4x4World) { m_xmf4x4World = xmf4x4World; }
	XMFLOAT4X4 GetWorldMatrix() const { return m_xmf4x4World; }
};

class CBackGroundObject : public CGameObject
{
public:
	CBackGroundObject();
	virtual ~CBackGroundObject();

public:
	BoundingOrientedBox	m_xmOOBBPlayerMoveCheck = BoundingOrientedBox();
	XMFLOAT4 m_pxmf4WallPlanes[6] = {};

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};

class CRailObject : public CGameObject
{
public:
	CRailObject() {}
	virtual ~CRailObject() {}

public:
	BoundingOrientedBox	m_xmOOBBPlayerMoveCheck = BoundingOrientedBox();
	XMFLOAT4 m_pxmf4WallPlanes[6] = {};

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};