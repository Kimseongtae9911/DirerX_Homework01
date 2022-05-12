#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"

class CScene
{
public:
	CScene(CPlayer* pPlayer);
	virtual ~CScene();

private:
	int	m_nObjects = 0;
	CGameObject** m_ppObjects = NULL;

	CPlayer* m_pPlayer = NULL;
	CBackGroundObject* m_pBackGround = NULL;

private:
	std::vector<DIR> m_vRail;
	int m_nLeftRail = 0;
	XMFLOAT3 m_xmf3RailPos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_xmf3RailAngle = { 0.0f, 0.0f, 0.0f };

#ifdef _WITH_DRAW_AXIS
	CGameObject* m_pWorldAxis = NULL;
#endif

public:
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	void CheckObjectByObjectCollisions();

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

public:
	void MakeRail();
	void FindRailPos(int n);
	const std::vector<DIR>& GetRail() { return m_vRail; }
};

