#pragma once

#include "GerenciadorDeRecursos.h"
#include "VideoResource.h"

using Matematica::Vector;

class LineRender : public CResource, public VideoResource
{
	typedef struct tagLVERTEX
	{
		Vector coord;
		D3DCOLOR color;
	} LVERTEX;

	struct Line {
		Vector start, end;
		D3DCOLOR color;
	};

	#define LVERTEXFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
	unsigned int VBLength;
	std::stack<Line> lines;
public:
	LineRender(void);
	~LineRender(void);
	HRESULT OnLostDevice(void);
	HRESULT OnResetDevice(void);
	void DrawLine(const Vector &start, const Vector &end, const D3DCOLOR cor);
	inline HRESULT RenderLine(const Vector &start, const Vector &end, const D3DCOLOR cor) {
		DrawLine(start, end, cor);
		return Render();
	}
	HRESULT Render();
};
