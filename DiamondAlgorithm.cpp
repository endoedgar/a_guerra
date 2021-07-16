#include "stdafx.h"
#include "3DCamera.h"
#include "GameGlobals.h"
#include "CTerrain.h"
#include "diamondAlgorithm.h"
#include "GenericFunctions.h"
#include "globalheader.h"
#include "CMesh.h"

// TODO: Terrain Vertex Morphing
unsigned int renderCount = 0;

unsigned int freeTriSP, freePosSP, freeSplit, freeMerge;
Smart::Array<TDiamond*> TriStack;
Smart::Array<TDiamond> TriPool;
Smart::Array<TPos*> PosStack;
Smart::Array<TPos> PosPool;
Smart::Array<TNode*> SplitStack, MergeStack;
Smart::Array<TNode> SplitPool, MergePool;

// OK

#define GetTerrain(x, z) \
	terrain->tdata[x][z]
/*inline int GetTerrain(const int x, const int z)
{
	return terrain->tdata[x][z];
}*/

TDiamond *newTri(TDiamond *tri, const int index, CDiamondLOD *lod) 
{
	TDiamond *Result = NULL;;
	if(freeTriSP == 0)
	{
		return NULL;
	}
	else {
		--freeTriSP;
		Result = TriStack[freeTriSP];
		Result->parent = tri;
		Result->childs[0] = NULL;
		Result->childs[1] = NULL;
		Result->childs[2] = NULL;
		Result->childs[3] = NULL;
		Result->splitQ = NULL;
		Result->mergeQ = NULL;
		Result->variance = tri->variance;
		Result->varindex = (tri->varindex << 2) + index;
		Result->dlod = lod;
	}
	return Result;
}

inline void deleteTri(TDiamond *tri)
{
	if(tri){
		TriStack[freeTriSP] = tri;
		++freeTriSP;
	}
}


TPos *newPos(TPos *pos1, TPos *pos2)
{
	TPos *Result;
	if(freePosSP == 0)
		return NULL;
	else {
		--freePosSP;
		Result = PosStack[freePosSP];
		Result->x = (pos1->x + pos2->x)*r(0.5);
		Result->y = (pos1->y + pos2->y)*r(0.5);
		Result->z = r(GetTerrain(static_cast<int>(Result->x*1023), static_cast<int>(Result->y*1023)));
		if(renderCount % 2 == 0)
			Result->clip = -1;
		else
			Result->clip = 1;
		//Result->lupdate = 0;
	}
	return Result;
}

inline void deletePos(TPos *ps)
{
	if(ps) {
		PosStack[freePosSP] = ps;
		++freePosSP;
	}
}

TNode *newSplit(TDiamond *tri)
{
	TNode *Result;
	if(freeSplit == 0)
		return NULL;
	else {
		--freeSplit;
		Result = SplitStack[freeSplit];
		Result->tri = tri;
	}
	return Result;
}

inline void deleteSplit(TNode *node)
{
	if(node) {
		SplitStack[freeSplit] = node;
		++freeSplit;
	}
}

TNode *newMerge(TDiamond *tri)
{
	TNode *Result;
	if(freeMerge == 0)
		return NULL;
	else {
		--freeMerge;
		Result = MergeStack[freeMerge];
		Result->tri = tri;
	}
	return Result;
}

inline void deleteMerge(TNode *node)
{
	if(node) {
		MergeStack[freeMerge] = node;
		++freeMerge;
	}
}

void deletePools()
{
	TRACE("Liberando Pools...\n");

	freeTriSP = 0;
	freePosSP = 0;
	freeSplit = 0;
	freeMerge = 0;
}

void AllocPools(const unsigned int maxTris)
{
	TRACE("Alocando Pools (%d)...\n", maxTris);
	deletePools();

	TriStack.SetSize(maxTris);
	TriPool.SetSize(maxTris);
	PosStack.SetSize(maxTris);
	PosPool.SetSize(maxTris);
	SplitStack.SetSize(maxTris);
	SplitPool.SetSize(maxTris);
	MergeStack.SetSize(maxTris);
	MergePool.SetSize(maxTris);

	TRACE("Pools alocados!\n");
	/*ZeroMemory(TriStack, sizeof(TDiamond*)*maxTris);
	ZeroMemory(TriPool, sizeof(TDiamond)*maxTris);
	ZeroMemory(PosStack, sizeof(TPos*)*maxTris);
	ZeroMemory(PosPool, sizeof(TPos)*maxTris);
	ZeroMemory(SplitStack, sizeof(TNode*)*maxTris);
	ZeroMemory(SplitPool, sizeof(TNode)*maxTris);
	ZeroMemory(MergeStack, sizeof(TNode*)*maxTris);
	ZeroMemory(MergePool, sizeof(TNode)*maxTris);*/
}

void InitPools(const unsigned int maxTris)
{
	freeTriSP = maxTris;
	freePosSP = maxTris;
	freeSplit = maxTris;
	freeMerge = maxTris;

	for(register unsigned int i = 0; i < maxTris; ++i) {
		TriStack[i] = &TriPool[i];
		PosStack[i] = &PosPool[i];
		SplitStack[i] = &SplitPool[i];
		MergeStack[i] = &MergePool[i];
	}
}

void CDiamondLOD::ClearTrash()
{
	this->Variance1.DestroyData();
	this->Variance2.DestroyData();
	this->_ib.DestroyData();
	this->_vb.DestroyData();
}

void CDiamondLOD::Setup(const unsigned int mS)
{
	this->ClearTrash();
	this->varsize = mS * mS / 4 - 1;
	this->Variance1.SetSize(this->varsize);
	this->Variance2.SetSize(this->varsize);
	this->_vb.SetSize(this->_maxTris);
	this->_ib.SetSize(this->_maxTris*3);
	renderCount = 0;
}

inline TCustomVertex &CDiamondLOD::GetCustomVertex(const Vector &_p)
{
	static TCustomVertex Result;
	Result.X = _p.x;
	Result.Y = _p.z;
	Result.Z = _p.y;
	Result.color = D3DCOLOR_RGBA(mywrapper->AmbientLightR, mywrapper->AmbientLightG, mywrapper->AmbientLightB, 255);
	Result.U = _p.x;
	Result.V = _p.y;
	Result.dU = _p.x*900;
	Result.dV = _p.y*900;
	return Result;
}

CDiamondLOD::CDiamondLOD(CTerrain *e):tentity(e)
{
	this->targetPriority = gc->GetTerrainPriority();
	this->_maxTris = gc->GetMaxTerrainTri();
	this->Setup(this->tentity->width);
}

CDiamondLOD::~CDiamondLOD()
{
	ClearTrash();
	deletePools();
}

