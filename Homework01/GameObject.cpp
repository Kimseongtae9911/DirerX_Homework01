#include "pch.h"
#include "GameObject.h"

CGameObject::~CGameObject()
{
}

void CGameObject::SetRotationTransform(std::unique_ptr<XMFLOAT4X4> pmxf4x4Transform)
{
}

void CGameObject::SetPosition(float x, float y, float z)
{
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
}

void CGameObject::MoveStrafe(float fDistance)
{
}

void CGameObject::MoveUp(float fDistance)
{
}

void CGameObject::MoveForward(float fDistance)
{
}

void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
}

void CGameObject::Rotate(XMFLOAT3& xmf3Axis, float fAngle)
{
}

XMFLOAT3 CGameObject::GetPosition()
{
    return XMFLOAT3();
}

XMFLOAT3 CGameObject::GetLook()
{
    return XMFLOAT3();
}

XMFLOAT3 CGameObject::GetUp()
{
    return XMFLOAT3();
}

XMFLOAT3 CGameObject::GetRight()
{
    return XMFLOAT3();
}

void CGameObject::LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up)
{
}

void CGameObject::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
}

void CGameObject::UpdateBoundingBox()
{
}

void CGameObject::Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh)
{
}

void CGameObject::Animate(float fElapsedTime)
{
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
}

void CGameObject::GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection)
{
}

int CGameObject::PickObjectByRayIntersection(XMVECTOR& xmPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance)
{
    return 0;
}
