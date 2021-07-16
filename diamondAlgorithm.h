#ifndef _DIAMOND_H
#define _DIAMOND_H

#pragma once

#include "D3DObject.h"
#include "CTerrain.h"

class CCamera;
class CDiamondLOD;
class TDiamond;
class CTerrain;

//#define mTris 25000
//#define mapSize 1024
#define mapBase 10

#define TRIANGLE_INVISIBLE 1
#define TRIANGLE_VISIBLE 2

//#define targetPriority 0.125f
#define stargetPriority 0.1000f

#define con1 1
#define con2 2
#define con3 4

#define cn12 con1 | con2
#define cn13 con1 | con3
#define cn23 con2 | con3
#define cAll con1 | con2 | con3

#define prMax 9999999
#define prMin 0
#define prNeg -prMax

struct TCustomVertex
{            
	float X, Y, Z;    // position for the vertex
	DWORD color;
	float U, V;    // Texture coordinates
	float dU, dV;    // Detail Texture coordinates
};

#define D3DFVF_CUSTOMVERTEX  D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2

struct TVert
{
	real x, y, z;
};

struct TNode
{
	TDiamond *tri;
	TNode *prev, *next;
};

struct TPos : public Vector
{
	int clip;
	unsigned int lupdate;
	bool visible;
};

struct TQueue
{
	TNode first, guard;
};

class TDiamond
{
public:
	TDiamond *n1, *n2, *n3;
	TDiamond *childs[4];
	//TDiamond *a, *b, *c, *d;
	TDiamond *parent;
	CDiamondLOD *dlod;

	TPos *v1, *v2, *v3;

	real priority;
	TNode *splitQ, *mergeQ;

	int varindex, clipcode;
	Smart::Array<float> *variance;

	bool canMerge() const;
	void CullFrustum();
	inline void CalculatePriority(const Vector *);
	void SplitEnqueue(const real, const Vector *, TQueue *, TQueue *);
	void CullFrustum(const CCamera *cam);
};

extern unsigned int freeTriSP, freePosSP, freeSplit, freeMerge;

extern Smart::Array<TDiamond*> TriStack;
extern Smart::Array<TDiamond> TriPool;
extern Smart::Array<TPos*> PosStack;
extern Smart::Array<TPos> PosPool;
extern Smart::Array<TNode*> SplitStack, MergeStack;
extern Smart::Array<TNode> SplitPool, MergePool;

class CDiamondLOD {
	Smart::Array<TCustomVertex> _vb;
	Smart::Array<unsigned short> _ib;
	LPDIRECT3DVERTEXBUFFER9 vb_RM;
	LPDIRECT3DINDEXBUFFER9 ib_RM;
	real AngleXY, AngleXZ;
	const CCamera *cam;

	TQueue SplitAbove, SplitBelow, MergeAbove, MergeBelow;

	Smart::Array<float> Variance1, Variance2;
	Smart::Array<float> *varCurrent;
	int varstop;

	TDiamond base1, base2;
	TPos basev1, basev2, basev3, basev4;
	bool ResetDiamond;

	inline TCustomVertex &GetCustomVertex(const Vector &_p);
	inline TCustomVertex &GetCustomVertex(const Vector *_p)
	{ return GetCustomVertex(*_p); }

	void InitMesh();
	float ComputeVariance(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const int index);
	void InitVariance();
	void InitQueues();
	void ResetMesh();
	void SplitRequeue();
	void MergeRequeue();
	void Merge(TDiamond *tri);
	void MakeChildren(TDiamond *tri);
	void Split(TDiamond *tri);
	void RenderMesh();
	void DrawScapeDiamond();
	void ClearTrash();
	void RenderMesh0();
	void RenderMesh1();
public:
	int glTri;
	int glInd, varsize;
	real targetPriority;
	unsigned int _maxTris;
	CTerrain *tentity;

	const CCamera *GetCamera();
	void SetCamera(const CCamera *);

	CDiamondLOD(CTerrain *);
	~CDiamondLOD();

	void Setup(const unsigned int);
	HRESULT OnLostDevice();
	HRESULT OnResetDevice();
	void InitScapeDiamond();
	void DoDiamond();
};

#endif