void CDiamondLOD::InitMesh()
{
	base1.n3 = &base2;
	base2.n1 = &base1;

	base1.parent = NULL;
	base1.childs[0] = NULL;
	base1.childs[1] = NULL;
	base1.childs[2] = NULL;
	base1.childs[3] = NULL;
	base1.dlod = this;
	base2.parent = NULL;
	base2.childs[0] = NULL;
	base2.childs[1] = NULL;
	base2.childs[2] = NULL;
	base2.childs[3] = NULL;
	base2.dlod = this;
	
	base1.n1 = NULL;
	base1.n2 = NULL;
	base2.n2 = NULL;
	base2.n3 = NULL;

	base1.variance = Variance1.GetAddress();
	base1.varindex = 0;
	base2.variance = Variance2.GetAddress();
	base2.varindex = 0;

	base1.v1 = &basev1;
	base1.v2 = &basev2;
	base1.v3 = &basev3;
	base2.v1 = &basev2;
	base2.v2 = &basev3;
	base2.v3 = &basev4;

	base1.v1->x = real(0.0);
	base1.v1->y = real(0.0);
	base1.v1->z = real(GetTerrain(0, 0));

	base1.v2->x = real(0.0);
	base1.v2->y = real(1);
	base1.v2->z = real(GetTerrain(0, this->tentity->width-1));

	base1.v3->x = real(1);
	base1.v3->y = real(0.0);
	base1.v3->z = real(GetTerrain(this->tentity->width-1, 0));

	base2.v3->x = real(1);
	base2.v3->y = real(1);
	base2.v3->z = real(GetTerrain(this->tentity->width-1, this->tentity->width-1));

	base1.splitQ = NULL;
	base1.mergeQ = NULL;
	base2.splitQ = NULL;
	base2.mergeQ = NULL;
}

float CDiamondLOD::ComputeVariance(const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const int index)
{
	float xa, ya, za;
	float ea, eb, ec, ed, et;
	float vr;

	if(index >= varsize)
		return 0;
	else
	{
		// { compute local variances }
		xa = (x1 + x2) >> 1;
		ya = (y1 + y2) >> 1;                  // scaled by a bit to improve accuracy:
		za = GetTerrain(x1, y1) + GetTerrain(x2, y2);  //   = 2 * (GetPix + GetPix) div 2
		vr = abs(GetTerrain(xa, ya) - za); // vr is first variance value

		xa = (x1 + x3) >> 1;
		ya = (y1 + y3) >> 1;
		za = GetTerrain(x1, y1) + GetTerrain(x3, y3);
		vr = max(vr, abs(GetTerrain(xa, ya) - za)); // vr is current max variance

		xa = (x2 + x3) >> 1;
		ya = (y2 + y3) >> 1;
		za = GetTerrain(x2, y2) + GetTerrain(x3, y3);
		vr = max(vr, abs(GetTerrain(xa, ya) - za)); // vr is current max variance
		//{  compute variance of children }
		ea = ComputeVariance((x1 + x2) >> 1, (y1 + y2) >> 1, (x1 + x3) >> 1, (y1 + y3) >> 1, (x2 + x3) >> 1, (y2 + y3) >> 1, (index << 2) + 1);
		eb = ComputeVariance(x1, y1, (x1 + x2) >> 1, (y1 + y2) >> 1, (x1 + x3) >> 1, (y1 + y3) >> 1, (index << 2) + 2);
		ec = ComputeVariance((x1 + x2) >> 1, (y1 + y2) >> 1, x2, y2, (x2 + x3) >> 1, (y2 + y3) >> 1, (index << 2) + 3);
		ed = ComputeVariance((x1 + x3) >> 1, (y1 + y3) >> 1, (x2 + x3) >> 1, (y2 + y3) >> 1, x3, y3, (index << 2) + 4);
		// { find maximum variance in children }
		et = max(ea,eb);
		et = max(et,ec);
		et = max(et,ed);
		et = et + vr;
		if(et >= 255)
			et = 255;
		// { set result and return it }
		(*varCurrent)[index] = et / 2; // scale 1 bit down for array entry
	}
	return et;
}

void CDiamondLOD::InitVariance()
{
	varsize = this->tentity->width * this->tentity->width / 4 - 1;
	//{ base1 }
	varCurrent = Variance1.GetAddress();
	ComputeVariance(0,0, 0,this->tentity->width-1, this->tentity->width-1,0, 0);
    // { base2 }
	varCurrent = Variance2.GetAddress();
	ComputeVariance(0,this->tentity->width-1, this->tentity->width-1,0, this->tentity->width-1,this->tentity->width-1, 0);
}

void CDiamondLOD::InitQueues()
{
	//{ queue for splitable tris above required priority }
	SplitAbove.first.tri = NULL;
	SplitAbove.first.prev = NULL;
	SplitAbove.first.next = &SplitAbove.guard;
	SplitAbove.guard.tri = NULL;
	SplitAbove.guard.prev = &SplitAbove.first;
	SplitAbove.guard.next = NULL;
	//{ queue for splitable tris below required priority }
	SplitBelow.first.tri = NULL;
	SplitBelow.first.prev = NULL;
	SplitBelow.first.next = &SplitBelow.guard;
	SplitBelow.guard.tri = NULL;
	SplitBelow.guard.prev = &SplitBelow.first;
	SplitBelow.guard.next = NULL;
	//{ queue for mergeable tris above required priority }
	MergeAbove.first.tri = NULL;
	MergeAbove.first.prev = NULL;
	MergeAbove.first.next = &MergeAbove.guard;
	MergeAbove.guard.tri = NULL;
	MergeAbove.guard.prev = &MergeAbove.first;
	MergeAbove.guard.next = NULL;
	//{ queue for mergeable tris below required priority }
	MergeBelow.first.tri = NULL;
	MergeBelow.first.prev = NULL;
	MergeBelow.first.next = &MergeBelow.guard;
	MergeBelow.guard.tri = NULL;
	MergeBelow.guard.prev = &MergeBelow.first;
	MergeBelow.guard.next = NULL;
}

void CDiamondLOD::InitScapeDiamond()
{
	ResetDiamond = true;
	AllocPools(this->_maxTris);
	InitPools(this->_maxTris);
	//InitClipArray;
	InitMesh();
	InitVariance();
	InitQueues();
	//{ Initialize vertexbuffer }
	this->OnResetDevice();
	//{ Enable on clockwise culling }
	//d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//{ Disable D3D lighting - we perform manual lighting using vertex colors }
	//D3DDev8.SetR}erState(D3DRS_LIGHTING, Ord(false));
	//{ Set fill mode }
}

void CDiamondLOD::ResetMesh()
{
	//{ reset base mesh }
	base1.childs[0] = NULL;
	base1.childs[1] = NULL;
	base1.childs[2] = NULL;
	base1.childs[3] = NULL;
	base2.childs[0] = NULL;
	base2.childs[1] = NULL;
	base2.childs[2] = NULL;
	base2.childs[3] = NULL;
	//{ reset queues }
	SplitAbove.first.next = &SplitAbove.guard;
	SplitBelow.first.next = &SplitBelow.guard;
	MergeAbove.first.next = &MergeAbove.guard;
	MergeBelow.first.next = &MergeBelow.guard;
	SplitAbove.guard.prev = &SplitAbove.first;
	SplitBelow.guard.prev = &SplitBelow.first;
	MergeAbove.guard.prev = &MergeAbove.first;
	MergeBelow.guard.prev = &MergeBelow.first;
	//{ re-initialize pools }
	InitPools(this->_maxTris);
	renderCount = 0;
}

