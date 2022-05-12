#include "pch.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_xmf3RailPos = { 0.0f, -2.5f, 0.0f };
}

CScene::~CScene()
{

}

void CScene::BuildObjects()
{
	float fHalfWidth = 45.0f, fHalfHeight = 45.0f, fHalfDepth = 200.0f;
	CBackGroundMesh* pBackCubeMesh = new CBackGroundMesh(fHalfWidth * 2.0f, fHalfHeight * 2.0f, fHalfDepth * 2.0f, 30);

	m_pBackGround = new CBackGroundObject();
	m_pBackGround->SetPosition(0.0f, 0.0f, 0.0f);
	m_pBackGround->SetMesh(pBackCubeMesh);
	m_pBackGround->SetColor(RGB(0, 0, 0));
	/*m_pBackGround->m_pxmf4WallPlanes[0] = XMFLOAT4(+1.0f, 0.0f, 0.0f, fHalfWidth);
	m_pBackGround->m_pxmf4WallPlanes[1] = XMFLOAT4(-1.0f, 0.0f, 0.0f, fHalfWidth);
	m_pBackGround->m_pxmf4WallPlanes[2] = XMFLOAT4(0.0f, +1.0f, 0.0f, fHalfHeight);
	m_pBackGround->m_pxmf4WallPlanes[3] = XMFLOAT4(0.0f, -1.0f, 0.0f, fHalfHeight);
	m_pBackGround->m_pxmf4WallPlanes[4] = XMFLOAT4(0.0f, 0.0f, +1.0f, fHalfDepth);
	m_pBackGround->m_pxmf4WallPlanes[5] = XMFLOAT4(0.0f, 0.0f, -1.0f, fHalfDepth);
	m_pBackGround->m_xmOOBBPlayerMoveCheck = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth * 0.05f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));*/
	
	MakeRail();

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void CScene::ReleaseObjects()
{
	for (int i = 0; i < m_nObjects; i++)
		if (m_ppObjects[i]) 
			delete m_ppObjects[i];
	if (m_ppObjects) 
		delete[] m_ppObjects;

	if (m_pBackGround) 
		delete m_pBackGround;
}

void CScene::CheckObjectByObjectCollisions()
{
	for (int i = 0; i < m_nObjects; i++)
		m_ppObjects[i]->GetObjectCollided() = NULL;
	

	//for (int i = 0; i < m_nObjects; i++)
	//{
	//	for (int j = (i + 1); j < m_nObjects; j++)
	//	{
	//		if (m_ppObjects[i]->GetBOBox().Intersects(m_ppObjects[j]->GetBOBox()))
	//		{
	//			m_ppObjects[i]->GetObjectCollided() = m_ppObjects[j];
	//			m_ppObjects[j]->GetObjectCollided() = m_ppObjects[i];
	//		}
	//	}
	//}
	//for (int i = 0; i < m_nObjects; i++)
	//{
	//	if (m_ppObjects[i]->GetObjectCollided())
	//	{
	//		XMFLOAT3 xmf3MovingDirection = m_ppObjects[i]->GetMovingDirection();
	//		float fMovingSpeed = m_ppObjects[i]->GetMovingSpeed();
	//		m_ppObjects[i]->SetMovingDirection(m_ppObjects[i]->GetObjectCollided()->GetMovingDirection());
	//		m_ppObjects[i]->SetMovingSpeed(m_ppObjects[i]->GetObjectCollided()->GetMovingSpeed());
	//		m_ppObjects[i]->GetObjectCollided()->SetMovingDirection(xmf3MovingDirection);
	//		m_ppObjects[i]->GetObjectCollided()->SetMovingSpeed(fMovingSpeed);
	//		m_ppObjects[i]->GetObjectCollided()->GetObjectCollided() = NULL;
	//		m_ppObjects[i]->GetObjectCollided() = NULL;
	//	}
	//}
}

void CScene::Animate(float fElapsedTime)
{
	m_pBackGround->Animate(fElapsedTime);

	for (int i = 0; i < m_nObjects; i++) 
		m_ppObjects[i]->Animate(fElapsedTime);


	CheckObjectByObjectCollisions();
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	
	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);
	
	//m_pBackGround->Render(hDCFrameBuffer, pCamera);
	 
	for (int i = 0; i < m_nObjects; ++i) 
		m_ppObjects[i]->Render(hDCFrameBuffer, pCamera);

	if (m_pPlayer) 
		m_pPlayer->Render(hDCFrameBuffer, pCamera);

	//UI
#ifdef _WITH_DRAW_AXIS
	CGraphicsPipeline::SetViewOrthographicProjectTransform(&pCamera->m_xmf4x4ViewOrthographicProject);
	m_pWorldAxis->SetRotationTransform(&m_pPlayer->m_xmf4x4World);
	m_pWorldAxis->Render(hDCFrameBuffer, pCamera);
#endif
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'A':
			m_pPlayer->Rotate(0.0f, 2.0f, 0.0f);
			break;
		case 'D':
			m_pPlayer->Rotate(0.0f, -2.0f, 0.0f);
			break;
		case 'W':
			m_pPlayer->Rotate(2.0f, 0.0f, 0.0f);
			break;
		case 'S':
			m_pPlayer->Rotate(-2.0f, 0.0f, 0.0f);
			break;
		default:
			break;
		}
	}
}

