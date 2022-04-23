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

//void CPolygon::SetVertex(int nIndex, CVertex vertex)
//{
//    if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
//        m_pVertices[nIndex] = vertex;
//}

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

CBackGroundMesh::CBackGroundMesh(float fWidth, float fHeight, float fDepth, int nSubRects)
{
   /* float fHalfWidth = fWidth * 0.5f;
    float fHalfHeight = fHeight * 0.5f;
    float fHalfDepth = fDepth * 0.5f;
    float fCellWidth = fWidth * (1.0f / nSubRects);
    float fCellHeight = fHeight * (1.0f / nSubRects);
    float fCellDepth = fDepth * (1.0f / nSubRects);

    int k = 0;
    CPolygon* pLeftFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            pLeftFace = new CPolygon(4);
            pLeftFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
            pLeftFace->SetVertex(1, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
            pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
            pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
            SetPolygon(k++, pLeftFace);
        }
    }

    CPolygon* pRightFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            pRightFace = new CPolygon(4);
            pRightFace->SetVertex(0, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
            pRightFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
            pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
            pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
            SetPolygon(k++, pRightFace);
        }
    }

    CPolygon* pTopFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            pTopFace = new CPolygon(4);
            pTopFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
            pTopFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
            pTopFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
            pTopFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
            SetPolygon(k++, pTopFace);
        }
    }

    CPolygon* pBottomFace;
    for (int i = 0; i < nSubRects; i++)
    {
        for (int j = 0; j < nSubRects; j++)
        {
            pBottomFace = new CPolygon(4);
            pBottomFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
            pBottomFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
            pBottomFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
            pBottomFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
            SetPolygon(k++, pBottomFace);
        }
    }

    CPolygon* pFrontFace = new CPolygon(4);
    pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
    pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
    pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
    pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
    SetPolygon(k++, pFrontFace);

    CPolygon* pBackFace = new CPolygon(4);
    pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
    pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
    pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
    pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
    SetPolygon(k++, pBackFace);

    m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));*/
}