void TDiamond::CullFrustum()
{
	//Vector tmp, maximum;
	//real t;
	//tmp = this->v1;
	//t = 1;

	/*if(this->v1->lupdate < GameTicks)
	{
		tmp.x = this->v1->x;
		tmp.y = this->v1->z;
		tmp.z = this->v1->y;
		tmp.x *= this->dlod->scale->x;
		tmp.y *= this->dlod->scale->y;
		tmp.z *= this->dlod->scale->z;
		if(this->dlod->GetCamera()->PointVisible(&tmp))
			this->v1->visible = true;
		else
			this->v1->visible = false;
		this->v1->lupdate = GameTicks;
	}
	if(this->v2->lupdate < GameTicks)
	{
		tmp.x = this->v2->x;
		tmp.y = this->v2->z;
		tmp.z = this->v2->y;
		tmp.x *= this->dlod->scale->x;
		tmp.y *= this->dlod->scale->y;
		tmp.z *= this->dlod->scale->z;
		if(this->dlod->GetCamera()->PointVisible(&tmp))
			this->v2->visible = true;
		else
			this->v2->visible = false;
		this->v2->lupdate = GameTicks;
	}
	if(this->v3->lupdate < GameTicks)
	{
		tmp.x = this->v3->x;
		tmp.y = this->v3->z;
		tmp.z = this->v3->y;
		tmp.x *= this->dlod->scale->x;
		tmp.y *= this->dlod->scale->y;
		tmp.z *= this->dlod->scale->z;
		if(this->dlod->GetCamera()->PointVisible(&tmp))
			this->v3->visible = true;
		else
			this->v3->visible = false;
		this->v3->lupdate = GameTicks;
	}*/

	//if(this->dlod->GetCamera()->SphereVisible(&tmp, t))//this->v1->visible || this->v2->visible || this->v3->visible)
		this->clipcode = TRIANGLE_VISIBLE;
	//else
	//	this->clipcode = TRIANGLE_INVISIBLE;
}

inline void TDiamond::CalculatePriority(const Vector *p)
{
	Vector c;
	real dist;

	PROFILE_START;

	//{ if smallest possible tri, or it tri outside then minimum priority }
	if (this->varindex >= this->dlod->varsize || !freeTriSP || (this->clipcode != TRIANGLE_VISIBLE))
		this->priority = prMin;
	//{ calculate midpoint and variance over distance to midpoint }
	else {
		c.x = (this->v1->x + this->v2->x + this->v3->x)*this->dlod->tentity->GetScale()->x;
		c.y = (this->v1->y + this->v2->y + this->v3->y)*this->dlod->tentity->GetScale()->z;
		c.z = (this->v1->z + this->v2->z + this->v3->z)*this->dlod->tentity->GetScale()->y;
		c *= r(0.33333333);

		const real c1 = c.x - p->x;
		const real c2 = c.z - p->y;
		const real c3 = c.y - p->z;
		dist = r(sqrt(c1*c1 + c2*c2 + c3*c3))*gc->GetTerrainDistFactor();
		//if(fabs(dist) < 0.00001f)
		//	dist = 0.00001f;
		this->priority = ((*this->variance)[this->varindex]) / (dist);
		/*if(this->parent)
			if(this->priority >= this->parent->priority)
				this->priority = this->parent->priority/4.0f;*/
	}
	PROFILE_END;
}

void TDiamond::SplitEnqueue(const real targetPriority, const Vector *cpos, TQueue * SplitBelow, TQueue * SplitAbove)
{
	this->splitQ = newSplit(this);
	this->CalculatePriority(cpos);
	//{ enqueue tri in Split queues }
	if(this->priority < targetPriority) {
		this->splitQ->next = SplitBelow->first.next;
		this->splitQ->prev = &SplitBelow->first;
		SplitBelow->first.next->prev = this->splitQ;
		SplitBelow->first.next = this->splitQ;
	} else {
		this->splitQ->next = SplitAbove->first.next;
		this->splitQ->prev = &SplitAbove->first;
		SplitAbove->first.next->prev = this->splitQ;
		SplitAbove->first.next = this->splitQ;
	}
}

const CCamera *CDiamondLOD::GetCamera()
{ return this->cam; }

void CDiamondLOD::SetCamera(const CCamera *c)
{ this->cam = c; }

void CDiamondLOD::SplitRequeue()
{
	TNode *node;
	TDiamond *tri;

	node = SplitBelow.first.next;
	while(node->next) {
		tri = node->tri;
		node = node->next;
		tri->CullFrustum();//CullFrustum(tri);
		tri->CalculatePriority(this->GetCamera()->GetPosition());
		if(tri->priority >= targetPriority) {
			//{ dequeue from above }
			tri->splitQ->next->prev = tri->splitQ->prev;
			tri->splitQ->prev->next = tri->splitQ->next;
			//{ enqueue in below }
			tri->splitQ->next = SplitAbove.first.next;
			tri->splitQ->prev = &SplitAbove.first;
			SplitAbove.first.next->prev = tri->splitQ;
			SplitAbove.first.next = tri->splitQ;
		}
	}
}

void CDiamondLOD::MergeRequeue()
{
	TNode *node;
	TDiamond *tri;

	node = MergeAbove.first.next;
	while(node->next) {
		tri = node->tri;
		node = node->next;
		tri->CullFrustum();
		tri->CalculatePriority(this->GetCamera()->GetPosition());
		if(tri->priority < targetPriority) {
			//{ dequeue from above }
			tri->mergeQ->next->prev = tri->mergeQ->prev;
			tri->mergeQ->prev->next = tri->mergeQ->next;
			//{ enqueue in below }
			tri->mergeQ->next = MergeBelow.first.next;
			tri->mergeQ->prev = &MergeBelow.first;
			MergeBelow.first.next->prev = tri->mergeQ;
			MergeBelow.first.next = tri->mergeQ;
		}
	}
}

bool TDiamond::canMerge() const
{
	// determine whether tri is mergeable
	// check if children exist and if children have children 
	if((this->childs[0] == NULL) || (this->childs[0]->childs[0]) || (this->childs[1]->childs[0]) || (this->childs[2]->childs[0]) || (this->childs[3]->childs[0]))
		return false;
	if((this->n1) && (this->n1->childs[0]) && ((this->n1->childs[2]->childs[0]) || (this->n1->childs[3]->childs[0])))
		return false;
	if((this->n2) && (this->n2->childs[0]) && ((this->n2->childs[1]->childs[0]) || (this->n2->childs[3]->childs[0])))
		return false;
	if((this->n3) && (this->n3->childs[0]) && ((this->n3->childs[1]->childs[0]) || (this->n3->childs[2]->childs[0])))
		return false;
	// tri can be merged 
	return true;
}