void CScene::MakeRail()
{
	std::ifstream in("Rail.txt");

	int n;

	while (!in.eof()) {
		in >> n;
		m_vRail.push_back((DIR)n);
	}

	CRailMesh* pRailForwardMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::FORWARD);
	CRailMesh* pRailLeftMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::LEFT);
	CRailMesh* pRailRightMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::RIGHT);
	m_nObjects = (int)m_vRail.size();

	m_ppObjects = new CGameObject * [m_nObjects];

	for (int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i] = new CRailObject();
		m_ppObjects[i]->SetColor(RGB(0, 0, 255));

		FindRailPos(i);

		switch (m_vRail[i])
		{
		case DIR::FORWARD:		//forward
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			m_ppObjects[i]->Rotate(m_xmf3RailAngle.x, m_xmf3RailAngle.y, m_xmf3RailAngle.z);
			break;
		case DIR::LEFT:		//left
			m_ppObjects[i]->SetMesh(pRailLeftMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			m_ppObjects[i]->Rotate(m_xmf3RailAngle.x, m_xmf3RailAngle.y, m_xmf3RailAngle.z);
			break;
		case DIR::RIGHT:		//right
			m_ppObjects[i]->SetMesh(pRailRightMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			m_ppObjects[i]->Rotate(m_xmf3RailAngle.x, m_xmf3RailAngle.y, m_xmf3RailAngle.z);
			break;
		case DIR::UP:		//up
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			m_ppObjects[i]->Rotate(m_xmf3RailAngle.x, m_xmf3RailAngle.y, m_xmf3RailAngle.z);
			break;
		case DIR::DOWN:		//down
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			m_ppObjects[i]->Rotate(m_xmf3RailAngle.x, m_xmf3RailAngle.y, m_xmf3RailAngle.z);
			break;
		default:
			break;
		}
	}
}

