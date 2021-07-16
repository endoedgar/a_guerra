//-----------------------------------------------------------------------------
// Name: CMesh.h
// Desc: Declaration for CMesh class
//-----------------------------------------------------------------------------

#ifndef CMESH_H
#define CMESH_H

#include "ExternalResource.h"
#include "CVisibleEntity.h"
#include "3DTexture.h"

/*#define DIAa

#ifdef DIA
#define GG_LIGHT_R 255
#define GG_LIGHT_G 255
#define GG_LIGHT_B 230
#else
#define GG_LIGHT_R 100
#define GG_LIGHT_G 100
#define GG_LIGHT_B 150
#endif

#define GG_LIGHT_R_R (GG_LIGHT_R/255.0f)
#define GG_LIGHT_G_R (GG_LIGHT_G/255.0f)
#define GG_LIGHT_B_R (GG_LIGHT_B/255.0f)*/

//-------------------------------------------------------------------------
// Name: class CMesh
// Desc: Loads and renders meshes
//-------------------------------------------------------------------------
class CMesh : public virtual ExternalResource, public virtual CVisibleEntity
{
	// Variables
	DWORD			    numMaterials;	// Number of materials
	static const DWORD  MESHFVF;

	//Smart::String		filename;
	//Smart::String		path;

	DWORD				numFaces;		// Number of faces
	DWORD				numVerts;		// Number of vertices

	// Objects
	D3DMATERIAL9*		meshMaterials;	// Materials for mesh
	CTexture** meshTextures;	// Textures for mesh
	LPD3DXMESH			meshObject;		// D3DX Mesh object

	CMesh();
	~CMesh();
	CMesh *_Load(const Smart::String &filename);

	HRESULT Cleanup();
	Vector scaled, centerDisplacement;
	bool centered;
public:
	
	const Vector &getCenterDisplacement() const;
	DWORD GetNumFaces() { return numFaces; }
	DWORD GetNumVerts() { return numVerts; }
	DWORD GetNumMaterials() { return numMaterials; }

	inline D3DMATERIAL9 &getMaterial(const unsigned int i)
	{ return this->meshMaterials[i]; }

	HRESULT OnLostDevice();
	HRESULT OnResetDevice();

	// Functions

	CMesh *Load(const Smart::String &filename);
	inline LPD3DXMESH getMeshObject(void) const {
		return this->meshObject;
	}

	bool GetMeshMinMaxDimensions(Vector *vMinOut, Vector *vMaxOut) const
	{
		D3DXVECTOR3 *Vertex = NULL;
		if(SUCCEEDED(this->meshObject->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<LPVOID*>(&Vertex)))) {
			D3DXVECTOR3 pMin, pMax;
			if(SUCCEEDED(D3DXComputeBoundingBox(Vertex, this->meshObject->GetNumVertices(), this->meshObject->GetNumBytesPerVertex(), &pMin, &pMax))) {
				vMinOut->x = pMin.x;
				vMinOut->y = pMin.y;
				vMinOut->z = pMin.z;
				vMaxOut->x = pMax.x;
				vMaxOut->y = pMax.y;
				vMaxOut->z = pMax.z;
				this->meshObject->UnlockVertexBuffer();
				return true;
			}
			this->meshObject->UnlockVertexBuffer();
		}
		return false;
	}

	Vector *GetMeshDimensions(Vector *vOut) const
	{
		D3DXVECTOR3 *Vertex = NULL;
		if(SUCCEEDED(this->meshObject->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<LPVOID*>(&Vertex)))) {
			D3DXVECTOR3 pMin, pMax;
			if(SUCCEEDED(D3DXComputeBoundingBox(Vertex, this->meshObject->GetNumVertices(), this->meshObject->GetNumBytesPerVertex(), &pMin, &pMax))) {
				vOut->x = pMax.x+pMin.x;
				vOut->y = pMax.y+pMin.y;
				vOut->z = pMax.z+pMin.z;
				this->meshObject->UnlockVertexBuffer();
				return vOut;
			}
			this->meshObject->UnlockVertexBuffer();
		}
		return NULL;
	}

	HRESULT center();

	HRESULT Translate(const Vector *v) {
		HRESULT hr;
		D3DXVECTOR3 *Vertex = NULL;

		hr = this->meshObject->LockVertexBuffer(0, reinterpret_cast<LPVOID*>(&Vertex));
		if(SUCCEEDED(hr)) {
			for(register size_t a = 0; a < this->meshObject->GetNumVertices(); ++a) {
				Vertex->x += v->x;
				Vertex->y += v->y;
				Vertex->z += v->z;
				Vertex = (D3DXVECTOR3*)((unsigned char*)Vertex + this->meshObject->GetNumBytesPerVertex());
			}
			hr = this->meshObject->UnlockVertexBuffer();
		}
		return hr;
	}

	HRESULT Scale(const Vector *s)
	{
		HRESULT hr;
		D3DXVECTOR3 *Vertex = NULL;

		hr = this->meshObject->LockVertexBuffer(0, reinterpret_cast<LPVOID*>(&Vertex));
		if(SUCCEEDED(hr)) {
			for(register size_t a = 0; a < this->meshObject->GetNumVertices(); ++a) {
				Vertex->x *= s->x;
				Vertex->y *= s->y;
				Vertex->z *= s->z;
				Vertex = (D3DXVECTOR3*)((unsigned char*)Vertex + this->meshObject->GetNumBytesPerVertex());
			}
			hr = this->meshObject->UnlockVertexBuffer();
			this->scaled = *s;
		}
		return hr;
	}

	HRESULT Release() {
		return ExternalResource::Release();
	}

	const CTexture *GetTexture(const size_t i) const
	{
		ASSERT(i < numMaterials)
		return this->meshTextures[i];
	}

	HRESULT Texture(CTexture *tex);
	virtual HRESULT Render();
	virtual HRESULT RenderWM();
};


#endif