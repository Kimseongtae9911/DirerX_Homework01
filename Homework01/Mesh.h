#pragma once

class CVertex
{
private:
	XMFLOAT3 m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	virtual ~CVertex() {}

public:
	XMFLOAT3& GetPos() { return m_xmf3Position; }
};

class CPolygon
{
private:
	int m_nVertices = 0;
	std::unique_ptr<CVertex[]> m_pVertices = nullptr;

public:
	CPolygon() {}
	CPolygon(int nVertices);
	virtual ~CPolygon();

	void SetVertex(int nIndex, CVertex& vertex);
	CVertex* GetVertex() { return m_pVertices.get(); }
	int& GetVertexNum() { return m_nVertices; }
};

class CMesh
{
private:
	int m_nReferences = 1;

protected :
	int m_nPolygons = 0;
	//CPolygon** m_ppPolygons = NULL;
	std::shared_ptr<CPolygon* []> m_ppPolygons = NULL;

public:
	BoundingOrientedBox m_xmOOBB = BoundingOrientedBox();

public:
	CMesh() {}
	CMesh(int nPolygons);
	virtual ~CMesh();

public:
	void AddRef() { m_nReferences++; }
	void Release() { m_nReferences--; if (m_nReferences <= 0) delete this; }

public:
	void SetPolygon(int nIndex, std::unique_ptr<CPolygon[]> pPolygon);

	virtual void Render(HDC hDCFrameBuffer);

	BOOL RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance);
	int CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance);
};

