#pragma once
#include "Timer.h"
#include "Player.h"
#include "Scene.h"

class CGameFramework
{
public:
	CGameFramework() {}
	~CGameFramework() {}

private:
	HINSTANCE m_hInstance = NULL;
	HWND m_hWnd = NULL;
	HDC m_hDCFrameBuffer = NULL;
	HBITMAP m_hBitmapFrameBuffer = NULL;
	HBITMAP m_hBitmapSelect = NULL;

	RECT m_rcClient = {};
	std::array<_TCHAR, 50> m_pszFrameRate;

private:
	CGameTimer m_GameTimer;
	CPlayer* m_pPlayer = NULL;
	CScene* m_pScene = NULL;

public:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

public:
	void BuildFrameBuffer();
	void ClearFrameBuffer(DWORD dwColor);
	void PresentFrameBuffer();

public:
	void BuildObjects();
	void ReleaseObjects();

public:
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