void CDiamondLOD::Merge(TDiamond *tri)
{
	// merges tri and requeues appropriately
	TDiamond *child = NULL;

	//{ remove children from split queues }
	if(tri->childs[0]->splitQ) {
		for(register int i = 3; i >= 0; --i) {
			//switch(i) {
				//case 1:
					child = tri->childs[i];
				//	break;
				//case 2:
				//	child = tri->b;
				//	break;
				//case 3:
				//	child = tri->c;
				//	break;
				//case 4:
				//	child = tri->d;
				//	break;
			//}
			if(child->splitQ)
			{
				child->splitQ->next->prev = child->splitQ->prev;
				child->splitQ->prev->next = child->splitQ->next;
				deleteSplit(child->splitQ);
				child->splitQ = NULL;
			}
		}
	}
	
	//{ dequeue tri from MergeBelow }
	
	tri->mergeQ->next->prev = tri->mergeQ->prev;
	tri->mergeQ->prev->next = tri->mergeQ->next;
	deleteMerge(tri->mergeQ);
	tri->mergeQ = NULL;
	
	//{ delete vertices that are now obsolete }
	if((tri->n1 == NULL) || (tri->n1->childs[0] == NULL))
		deletePos(tri->childs[0]->v1);
	if((tri->n2 == NULL) || (tri->n2->childs[0] == NULL))
		deletePos(tri->childs[0]->v2);
	if((tri->n3 == NULL) || (tri->n3->childs[0] == NULL))
		deletePos(tri->childs[0]->v3);
	//{ relocate neighbours }
	
	if((tri->n1) && (tri->n1->childs[0])) {
		tri->n1->childs[2]->n3 = tri;
		tri->n1->childs[3]->n3 = tri;
	}
	
	if((tri->n2) && (tri->n2->childs[0])) {
		tri->n2->childs[1]->n2 = tri;
		tri->n2->childs[3]->n2 = tri;
	}
	if((tri->n3) && (tri->n3->childs[0])) {
		tri->n3->childs[1]->n1 = tri;
		tri->n3->childs[2]->n1 = tri;
	}
	
	//{ kill children }
	deleteTri(tri->childs[0]);
	deleteTri(tri->childs[1]);
	deleteTri(tri->childs[2]);
	deleteTri(tri->childs[3]);
	
	tri->childs[0] = NULL;
	tri->childs[1] = NULL;
	tri->childs[2] = NULL;
	tri->childs[3] = NULL;
	
	//{ handle major neighbours }
	//{ if left neighbour's parent is now mergeable, enqueue it }
	
	if((tri->n1) && (tri->n1->parent) && (tri->n1->parent != tri->parent) && (tri->n1->parent->canMerge())) {
		tri->n1->parent->CullFrustum();
		tri->n1->parent->CalculatePriority(this->GetCamera()->GetPosition());
		tri->n1->parent->mergeQ = newMerge(tri->n1->parent);
		if(tri->n1->parent->priority < targetPriority) {
			tri->n1->parent->mergeQ->next = MergeBelow.first.next;
			tri->n1->parent->mergeQ->prev = &MergeBelow.first;
			MergeBelow.first.next->prev = tri->n1->parent->mergeQ;
			MergeBelow.first.next = tri->n1->parent->mergeQ;
		} else {
			tri->n1->parent->mergeQ->next = MergeAbove.first.next;
			tri->n1->parent->mergeQ->prev = &MergeAbove.first;
			MergeAbove.first.next->prev = tri->n1->parent->mergeQ;
			MergeAbove.first.next = tri->n1->parent->mergeQ;
		}
	}

	// { if middle neighbour's parent is now mergeable, enqueue it }
	if((tri->n2) && (tri->n2->parent) && (tri->n2->parent != tri->parent) && (tri->n2->parent->canMerge())) {
		tri->n2->parent->CullFrustum();
		tri->n2->parent->CalculatePriority(this->GetCamera()->GetPosition());
		tri->n2->parent->mergeQ = newMerge(tri->n2->parent);
		if(tri->n2->parent->priority < targetPriority) {
			tri->n2->parent->mergeQ->next = MergeBelow.first.next;
			tri->n2->parent->mergeQ->prev = &MergeBelow.first;
			MergeBelow.first.next->prev = tri->n2->parent->mergeQ;
			MergeBelow.first.next = tri->n2->parent->mergeQ;
		} else {
			tri->n2->parent->mergeQ->next = MergeAbove.first.next;
			tri->n2->parent->mergeQ->prev = &MergeAbove.first;
			MergeAbove.first.next->prev = tri->n2->parent->mergeQ;
			MergeAbove.first.next = tri->n2->parent->mergeQ;
		}
	}

	// { if right neighbour's parent is now mergeable, enqueue it }
	if((tri->n3) && (tri->n3->parent) && (tri->n3->parent != tri->parent) && (tri->n3->parent->canMerge())) {
		tri->n3->parent->CullFrustum();
		tri->n3->parent->CalculatePriority(this->GetCamera()->GetPosition());
		tri->n3->parent->mergeQ = newMerge(tri->n3->parent);
		if(tri->n3->parent->priority < targetPriority) {
			tri->n3->parent->mergeQ->next = MergeBelow.first.next;
			tri->n3->parent->mergeQ->prev = &MergeBelow.first;
			MergeBelow.first.next->prev = tri->n3->parent->mergeQ;
			MergeBelow.first.next = tri->n3->parent->mergeQ;
		} else {
			tri->n3->parent->mergeQ->next = MergeAbove.first.next;
			tri->n3->parent->mergeQ->prev = &MergeAbove.first;
			MergeAbove.first.next->prev = tri->n3->parent->mergeQ;
			MergeAbove.first.next = tri->n3->parent->mergeQ;
		}
	}
	//{ handle tri's parent }
	if(tri->parent) {
		if(tri->parent->canMerge()) {
			tri->parent->CullFrustum();
			tri->parent->CalculatePriority(this->GetCamera()->GetPosition());
			tri->parent->mergeQ = newMerge(tri->parent);
			//{ enqueue in MergeBelow }
			if(tri->parent->priority < targetPriority) {
				tri->parent->mergeQ->next = MergeBelow.first.next;
				tri->parent->mergeQ->prev = &MergeBelow.first;
				MergeBelow.first.next->prev = tri->parent->mergeQ;
				MergeBelow.first.next = tri->parent->mergeQ;
				//{ enqueue in MergeAbove }
			} else {
				tri->parent->mergeQ->next = MergeAbove.first.next;
				tri->parent->mergeQ->prev = &MergeAbove.first;
				MergeAbove.first.next->prev = tri->parent->mergeQ;
				MergeAbove.first.next = tri->parent->mergeQ;
			}
		}
	}
	//{ enqueue tri in split queue }
	tri->splitQ = newSplit(tri);
	tri->splitQ->next = SplitBelow.first.next;
	tri->splitQ->prev = &SplitBelow.first;
	SplitBelow.first.next->prev = tri->splitQ;
	SplitBelow.first.next = tri->splitQ;
}

