#include "pch.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

CPolygon::CPolygon(int nVertices)
{
    m_nVertices = nVertices;
    m_pVertices = std::make_unique<CVertex[]>(nVertices);
}

CPolygon::~CPolygon()
{
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
    if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
        m_pVertices.get()[nIndex] = vertex;
}

CMesh::CMesh(int nPolygons)
{
    m_nPolygons = nPolygons;
    //m_ppPolygons = new CPolygon * [nPolygons];
    m_ppPolygons = std::make_shared<CPolygon* []>(nPolygons);
}

CMesh::~CMesh()
{
    if (m_ppPolygons)
    {
        m_ppPolygons.reset();
    }
}

void CMesh::SetPolygon(int nIndex, std::unique_ptr<CPolygon[]> pPolygon)
{
    if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
        m_ppPolygons[nIndex] = pPolygon.get();
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
