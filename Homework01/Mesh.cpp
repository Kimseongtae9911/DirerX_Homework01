#include "pch.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

CPolygon::CPolygon(int nVertices)
{
    m_nVertices = nVertices;
    m_pVertices = new CVertex[nVertices];
}

CPolygon::~CPolygon()
{
    if (m_pVertices)
        delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
    if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
        m_pVertices[nIndex] = vertex;
}

CMesh::CMesh(int nPolygons)
{
    m_nPolygons = nPolygons;
    m_ppPolygons = new CPolygon * [nPolygons];
}

CMesh::~CMesh()
{
    if (m_ppPolygons)
    {
        for (int i = 0; i < m_nPolygons; i++) 
            if (m_ppPolygons[i]) 
                delete m_ppPolygons[i];
        delete[] m_ppPolygons;
    }
}

void CMesh::SetPolygon(int nIndex, CPolygon* pPolygon)
{
    if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
        m_ppPolygons[nIndex] = pPolygon;
    }
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject)
{
    XMFLOAT3 f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
    XMFLOAT3 f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
    ::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
    ::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
    XMFLOAT3 f3InitialProject, f3PreviousProject;
    bool bPreviousInside = false, bInitialInside = false, bCurrentInside = false, bIntersectInside = false;

    for (int j = 0; j < m_nPolygons; j++)
    {
        int nVertices = m_ppPolygons[j]->GetVertexNum();
        CVertex* pVertices = m_ppPolygons[j]->GetVertex();

        f3PreviousProject = f3InitialProject = CGraphicsPipeline::Project(pVertices[0].GetPos());
        bPreviousInside = bInitialInside = (-1.0f <= f3InitialProject.x) && (f3InitialProject.x <= 1.0f) && (-1.0f <= f3InitialProject.y) && (f3InitialProject.y <= 1.0f);
        for (int i = 1; i < nVertices; i++)
        {
            XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].GetPos());
            bCurrentInside = (-1.0f <= f3CurrentProject.x) && (f3CurrentProject.x <= 1.0f) && (-1.0f <= f3CurrentProject.y) && (f3CurrentProject.y <= 1.0f);
            if (((0.0f <= f3CurrentProject.z) && (f3CurrentProject.z <= 1.0f)) && ((bCurrentInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
            f3PreviousProject = f3CurrentProject;
            bPreviousInside = bCurrentInside;
        }
        if (((0.0f <= f3InitialProject.z) && (f3InitialProject.z <= 1.0f)) && ((bInitialInside || bPreviousInside))) ::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
    }
}

BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
    float fHitDistance;
    BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
    if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

    return(bIntersected);
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
    int nIntersections = 0;
    bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);
    if (bIntersected)
    {
        for (int i = 0; i < m_nPolygons; i++)
        {
            switch (m_ppPolygons[i]->GetVertexNum())
            {
            case 3:
            {
                XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[0].GetPos()));
                XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[1].GetPos()));
                XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[2].GetPos()));
                BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
                if (bIntersected) nIntersections++;
                break;
            }
            case 4:
            {
                XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[0].GetPos()));
                XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[1].GetPos()));
                XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[2].GetPos()));
                BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
                if (bIntersected) nIntersections++;
                v0 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[0].GetPos()));
                v1 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[2].GetPos()));
                v2 = XMLoadFloat3(&(m_ppPolygons[i]->GetVertex()[3].GetPos()));
                bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
                if (bIntersected) nIntersections++;
                break;
            }
            }
        }
    }
    return(nIntersections);
}