void CDiamondLOD::MakeChildren(TDiamond *tri)
{
	TPos *p1,*p2,*p3;

	// { create children }
	tri->childs[0] = newTri(tri,1, this);
	tri->childs[1] = newTri(tri,2, this);
	tri->childs[2] = newTri(tri,3, this);
	tri->childs[3] = newTri(tri,4, this);
	
	//{ check for/create additional vertices }
	if((tri->n1) && (tri->n1->childs[0]))
		p1 = tri->n1->childs[0]->v3;
	else
		p1 = newPos(tri->v1, tri->v2);

	if((tri->n2) && (tri->n2->childs[0]))
		p2 = tri->n2->childs[0]->v2;
	else
		p2 = newPos(tri->v1, tri->v3);
	
	if((tri->n3) && (tri->n3->childs[0]))
		p3 = tri->n3->childs[0]->v1;
	else
		p3 = newPos(tri->v2, tri->v3);
	//{ assign existing vertices }
	tri->childs[1]->v1 = tri->v1;
	tri->childs[2]->v2 = tri->v2;
	tri->childs[3]->v3 = tri->v3;
	//{ assign p1 }
	tri->childs[0]->v1 = p1;
	tri->childs[1]->v2 = p1;
	tri->childs[2]->v1 = p1;
	//{ assign p2 }
	tri->childs[0]->v2 = p2;
	tri->childs[1]->v3 = p2;
	tri->childs[3]->v1 = p2;
	//{ assign p3 }
	tri->childs[0]->v3 = p3;
	tri->childs[2]->v3 = p3;
	tri->childs[3]->v2 = p3;
	//{ assign neighbours for child A }
	tri->childs[0]->n1 = tri->childs[1];
	tri->childs[0]->n2 = tri->childs[2];
	tri->childs[0]->n3 = tri->childs[3];
	//{ assign neighbours from child A }
	tri->childs[1]->n3 = tri->childs[0];
	tri->childs[2]->n2 = tri->childs[0];
	tri->childs[3]->n1 = tri->childs[0];
	//{ assign left neighbour for children B,C }
	if(tri->n1) {
		if(tri->n1->childs[0]) {
			tri->childs[1]->n1 = tri->n1->childs[2];
			tri->childs[2]->n1 = tri->n1->childs[3];
			tri->n1->childs[2]->n3 = tri->childs[1];
			tri->n1->childs[3]->n3 = tri->childs[2];
		} else {
			tri->childs[1]->n1 = tri->n1;
			tri->childs[2]->n1 = tri->n1;
		}
	} else {
		tri->childs[1]->n1 = NULL;
		tri->childs[2]->n1 = NULL;
	}
	
	//{ assign vertical neighbour for children B,D }
	if(tri->n2) {
		if(tri->n2->childs[0]) {
			tri->childs[1]->n2 = tri->n2->childs[1];
			tri->childs[3]->n2 = tri->n2->childs[3];
			tri->n2->childs[1]->n2 = tri->childs[1];
			tri->n2->childs[3]->n2 = tri->childs[3];
		} else {
			tri->childs[1]->n2 = tri->n2;
			tri->childs[3]->n2 = tri->n2;
		}
	} else {
		tri->childs[1]->n2 = NULL;
		tri->childs[3]->n2 = NULL;
	}
	
	//{ assign right neighbour for children C,D }
	if(tri->n3) {
		if(tri->n3->childs[0]) {
			tri->childs[2]->n3 = tri->n3->childs[1];
			tri->childs[3]->n3 = tri->n3->childs[2];
			tri->n3->childs[1]->n1 = tri->childs[2];
			tri->n3->childs[2]->n1 = tri->childs[3];
		} else {
			tri->childs[2]->n3 = tri->n3;
			tri->childs[3]->n3 = tri->n3;
		}
	} else {
		tri->childs[2]->n3 = NULL;
		tri->childs[3]->n3 = NULL;
	}
}

void CDiamondLOD::Split(TDiamond *tri)
{
	TDiamond *child;

	// { if in queue, dequeue tri from SplitAbove }
	if(tri->splitQ) {
		tri->splitQ->next->prev = tri->splitQ->prev;
		tri->splitQ->prev->next = tri->splitQ->next;
		deleteSplit(tri->splitQ);
		tri->splitQ = NULL;
	}
	
	//{ ensure that parent is not in merge queue }
	if((tri->parent != NULL) && (tri->parent->mergeQ != NULL)) {
		tri->parent->mergeQ->next->prev = tri->parent->mergeQ->prev;
		tri->parent->mergeQ->prev->next = tri->parent->mergeQ->next;
		deleteMerge(tri->parent->mergeQ);
		tri->parent->mergeQ = NULL;
	}

	//{ ensure that it is legal to split tri }
	if((tri->n1 != NULL) && (tri != tri->n1->n3))
		Split(tri->n1);
	if((tri->n2 != NULL) && (tri != tri->n2->n2))
		Split(tri->n2);
	if((tri->n3 != NULL) && (tri != tri->n3->n1))
		Split(tri->n3);
	
	//{ remove major neighbours out of merge queue }
	if((tri->n1 != NULL) && (tri->n1->parent != NULL) && (tri->n1->parent->mergeQ != NULL)) {
		tri->n1->parent->mergeQ->next->prev = tri->n1->parent->mergeQ->prev;
		tri->n1->parent->mergeQ->prev->next = tri->n1->parent->mergeQ->next;
		deleteMerge(tri->n1->parent->mergeQ);
		tri->n1->parent->mergeQ = NULL;
	}
	if((tri->n2 != NULL) && (tri->n2->parent != NULL) && (tri->n2->parent->mergeQ != NULL)) {
		tri->n2->parent->mergeQ->next->prev = tri->n2->parent->mergeQ->prev;
		tri->n2->parent->mergeQ->prev->next = tri->n2->parent->mergeQ->next;
		deleteMerge(tri->n2->parent->mergeQ);
		tri->n2->parent->mergeQ = NULL;
	}
	if((tri->n3 != NULL) && (tri->n3->parent != NULL) && (tri->n3->parent->mergeQ != NULL)) {
		tri->n3->parent->mergeQ->next->prev = tri->n3->parent->mergeQ->prev;
		tri->n3->parent->mergeQ->prev->next = tri->n3->parent->mergeQ->next;
		deleteMerge(tri->n3->parent->mergeQ);
		tri->n3->parent->mergeQ = NULL;
	}
	//{ split tri }
	MakeChildren(tri);
	//{ enqueue as appropriate }
	for(register int i = 3; i >= 0; --i)
	{
		child = tri->childs[i];

		//{ find child priority }
		child->CullFrustum();
		child->CalculatePriority(this->GetCamera()->GetPosition());
		//{ split }
		if(child->priority >= targetPriority) {
			//{ enqueue in SplitAbove }
			child->splitQ = newSplit(child);
			child->splitQ->next = SplitAbove.first.next;
			child->splitQ->prev = &SplitAbove.first;
			SplitAbove.first.next->prev = child->splitQ;
			SplitAbove.first.next = child->splitQ;
			//{ enqueue in SplitBelow }
		} else {
			child->splitQ = newSplit(child);
			child->splitQ->next = SplitBelow.first.next;
			child->splitQ->prev = &SplitBelow.first;
			SplitBelow.first.next->prev = child->splitQ;
			SplitBelow.first.next = child->splitQ;
		}
	}
	//{ if tri is mergeable, enqueue }
	if((tri->varindex >= varsize) || (tri->canMerge())) {
		tri->mergeQ = newMerge(tri);
		tri->mergeQ->next = MergeAbove.first.next;
		tri->mergeQ->prev = &MergeAbove.first;
		MergeAbove.first.next->prev = tri->mergeQ;
		MergeAbove.first.next = tri->mergeQ;
	}
}

