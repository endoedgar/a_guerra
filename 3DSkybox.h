#ifndef _3DSKYBOX_H
#define _3DSKYBOX_H

#include "D3DObject.h"
#include "3DCamera.h"
#include "3DTexture.h"

typedef struct tagSBVERTEX
{
	D3DVECTOR coord;
	float tu, tv;
} SBVERTEX;
const DWORD SBVERTEXFVF = ( D3DFVF_XYZ | D3DFVF_TEX1 );

class CSkybox : public CResource, public VideoResource
{
	CTexture **skytex;
	LPDIRECT3DVERTEXBUFFER9 VB;
	LPDIRECT3DINDEXBUFFER9 IB;
	HRESULT RotinaRepetitiva(const Smart::String &filename, const Smart::String &sufix, const Smart::String &end, CTexture **skytex);
public:

	CSkybox();
	HRESULT Load(const Smart::String &filename, const Smart::String &end = ".jpg");
	void UnloadTexture();
	HRESULT OnLostDevice();
	HRESULT OnResetDevice();
	HRESULT Render(const CCamera *camera);
	~CSkybox();
};

#endif