CRollerCoasterMesh::CRollerCoasterMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
    float fHalfWidth = fWidth * 0.5f;
    float fHalfHeight = fHeight * 0.5f;
    float fHalfDepth = fDepth * 0.5f;

    CVertex FrontTopLeft = CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth);
    CVertex FrontTopRight = CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth);
    CVertex FrontBottomLeft = CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth);
    CVertex FrontBottomRight = CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth);
    CVertex BackTopLeft = CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth);
    CVertex BackTopRight = CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth);
    CVertex BackBottomLeft = CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth);
    CVertex BackBottomRight = CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth);

    CPolygon* pFrontFace = new CPolygon(4);
    pFrontFace->SetVertex(0, FrontTopRight);
    pFrontFace->SetVertex(1, FrontTopLeft);
    pFrontFace->SetVertex(2, FrontBottomLeft);
    pFrontFace->SetVertex(3, FrontBottomRight);
    SetPolygon(0, pFrontFace);

    CPolygon* pTopFace = new CPolygon(4);
    pTopFace->SetVertex(0, BackTopRight);
    pTopFace->SetVertex(1, BackTopLeft);
    pTopFace->SetVertex(2, FrontTopLeft);
    pTopFace->SetVertex(3, FrontTopRight);
    SetPolygon(1, pTopFace);

    CPolygon* pBackFace = new CPolygon(4);
    pBackFace->SetVertex(0, BackBottomRight);
    pBackFace->SetVertex(1, BackBottomLeft);
    pBackFace->SetVertex(2, BackTopLeft);
    pBackFace->SetVertex(3, BackTopRight);
    SetPolygon(2, pBackFace);

    CPolygon* pBottomFace = new CPolygon(4);
    pBottomFace->SetVertex(0, FrontBottomRight);
    pBottomFace->SetVertex(1, FrontBottomLeft);
    pBottomFace->SetVertex(2, BackBottomLeft);
    pBottomFace->SetVertex(3, BackBottomRight);
    SetPolygon(3, pBottomFace);

    CPolygon* pLeftFace = new CPolygon(4);
    pLeftFace->SetVertex(0, BackTopRight);
    pLeftFace->SetVertex(1, FrontTopRight);
    pLeftFace->SetVertex(2, FrontBottomRight);
    pLeftFace->SetVertex(3, BackBottomRight);
    SetPolygon(4, pLeftFace);

    CPolygon* pRightFace = new CPolygon(4);
    pRightFace->SetVertex(0, FrontTopLeft);
    pRightFace->SetVertex(1, BackTopLeft);
    pRightFace->SetVertex(2, BackBottomLeft);
    pRightFace->SetVertex(3, FrontBottomLeft);
    SetPolygon(5, pRightFace);

    m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CBackGroundMesh::CBackGroundMesh(float fWidth, float fHeight, float fDepth, int nSubRects) : CMesh((4 * nSubRects * nSubRects) + 2)
{
    float fHalfWidth = fWidth * 0.5f;
    float fHalfHeight = fHeight * 0.5f;
    float fHalfDepth = fDepth * 0.5f;
    float fCellWidth = fWidth * (1.0f / nSubRects);
    float fCellHeight = fHeight * (1.0f / nSubRects);
    float fCellDepth = fDepth * (1.0f / nSubRects);

    CVertex Vertex1 = CVertex(-fHalfWidth, -fHalfHeight + (0 * fCellHeight), -fHalfDepth + (0 * fCellDepth));
    CVertex Vertex2 = CVertex(-fHalfWidth, -fHalfHeight + ((0 + 1) * fCellHeight), -fHalfDepth + (0 * fCellDepth));
    CVertex Vertex3 = CVertex(-fHalfWidth, -fHalfHeight + ((0 + 1) * fCellHeight), -fHalfDepth + ((0 + 1) * fCellDepth));
    CVertex Vertex4 = CVertex(-fHalfWidth, -fHalfHeight + (0 * fCellHeight), -fHalfDepth + ((0 + 1) * fCellDepth));

    CVertex Vertex5 = CVertex(-fHalfWidth + (0 * fCellWidth), +fHalfHeight, -fHalfDepth + (0 * fCellDepth));
    CVertex Vertex6 = CVertex(-fHalfWidth + ((0 + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + (0 * fCellDepth));
    CVertex Vertex7 = CVertex(-fHalfWidth + ((0 + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + ((0 + 1) * fCellDepth));
    CVertex Vertex8 = CVertex(-fHalfWidth + (0 * fCellWidth), +fHalfHeight, -fHalfDepth + ((0 + 1) * fCellDepth));
    

    int k = 0;
    CPolygon* pLeftFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            Vertex1.SetPos(Vertex1.GetPos().x, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth));
            Vertex2.SetPos(Vertex2.GetPos().x, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth));
            Vertex3.SetPos(Vertex3.GetPos().x, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth));
            Vertex4.SetPos(Vertex4.GetPos().x, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth));

            pLeftFace = new CPolygon(4);
            pLeftFace->SetVertex(0, Vertex1);
            pLeftFace->SetVertex(1, Vertex2);
            pLeftFace->SetVertex(2, Vertex3);
            pLeftFace->SetVertex(3, Vertex4);
            SetPolygon(k++, pLeftFace);
        }
    }

    CPolygon* pRightFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            Vertex1.SetPos(fHalfDepth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth));
            Vertex2.SetPos(fHalfDepth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth));
            Vertex3.SetPos(fHalfDepth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth));
            Vertex4.SetPos(fHalfDepth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth));

            pRightFace = new CPolygon(4);
            pRightFace->SetVertex(0, Vertex1);
            pRightFace->SetVertex(1, Vertex2);
            pRightFace->SetVertex(2, Vertex3);
            pRightFace->SetVertex(3, Vertex4);
            SetPolygon(k++, pRightFace);
        }
    }

    CPolygon* pTopFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            Vertex5.SetPos(-fHalfWidth + (i * fCellWidth), Vertex5.GetPos().y, -fHalfDepth + (j * fCellDepth));
            Vertex6.SetPos(-fHalfWidth + ((i + 1) * fCellWidth), Vertex6.GetPos().y, -fHalfDepth + (j * fCellDepth));
            Vertex7.SetPos(-fHalfWidth + (i * fCellWidth), Vertex7.GetPos().y, -fHalfDepth + ((j + 1) * fCellDepth));
            Vertex8.SetPos(-fHalfWidth + ((i + 1) * fCellWidth), Vertex8.GetPos().y, -fHalfDepth + ((j + 1) * fCellDepth));

            pTopFace = new CPolygon(4);
            pTopFace->SetVertex(0, Vertex5);
            pTopFace->SetVertex(1, Vertex6);
            pTopFace->SetVertex(2, Vertex7);
            pTopFace->SetVertex(3, Vertex8);
            SetPolygon(k++, pTopFace);
        }
    }

    CPolygon* pBottomFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            Vertex5.SetPos(Vertex5.GetPos().x, Vertex5.GetPos().y * -1, Vertex5.GetPos().z);
            Vertex6.SetPos(Vertex6.GetPos().x, Vertex6.GetPos().y * -1, Vertex6.GetPos().z);
            Vertex7.SetPos(Vertex7.GetPos().x, Vertex7.GetPos().y * -1, Vertex7.GetPos().z);
            Vertex8.SetPos(Vertex8.GetPos().x, Vertex8.GetPos().y * -1, Vertex8.GetPos().z);

            pBottomFace = new CPolygon(4);
            pBottomFace->SetVertex(0, Vertex5);
            pBottomFace->SetVertex(1, Vertex6);
            pBottomFace->SetVertex(2, Vertex7);
            pBottomFace->SetVertex(3, Vertex8);
            SetPolygon(k++, pBottomFace);
        }
    }

    CVertex Vertex9 = CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth);
    CVertex Vertex10 = CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth);
    CVertex Vertex11 = CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth);
    CVertex Vertex12 = CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth);

    CPolygon* pFrontFace = new CPolygon(4);
    pFrontFace->SetVertex(0, Vertex9);
    pFrontFace->SetVertex(1, Vertex10);
    pFrontFace->SetVertex(2, Vertex11);
    pFrontFace->SetVertex(3, Vertex12);
    SetPolygon(k++, pFrontFace);

    Vertex9.SetPos(Vertex9.GetPos().x, Vertex9.GetPos().y * -1, Vertex9.GetPos().z);
    Vertex10.SetPos(Vertex10.GetPos().x, Vertex10.GetPos().y * -1, Vertex10.GetPos().z);
    Vertex11.SetPos(Vertex11.GetPos().x, Vertex11.GetPos().y * -1, Vertex11.GetPos().z);
    Vertex12.SetPos(Vertex12.GetPos().x, Vertex12.GetPos().y * -1, Vertex12.GetPos().z);

    CPolygon* pBackFace = new CPolygon(4);
    pBackFace->SetVertex(0, Vertex12);
    pBackFace->SetVertex(1, Vertex11);
    pBackFace->SetVertex(2, Vertex10);
    pBackFace->SetVertex(3, Vertex9);
    SetPolygon(k++, pBackFace);

    m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CRailMesh::CRailMesh(float fWidth, float fHeight, float fDepth, DIR dir) : CMesh(14)
{
    CVertex temp;
    float fHalfWidth = fWidth * 0.5f;
    float fHalfHeight = fHeight * 0.5f;
    float fHalfDepth = fDepth * 0.5f;

    float fMiddleWidth = 1.0f;
    float fMiddleDepth = 0.5f;

    int i = 0;
    float radius = 30.0f;
    float fT = 0.5;
    if (dir == DIR::FORWARD) {
        //top
        CVertex FrontTopLeft1 = CVertex(fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex FrontTopLeft2 = CVertex(fHalfWidth - 1.0f, fHalfHeight, -fHalfDepth);
        CVertex BackTopLeft1 = CVertex(fHalfWidth, fHalfHeight, fHalfDepth);
        CVertex BackTopLeft2 = CVertex(fHalfWidth - 1.0f, fHalfHeight, fHalfDepth);

        CVertex FrontTopRight1 = CVertex(-fHalfWidth + 1.0f, fHalfHeight, -fHalfDepth);
        CVertex FrontTopRight2 = CVertex(-fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex BackTopRight1 = CVertex(-fHalfWidth + 1.0f, fHalfHeight, fHalfDepth);
        CVertex BackTopRight2 = CVertex(-fHalfWidth, fHalfHeight, fHalfDepth);

        CVertex FrontTopMiddle1 = CVertex(-fMiddleWidth, fHalfHeight, -fMiddleDepth);
        CVertex FrontTopMiddle2 = CVertex(fMiddleWidth, fHalfHeight, -fMiddleDepth);
        CVertex BackTopMiddle1 = CVertex(fMiddleWidth, fHalfHeight, fMiddleDepth);
        CVertex BackTopMiddle2 = CVertex(-fMiddleWidth, fHalfHeight, fMiddleDepth);

        CPolygon* pTopLeftFace = new CPolygon(4);
        pTopLeftFace->SetVertex(0, FrontTopLeft2);
        pTopLeftFace->SetVertex(1, FrontTopLeft1);
        pTopLeftFace->SetVertex(2, BackTopLeft1);
        pTopLeftFace->SetVertex(3, BackTopLeft2);
        SetPolygon(i++, pTopLeftFace);

        CPolygon* pTopRightFace = new CPolygon(4);
        pTopRightFace->SetVertex(0, FrontTopRight2);
        pTopRightFace->SetVertex(1, FrontTopRight1);
        pTopRightFace->SetVertex(2, BackTopRight1);
        pTopRightFace->SetVertex(3, BackTopRight2);
        SetPolygon(i++, pTopRightFace);

        CPolygon* pTopMiddleFace1 = new CPolygon(2);
        pTopMiddleFace1->SetVertex(0, FrontTopMiddle1);
        pTopMiddleFace1->SetVertex(1, FrontTopMiddle2);
        SetPolygon(i++, pTopMiddleFace1);

        CPolygon* pTopMiddleFace2 = new CPolygon(2);
        pTopMiddleFace2->SetVertex(0, BackTopMiddle1);
        pTopMiddleFace2->SetVertex(1, BackTopMiddle2);
        SetPolygon(i++, pTopMiddleFace2);

        //bottom
        CVertex FrontBottomLeft1 = CVertex(fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomLeft2 = CVertex(fHalfWidth - 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomLeft1 = CVertex(fHalfWidth, -fHalfHeight, fHalfDepth);
        CVertex BackBottomLeft2 = CVertex(fHalfWidth - 1.0f, -fHalfHeight, fHalfDepth);

        CVertex FrontBottomRight1 = CVertex(-fHalfWidth + 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomRight2 = CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomRight1 = CVertex(-fHalfWidth + 1.0f, -fHalfHeight, fHalfDepth);
        CVertex BackBottomRight2 = CVertex(-fHalfWidth, -fHalfHeight, fHalfDepth);

        CVertex FrontBottomMiddle1 = CVertex(-fMiddleWidth, -fHalfHeight, -fMiddleDepth);
        CVertex FrontBottomMiddle2 = CVertex(fMiddleWidth, -fHalfHeight, -fMiddleDepth);
        CVertex BackBottomMiddle1 = CVertex(fMiddleWidth, -fHalfHeight, fMiddleDepth);
        CVertex BackBottomMiddle2 = CVertex(-fMiddleWidth, -fHalfHeight, fMiddleDepth);

        CPolygon* pBottomLeftFace = new CPolygon(4);
        pBottomLeftFace->SetVertex(0, FrontBottomLeft1);
        pBottomLeftFace->SetVertex(1, BackBottomLeft1);
        pBottomLeftFace->SetVertex(2, BackBottomLeft2);
        pBottomLeftFace->SetVertex(3, FrontBottomLeft2);
        SetPolygon(i++, pBottomLeftFace);

        CPolygon* pBottomRightFace = new CPolygon(4);
        pBottomRightFace->SetVertex(0, FrontBottomRight2);
        pBottomRightFace->SetVertex(1, FrontBottomRight1);
        pBottomRightFace->SetVertex(2, BackBottomRight1);
        pBottomRightFace->SetVertex(3, BackBottomRight2);
        SetPolygon(i++, pBottomRightFace);

        CPolygon* pBottomMiddleFace1 = new CPolygon(2);
        pBottomMiddleFace1->SetVertex(0, FrontBottomMiddle1);
        pBottomMiddleFace1->SetVertex(1, FrontBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace1);

        CPolygon* pBottomMiddleFace2 = new CPolygon(2);
        pBottomMiddleFace2->SetVertex(0, BackBottomMiddle1);
        pBottomMiddleFace2->SetVertex(1, BackBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace2);

        // Front
        CVertex FrontLeftTop1 = CVertex(fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex FrontLeftTop2 = CVertex(fHalfWidth - 1.0f, fHalfHeight, -fHalfDepth);
        CVertex FrontLeftBottom1 = CVertex(fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex FrontLeftBottom2 = CVertex(fHalfWidth - 1.0f, -fHalfHeight, -fHalfDepth);

        CVertex FrontMiddleTop1 = CVertex(-fMiddleWidth, fHalfHeight, -fMiddleDepth);
        CVertex FrontMiddleTop2 = CVertex(fMiddleWidth, fHalfHeight, -fMiddleDepth);
        CVertex FrontMiddleBottom1 = CVertex(-fMiddleWidth, -fHalfHeight, -fMiddleDepth);
        CVertex FrontMiddleBottom2 = CVertex(fMiddleWidth, -fHalfHeight, -fMiddleDepth);

        CVertex FrontRightTop1 = CVertex(-fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex FrontRightTop2 = CVertex(-fHalfWidth + 1.0f, fHalfHeight, -fHalfDepth);
        CVertex FrontRightBottom1 = CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex FrontRightBottom2 = CVertex(-fHalfWidth + 1.0f, -fHalfHeight, -fHalfDepth);

        CPolygon* pFrontLeftFace = new CPolygon(4);
        pFrontLeftFace->SetVertex(0, FrontLeftTop2);
        pFrontLeftFace->SetVertex(1, FrontLeftTop1);
        pFrontLeftFace->SetVertex(2, FrontLeftBottom1);
        pFrontLeftFace->SetVertex(3, FrontLeftBottom2);
        SetPolygon(i++, pFrontLeftFace);

        CPolygon* pFrontMiddleFace = new CPolygon(4);
        pFrontMiddleFace->SetVertex(0, FrontMiddleTop2);
        pFrontMiddleFace->SetVertex(1, FrontMiddleTop1);
        pFrontMiddleFace->SetVertex(2, FrontMiddleBottom1);
        pFrontMiddleFace->SetVertex(3, FrontMiddleBottom2);
        SetPolygon(i++, pFrontMiddleFace);

        CPolygon* pFrontRightFace = new CPolygon(4);
        pFrontRightFace->SetVertex(0, FrontRightTop2);
        pFrontRightFace->SetVertex(1, FrontRightTop1);
        pFrontRightFace->SetVertex(2, FrontRightBottom1);
        pFrontRightFace->SetVertex(3, FrontRightBottom2);
        SetPolygon(i++, pFrontRightFace);

        // Back
        CVertex BackLeftTop1 = CVertex(fHalfWidth, fHalfHeight, fHalfDepth);
        CVertex BackLeftTop2 = CVertex(fHalfWidth - 1.0f, fHalfHeight, fHalfDepth);
        CVertex BackLeftBottom1 = CVertex(fHalfWidth, -fHalfHeight, fHalfDepth);
        CVertex BackLeftBottom2 = CVertex(fHalfWidth - 1.0f, -fHalfHeight, fHalfDepth);

        CVertex BackMiddleTop1 = CVertex(-fMiddleWidth, fHalfHeight, fMiddleDepth);
        CVertex BackMiddleTop2 = CVertex(fMiddleWidth, fHalfHeight, fMiddleDepth);
        CVertex BackMiddleBottom1 = CVertex(-fMiddleWidth, -fHalfHeight, fMiddleDepth);
        CVertex BackMiddleBottom2 = CVertex(fMiddleWidth, -fHalfHeight, fMiddleDepth);

        CVertex BackRightTop1 = CVertex(-fHalfWidth, fHalfHeight, fHalfDepth);
        CVertex BackRightTop2 = CVertex(-fHalfWidth + 1.0f, fHalfHeight, fHalfDepth);
        CVertex BackRightBottom1 = CVertex(-fHalfWidth, -fHalfHeight, fHalfDepth);
        CVertex BackRightBottom2 = CVertex(-fHalfWidth + 1.0f, -fHalfHeight, fHalfDepth);

        CPolygon* pBackLeftFace = new CPolygon(4);
        pBackLeftFace->SetVertex(0, BackLeftTop1);
        pBackLeftFace->SetVertex(1, BackLeftTop2);
        pBackLeftFace->SetVertex(2, BackLeftBottom2);
        pBackLeftFace->SetVertex(3, BackLeftBottom1);
        SetPolygon(i++, pBackLeftFace);

        CPolygon* pBackMiddleFace = new CPolygon(4);
        pBackMiddleFace->SetVertex(0, BackMiddleTop1);
        pBackMiddleFace->SetVertex(1, BackMiddleTop2);
        pBackMiddleFace->SetVertex(2, BackMiddleBottom2);
        pBackMiddleFace->SetVertex(3, BackMiddleBottom1);
        SetPolygon(i++, pBackMiddleFace);

        CPolygon* pBackRightFace = new CPolygon(4);
        pBackRightFace->SetVertex(0, BackRightTop1);
        pBackRightFace->SetVertex(1, BackRightTop2);
        pBackRightFace->SetVertex(2, BackRightBottom2);
        pBackRightFace->SetVertex(3, BackRightBottom1);
        SetPolygon(i++, pBackRightFace);
    }
    else if (dir == DIR::LEFT) {
        //Top
        CVertex BackTopRight1(fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex BackTopRight2(fHalfWidth - 1.0f, fHalfHeight, -fHalfDepth );
        CVertex FrontTopRight1(fHalfWidth - fWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth);
        CVertex FrontTopRight2(fHalfWidth - 0.70f - fWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth - 0.65f);

        CVertex RightPoint1(fHalfWidth, fHalfHeight, 0.0f);
        CVertex RightPoint2(fHalfWidth - 1.0f, fHalfHeight, 0.0f);

        CPolygon* pTopRightFace = new CPolygon(102);
 
        CVertex FrontTopMiddle1;
        CVertex BackTopMiddle1;
        CVertex BackTopMiddle2;
        CVertex FrontTopMiddle2;

        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackTopRight1, RightPoint1, FrontTopRight1, (float)(i * 2 * 0.01), temp, temp);
           
            pTopRightFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontTopRight2, RightPoint2, BackTopRight2, (float)(i * 2 * 0.01), FrontTopMiddle1, BackTopMiddle1);

            pTopRightFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pTopRightFace);

        CVertex BackTopLeft1(-fHalfWidth + 1.0f, fHalfHeight, -fHalfDepth);
        CVertex BackTopLeft2(-fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex FrontTopLeft1(-fHalfWidth - fWidth * sin(DegreeToRadian(radius)) + 1.9f, fHalfHeight, fHalfDepth - 2.1f);
        CVertex FrontTopLeft2(-fHalfWidth - fWidth * sin(DegreeToRadian(radius)) + 1.2f, fHalfHeight, fHalfDepth - 2.8f);

        CVertex LeftPoint1(-fHalfWidth + 0.8f, fHalfHeight, -1.0f);
        CVertex LeftPoint2(-fHalfWidth - 0.2f, fHalfHeight, -1.0f);

        CPolygon* pTopLeftFace = new CPolygon(102);
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackTopLeft1, LeftPoint1, FrontTopLeft1, (float)(i * 2 * 0.01), BackTopMiddle2, FrontTopMiddle2);

            pTopLeftFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontTopLeft2, LeftPoint2, BackTopLeft2, (float)(i * 2 * 0.01), temp, temp);

            pTopLeftFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pTopLeftFace);
        
        CPolygon* pTopMiddleFace = new CPolygon(2);
        pTopMiddleFace->SetVertex(0, FrontTopMiddle1);
        pTopMiddleFace->SetVertex(1, FrontTopMiddle2);
        SetPolygon(i++, pTopMiddleFace);

        CPolygon* pTopMiddleFace2 = new CPolygon(2);
        pTopMiddleFace2->SetVertex(0, BackTopMiddle1);
        pTopMiddleFace2->SetVertex(1, BackTopMiddle2);
        SetPolygon(i++, pTopMiddleFace2);

        //Bottom
        CVertex BackBottomRight1(fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomRight2(fHalfWidth - 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomRight1(fHalfWidth - fWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth);
        CVertex FrontBottomRight2(fHalfWidth - 0.70f - fWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth - 0.65f);

        CVertex RightPoint3(fHalfWidth, -fHalfHeight, 0.0f);
        CVertex RightPoint4(fHalfWidth - 1.0f, -fHalfHeight, 0.0f);

        CPolygon* pBottomRightFace = new CPolygon(102);

        CVertex FrontBottomMiddle1;
        CVertex BackBottomMiddle1;
        CVertex BackBottomMiddle2;
        CVertex FrontBottomMiddle2;

        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackBottomRight1, RightPoint3, FrontBottomRight1, (float)(i * 2 * 0.01), temp, temp);

            pBottomRightFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontBottomRight2, RightPoint4, BackBottomRight2, (float)(i * 2 * 0.01), FrontBottomMiddle1, BackBottomMiddle1);

            pBottomRightFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pBottomRightFace);

        CVertex BackBottomLeft1(-fHalfWidth + 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomLeft2(-fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomLeft1(-fHalfWidth - fWidth * sin(DegreeToRadian(radius)) + 1.9f, -fHalfHeight, fHalfDepth - 2.1f);
        CVertex FrontBottomLeft2(-fHalfWidth - fWidth * sin(DegreeToRadian(radius)) + 1.2f, -fHalfHeight, fHalfDepth - 2.8f);

        CVertex LeftPoint3(-fHalfWidth + 0.8f, -fHalfHeight, -1.0f);
        CVertex LeftPoint4(-fHalfWidth - 0.2f, -fHalfHeight, -1.0f);

        CPolygon* pBottomLeftFace = new CPolygon(102);
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackBottomLeft1, LeftPoint3, FrontBottomLeft1, (float)(i * 2 * 0.01), BackBottomMiddle2, FrontBottomMiddle2);

            pBottomLeftFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontBottomLeft2, LeftPoint4, BackBottomLeft2, (float)(i * 2 * 0.01), temp, temp);
            
            pBottomLeftFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pBottomLeftFace);

        CPolygon* pBottomMiddleFace = new CPolygon(2);
        pBottomMiddleFace->SetVertex(0, FrontBottomMiddle1);
        pBottomMiddleFace->SetVertex(1, FrontBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace);

        CPolygon* pBottomMiddleFace2 = new CPolygon(2);
        pBottomMiddleFace2->SetVertex(0, BackBottomMiddle1);
        pBottomMiddleFace2->SetVertex(1, BackBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace2);

        //Front
        CPolygon* pFrontLeftFace = new CPolygon(4);
        pFrontLeftFace->SetVertex(0, FrontBottomLeft1);
        pFrontLeftFace->SetVertex(1, FrontTopLeft1);
        pFrontLeftFace->SetVertex(2, FrontTopLeft2);
        pFrontLeftFace->SetVertex(3, FrontBottomLeft2);
        SetPolygon(i++, pFrontLeftFace);

        CPolygon* pFrontRightFace = new CPolygon(4);
        pFrontRightFace->SetVertex(0, FrontBottomRight1);
        pFrontRightFace->SetVertex(1, FrontTopRight1);
        pFrontRightFace->SetVertex(2, FrontTopRight2);
        pFrontRightFace->SetVertex(3, FrontBottomRight2);
        SetPolygon(i++, pFrontRightFace);

        CPolygon* pFrontMiddleFace = new CPolygon(4);
        pFrontMiddleFace->SetVertex(0, FrontBottomMiddle2);
        pFrontMiddleFace->SetVertex(1, FrontTopMiddle2);
        pFrontMiddleFace->SetVertex(2, FrontTopMiddle1);
        pFrontMiddleFace->SetVertex(3, FrontBottomMiddle1);
        SetPolygon(i++, pFrontMiddleFace);


        //Back
        CPolygon* pBackLeftFace = new CPolygon(4);
        pBackLeftFace->SetVertex(0, BackBottomLeft1);
        pBackLeftFace->SetVertex(1, BackTopLeft1);
        pBackLeftFace->SetVertex(2, BackTopLeft2);
        pBackLeftFace->SetVertex(3, BackBottomLeft2);
        SetPolygon(i++, pBackLeftFace);

        CPolygon* pBackRightFace = new CPolygon(4);
        pBackRightFace->SetVertex(0, BackBottomRight1);
        pBackRightFace->SetVertex(1, BackTopRight1);
        pBackRightFace->SetVertex(2, BackTopRight2);
        pBackRightFace->SetVertex(3, BackBottomRight2);
        SetPolygon(i++, pBackRightFace);

        CPolygon* pBackMiddleFace = new CPolygon(4);
        pBackMiddleFace->SetVertex(0, BackBottomMiddle2);
        pBackMiddleFace->SetVertex(1, BackTopMiddle2);
        pBackMiddleFace->SetVertex(2, BackTopMiddle1);
        pBackMiddleFace->SetVertex(3, BackBottomMiddle1);
        SetPolygon(i++, pBackMiddleFace);
        
    }
    else if (dir == DIR::RIGHT) {
        //Top
        CVertex BackTopRight1(fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex BackTopRight2(fHalfWidth - 1.0f, fHalfHeight, -fHalfDepth);
        CVertex FrontTopRight1(fHalfWidth + fHalfWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth);
        CVertex FrontTopRight2(fHalfWidth - 1.0f + fHalfWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth);

        CVertex RightPoint1(fHalfWidth, fHalfHeight, 0.0f);
        CVertex RightPoint2(fHalfWidth - 1.0f, fHalfHeight, 0.0f);

        CPolygon* pTopRightFace = new CPolygon(102);

        CVertex FrontTopMiddle1;
        CVertex BackTopMiddle1;
        CVertex BackTopMiddle2;
        CVertex FrontTopMiddle2;

        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackTopRight1, RightPoint1, FrontTopRight1, (float)(i * 2 * 0.01), temp, temp);

            pTopRightFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontTopRight2, RightPoint2, BackTopRight2, (float)(i * 2 * 0.01), FrontTopMiddle1, BackTopMiddle1);

            pTopRightFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pTopRightFace);

        CVertex BackTopLeft1(-fHalfWidth + 1.0f, fHalfHeight, -fHalfDepth);
        CVertex BackTopLeft2(-fHalfWidth, fHalfHeight, -fHalfDepth);
        CVertex FrontTopLeft1(-fHalfWidth + 1.0f + fHalfWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth);
        CVertex FrontTopLeft2(-fHalfWidth + fHalfWidth * sin(DegreeToRadian(radius)), fHalfHeight, fHalfDepth);

        CVertex LeftPoint1(-fHalfWidth + 1.0f, fHalfHeight, 0.0f);
        CVertex LeftPoint2(-fHalfWidth, fHalfHeight, 0.0f);

        CPolygon* pTopLeftFace = new CPolygon(102);
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackTopLeft1, LeftPoint1, FrontTopLeft1, (float)(i * 2 * 0.01), BackTopMiddle2, FrontTopMiddle2);

            pTopLeftFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontTopLeft2, LeftPoint2, BackTopLeft2, (float)(i * 2 * 0.01), temp, temp);

            pTopLeftFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pTopLeftFace);

        CPolygon* pTopMiddleFace = new CPolygon(2);
        pTopMiddleFace->SetVertex(0, FrontTopMiddle1);
        pTopMiddleFace->SetVertex(1, FrontTopMiddle2);
        SetPolygon(i++, pTopMiddleFace);

        CPolygon* pTopMiddleFace2 = new CPolygon(2);
        pTopMiddleFace2->SetVertex(0, BackTopMiddle1);
        pTopMiddleFace2->SetVertex(1, BackTopMiddle2);
        SetPolygon(i++, pTopMiddleFace2);

        //Bottom
        CVertex BackBottomRight1(fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomRight2(fHalfWidth - 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomRight1(fHalfWidth + fHalfWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth);
        CVertex FrontBottomRight2(fHalfWidth - 1.0f + fHalfWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth);

        CVertex RightPoint3(fHalfWidth, -fHalfHeight, 0.0f);
        CVertex RightPoint4(fHalfWidth - 1.0f, -fHalfHeight, 0.0f);

        CPolygon* pBottomRightFace = new CPolygon(102);

        CVertex FrontBottomMiddle1;
        CVertex BackBottomMiddle1;
        CVertex BackBottomMiddle2;
        CVertex FrontBottomMiddle2;

        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackBottomRight1, RightPoint3, FrontBottomRight1, (float)(i * 2 * 0.01), temp, temp);

            pBottomRightFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontBottomRight2, RightPoint4, BackBottomRight2, (float)(i * 2 * 0.01), FrontBottomMiddle1, BackBottomMiddle1);

            pBottomRightFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pBottomRightFace);

        CVertex BackBottomLeft1(-fHalfWidth + 1.0f, -fHalfHeight, -fHalfDepth);
        CVertex BackBottomLeft2(-fHalfWidth, -fHalfHeight, -fHalfDepth);
        CVertex FrontBottomLeft1(-fHalfWidth + 1.0f + fHalfWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth);
        CVertex FrontBottomLeft2(-fHalfWidth + fHalfWidth * sin(DegreeToRadian(radius)), -fHalfHeight, fHalfDepth);

        CVertex LeftPoint3(-fHalfWidth + 1.0f, -fHalfHeight, 0.0f);
        CVertex LeftPoint4(-fHalfWidth, -fHalfHeight, 0.0f);

        CPolygon* pBottomLeftFace = new CPolygon(102);
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(BackBottomLeft1, LeftPoint3, FrontBottomLeft1, (float)(i * 2 * 0.01), BackBottomMiddle2, FrontBottomMiddle2);

            pBottomLeftFace->SetVertex(i, Point);
        }
        for (int i = 0; i < 51; ++i) {
            CVertex Point = GetBezier(FrontBottomLeft2, LeftPoint4, BackBottomLeft2, (float)(i * 2 * 0.01), temp, temp);

            pBottomLeftFace->SetVertex(i + 51, Point);
        }
        SetPolygon(i++, pBottomLeftFace);

        CPolygon* pBottomMiddleFace = new CPolygon(2);
        pBottomMiddleFace->SetVertex(0, FrontBottomMiddle1);
        pBottomMiddleFace->SetVertex(1, FrontBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace);

        CPolygon* pBottomMiddleFace2 = new CPolygon(2);
        pBottomMiddleFace2->SetVertex(0, BackBottomMiddle1);
        pBottomMiddleFace2->SetVertex(1, BackBottomMiddle2);
        SetPolygon(i++, pBottomMiddleFace2);

        //Front
        CPolygon* pFrontLeftFace = new CPolygon(4);
        pFrontLeftFace->SetVertex(0, FrontBottomLeft1);
        pFrontLeftFace->SetVertex(1, FrontTopLeft1);
        pFrontLeftFace->SetVertex(2, FrontTopLeft2);
        pFrontLeftFace->SetVertex(3, FrontBottomLeft2);
        SetPolygon(i++, pFrontLeftFace);

        CPolygon* pFrontRightFace = new CPolygon(4);
        pFrontRightFace->SetVertex(0, FrontBottomRight1);
        pFrontRightFace->SetVertex(1, FrontTopRight1);
        pFrontRightFace->SetVertex(2, FrontTopRight2);
        pFrontRightFace->SetVertex(3, FrontBottomRight2);
        SetPolygon(i++, pFrontRightFace);

        CPolygon* pFrontMiddleFace = new CPolygon(4);
        pFrontMiddleFace->SetVertex(0, FrontBottomMiddle2);
        pFrontMiddleFace->SetVertex(1, FrontTopMiddle2);
        pFrontMiddleFace->SetVertex(2, FrontTopMiddle1);
        pFrontMiddleFace->SetVertex(3, FrontBottomMiddle1);
        SetPolygon(i++, pFrontMiddleFace);


        //Back
        CPolygon* pBackLeftFace = new CPolygon(4);
        pBackLeftFace->SetVertex(0, BackBottomLeft1);
        pBackLeftFace->SetVertex(1, BackTopLeft1);
        pBackLeftFace->SetVertex(2, BackTopLeft2);
        pBackLeftFace->SetVertex(3, BackBottomLeft2);
        SetPolygon(i++, pBackLeftFace);

        CPolygon* pBackRightFace = new CPolygon(4);
        pBackRightFace->SetVertex(0, BackBottomRight1);
        pBackRightFace->SetVertex(1, BackTopRight1);
        pBackRightFace->SetVertex(2, BackTopRight2);
        pBackRightFace->SetVertex(3, BackBottomRight2);
        SetPolygon(i++, pBackRightFace);

        CPolygon* pBackMiddleFace = new CPolygon(4);
        pBackMiddleFace->SetVertex(0, BackBottomMiddle2);
        pBackMiddleFace->SetVertex(1, BackTopMiddle2);
        pBackMiddleFace->SetVertex(2, BackTopMiddle1);
        pBackMiddleFace->SetVertex(3, BackBottomMiddle1);
        SetPolygon(i++, pBackMiddleFace);

    }

    m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

inline CVertex GetBezier(const CVertex& p1, const CVertex& p2, const CVertex& p3, const float fT, CVertex& middle1, CVertex& middle2)
{
    CVertex temp;
    float c1 = (1 - fT) * (1 - fT);
    float c2 = 2 * (1 - fT) * fT;
    float c3 = fT * fT;

    temp.SetPos(c1 * p1.GetPos().x + c2 * p2.GetPos().x + c3 * p3.GetPos().x, 
                c1 * p1.GetPos().y + c2 * p2.GetPos().y + c3 * p3.GetPos().y,
                c1 * p1.GetPos().z + c2 * p2.GetPos().z + c3 * p3.GetPos().z);

    if (IsEqual(fT, 0.24f)) {
        middle1 = temp;
    }
    else if (IsEqual(fT, 0.48f)) {
        middle2 = temp;
    }
    return temp;
}