HRESULT CDiamondLOD::OnLostDevice()
{
	SafeRelease(this->vb_RM);
	SafeRelease(this->ib_RM);

	return S_OK;
}

HRESULT CDiamondLOD::OnResetDevice()
{
	HRESULT hr;

	hr = d3dDevice->CreateVertexBuffer(sizeof(TCustomVertex)*this->_maxTris, D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &vb_RM, NULL);
	if(SUCCEEDED(hr))
	{
		hr = d3dDevice->CreateIndexBuffer(sizeof(short)*(this->_maxTris*3), D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ib_RM, NULL);
		if(FAILED(hr))
			RuntimeError("O buffer de indices não pode ser criado!\nCódigo do Erro: %d", HRESULT_CODE(hr));
	}
	else
		RuntimeError("O buffer de vertices não pode ser criado!\nCódigo do Erro: %d", HRESULT_CODE(hr));
	return hr;
}

void CDiamondLOD::RenderMesh0()
{
	// generate vertexbuffer and render the result
	TNode *node;
	TDiamond *tri;
	int cnt;
	TPos *s1 = NULL, *s2 = NULL, *s3 = NULL;
	unsigned char *_ptr;

	glTri = 0;
	glInd = 0;
	
	//const unsigned int mini = static_cast<unsigned int>(floorf(freePosSP * 0.9f));
	//for(register unsigned int i = 0; i < PosPool.GetCapacity(); ++i)
	//	PosPool[i].clip = -1;

	basev1.clip = -1;
	basev2.clip = -1;
	basev3.clip = -1;
	basev4.clip = -1;

	node = &SplitBelow.first;
	
	TCustomVertex *vb = &_vb[glTri]; 
	*vb = GetCustomVertex(basev1);
	basev1.clip = glTri;
	++vb;
	++glTri;

	//{ parse SplitBelow and insert visible tris }
	while(node->next->next) {
		node = node->next;
		//{ only draw visible tris }
		if(node->tri->clipcode == TRIANGLE_VISIBLE) {
			tri =  node->tri;
			cnt = 0;
			//{ ensure that v1 is in vertexbuffer }
			if(tri->v1->clip < 0) {
				*vb = GetCustomVertex(tri->v1);
				tri->v1->clip = glTri;
				++vb;
				++glTri;
			}
			//{ ensure that v2 is in vertexbuffer }
			if(tri->v2->clip < 0) {
				*vb = GetCustomVertex(tri->v2);
				tri->v2->clip = glTri;
				++vb;
				++glTri;
			}
			//{ ensure that v3 is in vertexbuffer }
			if(tri->v3->clip < 0) {
				*vb = GetCustomVertex(tri->v3);
				tri->v3->clip = glTri;
				++vb;
				++glTri;
			}
			//{ find mid vertices as appropriate }
			if((tri->n1 != NULL) && (tri->n1->childs[0] != NULL)) {
				s1 = tri->n1->childs[0]->v3;
				//{ ensure that s1 is in vertex buffer }
				if(s1->clip < 0) {
					*vb = GetCustomVertex(s1);
					s1->clip = glTri;
					++vb;
					++glTri;
				}
				cnt = con1;
			}
			//{ find mid vertices as appropriate }
			if((tri->n2 != NULL) && (tri->n2->childs[0] != NULL)) {
				s2 = tri->n2->childs[0]->v2;
				// { ensure that s2 is in vertex buffer }
				if(s2->clip < 0) {
					*vb = GetCustomVertex(s2);
					s2->clip = glTri;
					++vb;
					++glTri;
				}
				cnt = cnt | con2;
			}
			//{ find mid vertices as appropriate }
			if((tri->n3 != NULL) && (tri->n3->childs[0] != NULL)) {
				s3 = tri->n3->childs[0]->v1;
				//{ ensure that s3 is in vertex buffer }
				if(s3->clip < 0) {
					*vb = GetCustomVertex(s3);
					s3->clip = glTri;
					++vb;
					++glTri;
				}
				cnt = cnt | con3;
			}

			unsigned short *ib = &_ib[glInd];
			*ib = tri->v1->clip;
			//{ if tri points down }
			if(tri->v1->y < tri->v2->y) {
				switch(cnt){
					case 0:
						*(ib+1)= tri->v2->clip;
						*(ib+2) = tri->v3->clip;
						glInd = glInd + 3;
						break;
					case con1:
						*(ib+1) = s1->clip;
						*(ib+2) = tri->v3->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = tri->v2->clip;
						*(ib+5) = tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con2:
						*(ib+1) = tri->v2->clip;
						*(ib+2) = s2->clip;
						*(ib+3) = s2->clip;
						*(ib+4) = tri->v2->clip;
						*(ib+5) = tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con3:
						*(ib+1) = s3->clip;
						*(ib+2) = tri->v3->clip;
						*(ib+3) = tri->v1->clip;
						*(ib+4) = tri->v2->clip;
						*(ib+5) = s3->clip;
						glInd = glInd + 6;
						break;
					case cn12:
						*(ib+1) = s1->clip;
						*(ib+2) = s2->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = tri->v3->clip;
						*(ib+5) = s2->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = tri->v2->clip;
						*(ib+8)  = tri->v3->clip;
						glInd = glInd + 9;
						break;
					case cn13:
						*(ib+1) = s1->clip;
						*(ib+2) = tri->v3->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = s3->clip;
						*(ib+5) = tri->v3->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = tri->v2->clip;
						*(ib+8) = s3->clip;
						glInd = glInd + 9;
						break;
					case cn23:
						*(ib+1) = tri->v2->clip;
						*(ib+2) = s3->clip;
						*(ib+3) = tri->v1->clip;
						*(ib+4) = s3->clip;
						*(ib+5) = s2->clip;
						*(ib+6) = s2->clip;
						*(ib+7) = s3->clip;
						*(ib+8) = tri->v3->clip;
						glInd = glInd + 9;
						break;
					case cAll:
						*(ib+1) = s1->clip;
						*(ib+2) = s2->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = s3->clip;
						*(ib+5) = s2->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = tri->v2->clip;
						*(ib+8) = s3->clip;
						*(ib+9) = s2->clip;
						*(ib+10) = s3->clip;
						*(ib+11) = tri->v3->clip;
						glInd = glInd + 12;
						break;
				};
			//{ if tri points up }
			} else {
				switch(cnt) {
					case 0:
						*(ib+1) = tri->v3->clip;
						*(ib+2) = tri->v2->clip;
						glInd = glInd + 3;
						break;
					case con1:
						*(ib+1) = tri->v3->clip;
						*(ib+2) = s1->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = tri->v3->clip;
						*(ib+5) = tri->v2->clip;
						glInd = glInd + 6;
						break;
					case con2:
						*(ib+1) = s2->clip;
						*(ib+2) = tri->v2->clip;
						*(ib+3) = tri->v2->clip;
						*(ib+4) = s2->clip;
						*(ib+5) = tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con3:
						*(ib+1) = tri->v3->clip;
						*(ib+2) = s3->clip;
						*(ib+3) = tri->v1->clip;
						*(ib+4) = s3->clip;
						*(ib+5) = tri->v2->clip;
						glInd = glInd + 6;
						break;
					case cn12:
						*(ib+1) = s2->clip;
						*(ib+2) = s1->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = s2->clip;
						*(ib+5) = tri->v3->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = tri->v3->clip;
						*(ib+8) = tri->v2->clip;
						glInd = glInd + 9;
						break;
					case cn13:
						*(ib+1) = tri->v3->clip;
						*(ib+2) = s1->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = tri->v3->clip;
						*(ib+5) = s3->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = s3->clip;
						*(ib+8) = tri->v2->clip;
						glInd = glInd + 9;
						break;
					case cn23:
						*(ib+1) = s3->clip;
						*(ib+2) = tri->v2->clip;
						*(ib+3) = tri->v1->clip;
						*(ib+4) = s2->clip;
						*(ib+5) = s3->clip;
						*(ib+6) = s2->clip;
						*(ib+7) = tri->v3->clip;
						*(ib+8) = s3->clip;
						glInd = glInd + 9;
						break;
					case cAll:
						*(ib+1) = s2->clip;
						*(ib+2) = s1->clip;
						*(ib+3) = s1->clip;
						*(ib+4) = s2->clip;
						*(ib+5) = s3->clip;
						*(ib+6) = s1->clip;
						*(ib+7) = s3->clip;
						*(ib+8) = tri->v2->clip;
						*(ib+9) = s2->clip;
						*(ib+10) = tri->v3->clip;
						*(ib+11) = s3->clip;
						glInd = glInd + 12;
						break;
					};
				}
			}
	}
	//{ move vertex data to vertex buffer }
	if(FAILED(vb_RM->Lock(0, glTri * sizeof(TCustomVertex), reinterpret_cast<void**>(&_ptr), D3DLOCK_DISCARD)))
		return;
	memcpy(_ptr, &_vb, glTri * sizeof(TCustomVertex));
	vb_RM->Unlock();
	//{ move index data to index buffer }
	if(FAILED(ib_RM->Lock(0, glInd * sizeof(short), reinterpret_cast<void**>(&_ptr), D3DLOCK_DISCARD)))
		return;
	memcpy(_ptr, &_ib, glInd * sizeof(short));
	ib_RM->Unlock();
	//{ draw the final mesh }
	//{ Enable on clockwise culling }
	PROFILE_START;
	//d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, glTri, 0, glInd / 3);
	PROFILE_END;
}

