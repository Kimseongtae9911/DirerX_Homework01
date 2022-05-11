#include "pch.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_xmf3RailPos = { 0.0f, -2.5f, 4.0f };
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
}

void CScene::MakeRail()
{
	std::ifstream in("Rail.txt");

	int n;

	while (!in.eof()) {
		in >> n;
		m_vRail.push_back(n);
	}

	CRailMesh* pRailForwardMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::FORWARD);
	CRailMesh* pRailLeftMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::LEFT);
	CRailMesh* pRailRightMesh = new CRailMesh(4.0f, 1.0f, 4.0f, DIR::RIGHT);
	m_nObjects = (int)m_vRail.size();

	m_ppObjects = new CGameObject * [m_nObjects];

	for (int i = 0; i < m_nObjects; ++i) {
		m_ppObjects[i] = new CRailObject();
		m_ppObjects[i]->SetColor(RGB(0, 0, 255));
		switch (m_vRail[i])
		{
		case 0:		//forward
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			if(!IsEqual(m_fRailAngle, 0))
				m_ppObjects[i]->Rotate(m_xmf3RailAxis, m_fRailAngle);
			m_xmf3RailPos.z += 4.0f;
			break;
		case 1:		//left
			m_ppObjects[i]->SetMesh(pRailLeftMesh);
			m_ppObjects[i]->SetPosition(m_xmf3RailPos);
			if (!IsEqual(m_fRailAngle, 0))
				m_ppObjects[i]->Rotate(m_xmf3RailAxis, m_fRailAngle);
			m_xmf3RailPos.z += 2.0f;
			m_xmf3RailPos.x -= 3.5f;
			m_xmf3RailAxis.y = 1.0f;
			m_fRailAngle -= 35.0f;
			break;
		case 2:		//right
			m_ppObjects[i]->SetMesh(pRailRightMesh);
			m_ppObjects[i]->SetPosition(0.0f, -2.5f, 4.0f * i);
			break;
		case 3:		//up
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(0.0f, -2.5f, 4.0f * i);
			break;
		case 4:		//down
			m_ppObjects[i]->SetMesh(pRailForwardMesh);
			m_ppObjects[i]->SetPosition(0.0f, -2.5f, 4.0f * i);
			break;
		default:
			break;
		}
	}
}