void CScene::FindRailPos(int n)
{
	if (n <= 0)
		return;

	if (m_nLeftRail > 0) {
		m_xmf3RailPos.z -= m_nLeftRail * 1.2f;
		m_xmf3RailPos.x -= m_nLeftRail * 2.85f;
	}

	if (m_vRail[n] == DIR::FORWARD && m_vRail[n - 1] == DIR::FORWARD)
	{
		m_xmf3RailPos.z += 4.0f;
	}
	else if(m_vRail[n] == DIR::UP && m_vRail[n - 1] == DIR::FORWARD)
	{
		m_xmf3RailAngle.x = -60.0f;
		m_xmf3RailPos.y += 2 * sin(DegreeToRadian(-m_xmf3RailAngle.x)) + 0.17f;
		m_xmf3RailPos.z += 2 + 2 * cos(DegreeToRadian(m_xmf3RailAngle.x)) + 0.5f;
		if (m_nLeftRail) {
			m_xmf3RailPos.x += 0.45f;
			m_xmf3RailPos.y += 0.15f;
			m_xmf3RailPos.z += 0.1f;
		}
	}
	else if (m_vRail[n] == DIR::DOWN && m_vRail[n - 1] == DIR::FORWARD)
	{
		m_xmf3RailAngle.x = 60.0f;
		m_xmf3RailPos.y -= 2 * sin(DegreeToRadian(m_xmf3RailAngle.x)) - 0.17f;
		m_xmf3RailPos.z += 2 + 2 * cos(DegreeToRadian(m_xmf3RailAngle.x)) - 0.5f;
		if (m_nLeftRail) {
			m_xmf3RailPos.x += 1.0f;
			m_xmf3RailPos.y += 0.1f;
			m_xmf3RailPos.z += 0.4f;
		}
	}
	else if (m_vRail[n] == DIR::LEFT && m_vRail[n - 1] == DIR::FORWARD)
	{
		m_xmf3RailPos.z += 4.0f;
	}
	else if (m_vRail[n] == DIR::RIGHT && m_vRail[n - 1] == DIR::FORWARD)
	{
		m_xmf3RailPos.z += 4.0f;
	}
	else if (m_vRail[n] == DIR::FORWARD && m_vRail[n - 1] == DIR::UP)
	{
		m_xmf3RailPos.z += 2.0f + 2 * cos(DegreeToRadian(m_xmf3RailAngle.x)) - sin(DegreeToRadian(30.0f));
		m_xmf3RailPos.y += 2 * sin(DegreeToRadian(-m_xmf3RailAngle.x)) - 0.78f + sin(DegreeToRadian(30.0f));
		m_xmf3RailAngle.x = 0.0f;
		if (m_nLeftRail) {
			m_xmf3RailPos.x += 1.0f;
			m_xmf3RailPos.y += 0.1f;
			m_xmf3RailPos.z += 0.4f;
		}
	}
	else if (m_vRail[n] == DIR::UP && m_vRail[n - 1] == DIR::UP)
	{
		m_xmf3RailPos.y += 2 * 2 * sin(DegreeToRadian(-m_xmf3RailAngle.x));
		m_xmf3RailPos.z += 2 * 2 * cos(DegreeToRadian(m_xmf3RailAngle.x));
		if (m_nLeftRail) {
			m_xmf3RailPos.x += 1.4f;
			m_xmf3RailPos.y += 0.0f;
			m_xmf3RailPos.z += 0.6f;
		}
	}
	else if (m_vRail[n] == DIR::DOWN && m_vRail[n - 1] == DIR::DOWN)
	{
		m_xmf3RailPos.y -= 2 * 2 * sin(DegreeToRadian(m_xmf3RailAngle.x));
		m_xmf3RailPos.z += 2 * 2 * cos(DegreeToRadian(m_xmf3RailAngle.x));
		if (m_nLeftRail) {
			m_xmf3RailPos.x += 1.4f;
			m_xmf3RailPos.y += 0.0f;
			m_xmf3RailPos.z += 0.6f;
		}
	}
	else if (m_vRail[n] == DIR::FORWARD && m_vRail[n - 1] == DIR::DOWN)
	{
		m_xmf3RailPos.z += 2.95f + 2 * cos(DegreeToRadian(-m_xmf3RailAngle.x)) - sin(DegreeToRadian(30.0f));
		m_xmf3RailPos.y -= 2 * sin(DegreeToRadian(m_xmf3RailAngle.x)) - 0.78f + 1.0f;
		m_xmf3RailAngle.x = 0.0f;
	}
	else if (m_vRail[n] == DIR::FORWARD && m_vRail[n - 1] == DIR::LEFT)
	{
		++m_nLeftRail;
		m_xmf3RailAngle.y = m_nLeftRail * -45.0f;
		m_xmf3RailPos.x -= 2 * 2 * sin(DegreeToRadian(-m_xmf3RailAngle.y));
		m_xmf3RailPos.z += 2 * 2 * cos(DegreeToRadian(m_xmf3RailAngle.y)) - 0.8f;
		
		//m_xmf3RailPos.z += 2.4f;
		//m_xmf3RailPos.x -= 2.9f;
		//m_xmf3RailAxis = { 0.0f, 1.0f, 0.0f };
		//m_xmf3RailAngle.y -= 33.0f;
	}
	else if (m_vRail[n] == DIR::LEFT && m_vRail[n - 1] == DIR::LEFT)
	{

	}
	else if (m_vRail[n] == DIR::FORWARD && m_vRail[n - 1] == DIR::RIGHT)
	{

	}
	else if (m_vRail[n] == DIR::RIGHT && m_vRail[n - 1] == DIR::RIGHT)
	{

	}

}