void CDiamondLOD::RenderMesh() {
	if(renderCount % 2 == 0)
		RenderMesh0();
	else
		RenderMesh1();
	++renderCount;
}

void CDiamondLOD::RenderMesh1()
{
	// generate vertexbuffer and render the result
	TNode *node;
	TDiamond *tri;
	int cnt;
	TPos *s1 = NULL, *s2 = NULL, *s3 = NULL;
	unsigned char *_ptr;

	glTri = 0;
	glInd = 0;
	
	//const unsigned int mini = static_cast<unsigned int>(floorf(freePosSP * 0.9f));
	//for(register unsigned int i = 0; i < PosPool.GetCapacity(); -i)
	//	PosPool[i].clip = -1;

	basev1.clip = 1;
	basev2.clip = 1;
	basev3.clip = 1;
	basev4.clip = 1;

	node = &SplitBelow.first;
	
	TCustomVertex *vb = &_vb[glTri]; 
	*vb = GetCustomVertex(basev1);
	basev1.clip = glTri;
	++vb;
	--glTri;

	//{ parse SplitBelow and insert visible tris }
	while(node->next->next) {
		node = node->next;
		//{ only draw visible tris }
		if(node->tri->clipcode == TRIANGLE_VISIBLE) {
			tri =  node->tri;
			cnt = 0;
			//{ ensure that v1 is in vertexbuffer }
			if(tri->v1->clip > 0) {
				*vb = GetCustomVertex(tri->v1);
				tri->v1->clip = glTri;
				++vb;
				--glTri;
			}
			//{ ensure that v2 is in vertexbuffer }
			if(tri->v2->clip > 0) {
				*vb = GetCustomVertex(tri->v2);
				tri->v2->clip = glTri;
				++vb;
				--glTri;
			}
			//{ ensure that v3 is in vertexbuffer }
			if(tri->v3->clip > 0) {
				*vb = GetCustomVertex(tri->v3);
				tri->v3->clip = glTri;
				++vb;
				--glTri;
			}
			//{ find mid vertices as appropriate }
			if((tri->n1 != NULL) && (tri->n1->childs[0] != NULL)) {
				s1 = tri->n1->childs[0]->v3;
				//{ ensure that s1 is in vertex buffer }
				if(s1->clip > 0) {
					*vb = GetCustomVertex(s1);
					s1->clip = glTri;
					++vb;
					--glTri;
				}
				cnt = con1;
			}
			//{ find mid vertices as appropriate }
			if((tri->n2 != NULL) && (tri->n2->childs[0] != NULL)) {
				s2 = tri->n2->childs[0]->v2;
				// { ensure that s2 is in vertex buffer }
				if(s2->clip > 0) {
					*vb = GetCustomVertex(s2);
					s2->clip = glTri;
					++vb;
					--glTri;
				}
				cnt = cnt | con2;
			}
			//{ find mid vertices as appropriate }
			if((tri->n3 != NULL) && (tri->n3->childs[0] != NULL)) {
				s3 = tri->n3->childs[0]->v1;
				//{ ensure that s3 is in vertex buffer }
				if(s3->clip > 0) {
					*vb = GetCustomVertex(s3);
					s3->clip = glTri;
					++vb;
					--glTri;
				}
				cnt = cnt | con3;
			}

			unsigned short *ib = &_ib[glInd];
			*ib = -tri->v1->clip;
			//{ if tri points down }
			if(tri->v1->y < tri->v2->y) {
				switch(cnt){
					case 0:
						*(ib+1)= -tri->v2->clip;
						*(ib+2) = -tri->v3->clip;
						glInd = glInd + 3;
						break;
					case con1:
						*(ib+1) = -s1->clip;
						*(ib+2) = -tri->v3->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -tri->v2->clip;
						*(ib+5) = -tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con2:
						*(ib+1) = -tri->v2->clip;
						*(ib+2) = -s2->clip;
						*(ib+3) = -s2->clip;
						*(ib+4) = -tri->v2->clip;
						*(ib+5) = -tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con3:
						*(ib+1) = -s3->clip;
						*(ib+2) = -tri->v3->clip;
						*(ib+3) = -tri->v1->clip;
						*(ib+4) = -tri->v2->clip;
						*(ib+5) = -s3->clip;
						glInd = glInd + 6;
						break;
					case cn12:
						*(ib+1) = -s1->clip;
						*(ib+2) = -s2->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -tri->v3->clip;
						*(ib+5) = -s2->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -tri->v2->clip;
						*(ib+8)  = -tri->v3->clip;
						glInd = glInd + 9;
						break;
					case cn13:
						*(ib+1) = -s1->clip;
						*(ib+2) = -tri->v3->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -s3->clip;
						*(ib+5) = -tri->v3->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -tri->v2->clip;
						*(ib+8) = -s3->clip;
						glInd = glInd + 9;
						break;
					case cn23:
						*(ib+1) = -tri->v2->clip;
						*(ib+2) = -s3->clip;
						*(ib+3) = -tri->v1->clip;
						*(ib+4) = -s3->clip;
						*(ib+5) = -s2->clip;
						*(ib+6) = -s2->clip;
						*(ib+7) = -s3->clip;
						*(ib+8) = -tri->v3->clip;
						glInd = glInd + 9;
						break;
					case cAll:
						*(ib+1) = -s1->clip;
						*(ib+2) = -s2->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -s3->clip;
						*(ib+5) = -s2->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -tri->v2->clip;
						*(ib+8) = -s3->clip;
						*(ib+9) = -s2->clip;
						*(ib+10) = -s3->clip;
						*(ib+11) = -tri->v3->clip;
						glInd = glInd + 12;
						break;
				};
			//{ if tri points up }
			} else {
				switch(cnt) {
					case 0:
						*(ib+1) = -tri->v3->clip;
						*(ib+2) = -tri->v2->clip;
						glInd = glInd + 3;
						break;
					case con1:
						*(ib+1) = -tri->v3->clip;
						*(ib+2) = -s1->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -tri->v3->clip;
						*(ib+5) = -tri->v2->clip;
						glInd = glInd + 6;
						break;
					case con2:
						*(ib+1) = -s2->clip;
						*(ib+2) = -tri->v2->clip;
						*(ib+3) = -tri->v2->clip;
						*(ib+4) = -s2->clip;
						*(ib+5) = -tri->v3->clip;
						glInd = glInd + 6;
						break;
					case con3:
						*(ib+1) = -tri->v3->clip;
						*(ib+2) = -s3->clip;
						*(ib+3) = -tri->v1->clip;
						*(ib+4) = -s3->clip;
						*(ib+5) = -tri->v2->clip;
						glInd = glInd + 6;
						break;
					case cn12:
						*(ib+1) = -s2->clip;
						*(ib+2) = -s1->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -s2->clip;
						*(ib+5) = -tri->v3->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -tri->v3->clip;
						*(ib+8) = -tri->v2->clip;
						glInd = glInd + 9;
						break;
					case cn13:
						*(ib+1) = -tri->v3->clip;
						*(ib+2) = -s1->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -tri->v3->clip;
						*(ib+5) = -s3->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -s3->clip;
						*(ib+8) = -tri->v2->clip;
						glInd = glInd + 9;
						break;
					case cn23:
						*(ib+1) = -s3->clip;
						*(ib+2) = -tri->v2->clip;
						*(ib+3) = -tri->v1->clip;
						*(ib+4) = -s2->clip;
						*(ib+5) = -s3->clip;
						*(ib+6) = -s2->clip;
						*(ib+7) = -tri->v3->clip;
						*(ib+8) = -s3->clip;
						glInd = glInd + 9;
						break;
					case cAll:
						*(ib+1) = -s2->clip;
						*(ib+2) = -s1->clip;
						*(ib+3) = -s1->clip;
						*(ib+4) = -s2->clip;
						*(ib+5) = -s3->clip;
						*(ib+6) = -s1->clip;
						*(ib+7) = -s3->clip;
						*(ib+8) = -tri->v2->clip;
						*(ib+9) = -s2->clip;
						*(ib+10) = -tri->v3->clip;
						*(ib+11) = -s3->clip;
						glInd = glInd + 12;
						break;
					};
				}
			}
	}
	//{ move vertex data to vertex buffer }
	if(FAILED(vb_RM->Lock(0, -glTri * sizeof(TCustomVertex), reinterpret_cast<void**>(&_ptr), D3DLOCK_DISCARD)))
		return;
	memcpy(_ptr, &_vb, -glTri * sizeof(TCustomVertex));
	vb_RM->Unlock();
	//{ move index data to index buffer }
	if(FAILED(ib_RM->Lock(0, glInd * sizeof(short), reinterpret_cast<void**>(&_ptr), D3DLOCK_DISCARD)))
		return;
	memcpy(_ptr, &_ib, glInd * sizeof(short));
	ib_RM->Unlock();
	//{ draw the final mesh }
	//{ Enable on clockwise culling }
	PROFILE_START;
	//d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, -glTri, 0, glInd / 3);
	PROFILE_END;
}

