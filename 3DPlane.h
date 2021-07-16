#ifndef _3DPLANE_H
#define _3DPLANE_H

#include "D3DObject.h"
#include "3DEntity.h"
#include "3DCamera.h"

typedef struct tagSPLANEVERTEX {
	D3DXVECTOR3 pos;
	D3DCOLOR color;
	D3DXVECTOR2 tex;
} SPLANEVERTEX;

#define PLANEFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

class CPlane : public Entity, public VideoResource, public CResource
{
	HRESULT Unload();
private:
	LPDIRECT3DVERTEXBUFFER9 vb_RM;
	D3DXVECTOR3 planePos;

public:
	CPlane();
	~CPlane();

	HRESULT OnLostDevice();
	HRESULT OnResetDevice();

	HRESULT Render(const CCamera *cam);
};

#endif