void CDiamondLOD::DrawScapeDiamond()
{
	//  ResetDiamond = true;  // uncomment to enable split-only version
	//{ clear clip states }
	//const unsigned int mini = static_cast<unsigned int>(floorf(freePosSP * 0.9f));
	//for(register unsigned int i = (_maxTris)-1; i >= mini; --i)
	//	PosPool[i].clip = 0;
	//basev1.clip = 0;
	//basev2.clip = 0;
	//basev3.clip = 0;
	//basev4.clip = 0;
	//{ adjust cull position }
	//prDot = 1;
	//px = posx + prDot*FOV[4].x;
	//py = posy + prDot*FOV[4].y;
	//pz = posz + prDot*FOV[4].z;
	//{ calculate base tri culls }
	base1.CullFrustum();
	base2.CullFrustum();
	//{ initial split queueing }
	if(ResetDiamond) {
		ResetDiamond = false;
		ResetMesh();
		base1.SplitEnqueue(targetPriority, this->GetCamera()->GetPosition(), &SplitBelow, &SplitAbove);
		base2.SplitEnqueue(targetPriority, this->GetCamera()->GetPosition(), &SplitBelow, &SplitAbove);
		/*if(base1.clipcode && base2.clipcode & clOut == clOut) {
			ResetDiamond = true;
			return;
		}*/
		//{ split and merge queueing }
	} else {
		/*if(base1.clipcode && base2.clipcode & clOut == clOut) {
			ResetDiamond = true;
			return;
		}*/
		SplitRequeue();
		MergeRequeue();
	}
	//{ generate mesh through merging and splitting }
	while(MergeBelow.first.next->next)
		Merge(MergeBelow.first.next->tri);
	while(SplitAbove.first.next->next)
		Split(SplitAbove.first.next->tri);
	//{ render results }
	PROFILE_START;
	d3dDevice->SetStreamSource(0, vb_RM, 0, sizeof(TCustomVertex));
	d3dDevice->SetIndices(ib_RM);
	d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	d3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	RenderMesh();
	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	PROFILE_END;
}

void CDiamondLOD::DoDiamond()
{
	//D3DDev8.SetVertexShader(D3DFVF_CUSTOMVERTEX);
	DrawScapeDiamond();
}