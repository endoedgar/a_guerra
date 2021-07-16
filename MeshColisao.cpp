#include "stdafx.h"
#include "MeshColisao.h"
#include "CMesh.h"

using namespace Matematica;

MeshColisao::MeshColisao(void)
{ }

void MeshColisao::subirVertices(const real r, const real y) {
	this->vertices[0].y += r-y;
	this->vertices[1].y += r-y;
	this->vertices[4].y += r-y;
	this->vertices[5].y += r-y;
}

bool MeshColisao::getVerticesFromMesh(LPD3DXMESH msh)
{
	if(msh) {
		LPDIRECT3DVERTEXBUFFER9 vb;
		if(SUCCEEDED(msh->GetVertexBuffer(&vb))) {
			Vector *p = NULL;
			if(SUCCEEDED(vb->Lock(0, 0, reinterpret_cast<void**>(&p), D3DLOCK_READONLY))) {
				if(this->vertices.SetSize(msh->GetNumVertices())) {
					const DWORD nBpV = msh->GetNumBytesPerVertex();
					for(register unsigned int a = 0; a < this->vertices.GetCapacity(); ++a) {
						this->vertices[a] = *p;
						p = reinterpret_cast<Vector*>(reinterpret_cast<char*>(p) + nBpV);
					}
					vb->Unlock();
					return true;
				}
				vb->Unlock();
			}
		}
	}
	return false;
}

void MeshColisao::otimizarEixosSeparadores(void)
{ }

void MeshColisao::cubo(void)
{
	const Vector vMin(static_cast<real>(-0.5), static_cast<real>(-0.5), static_cast<real>(-0.5)), 
				 vMax(static_cast<real>(0.5), static_cast<real>(0.5), static_cast<real>(0.5));
	this->cubo(vMin, vMax);
}

void MeshColisao::cubo(const Matematica::Vector &vMin, const Matematica::Vector &vMax)
{
	this->vertices.SetSize(8);
	this->normals.SetSize(6);

	this->vertices[0] = Vector(vMin.x, vMin.y, vMin.z);
	this->vertices[1] = Vector(vMax.x, vMin.y, vMin.z);
	this->vertices[2] = Vector(vMin.x, vMax.y, vMin.z);
	this->vertices[3] = Vector(vMax.x, vMax.y, vMin.z);
	this->vertices[4] = Vector(vMin.x, vMin.y, vMax.z);
	this->vertices[5] = Vector(vMax.x, vMin.y, vMax.z);
	this->vertices[6] = Vector(vMin.x, vMax.y, vMax.z);
	this->vertices[7] = Vector(vMax.x, vMax.y, vMax.z);

	this->normals[0] = Vector(-1, 0, 0);
	this->normals[1] = Vector(1, 0, 0);
	this->normals[2] = Vector(0, -1, 0);
	this->normals[3] = Vector(0, 1, 0);
	this->normals[4] = Vector(0, 0, -1);
	this->normals[5] = Vector(0, 0, 1);
}

void MeshColisao::transformed(const D3DXMATRIX &m, const CQuaternion &qt, MeshColisao &mc) const
{
	D3DXMATRIX mr;
	qt.rotationMatrix(&mr);

	mc.vertices.SetSize(this->vertices.GetCapacity());
	mc.normals.SetSize(this->normals.GetCapacity());

	for(register unsigned int i = 0; i < this->vertices.GetCapacity(); ++i) {
		mc.vertices[i] = this->vertices[i].Transformed(&m);
	}

	for(register unsigned int j = 0; j < this->normals.GetCapacity(); ++j) {
		mc.normals[j] = this->normals[j].Transformed(&mr);
	}
}

bool MeshColisao::getMesh(const CMesh *mesh) {
	if(mesh) {
		LPD3DXMESH msh = mesh->getMeshObject();
		if(msh) {
			if(this->getVerticesFromMesh(msh)) {
				const DWORD nFaces = msh->GetNumFaces();
				if(nFaces) {
					Smart::Array<DWORD> tmpArray;
					if(tmpArray.SetSize(3*nFaces)) {
						if(SUCCEEDED(msh->GenerateAdjacency(EPSILON, &tmpArray))) {
							if(this->normals.SetSize(nFaces)) {
								LPDIRECT3DINDEXBUFFER9 ppIB = NULL;
								if(SUCCEEDED(msh->GetIndexBuffer(&ppIB))) {
									short *ppIBData;
									if(SUCCEEDED(ppIB->Lock(0, 0, reinterpret_cast<void**>(&ppIBData), D3DLOCK_READONLY))) {
										for(register unsigned int a = 0, b = 0; a < this->normals.GetCapacity(); ++a, b+=3) {
											const Vector &b0 = this->vertices[ppIBData[b]];
											const Vector &b1 = this->vertices[ppIBData[b+1]];
											const Vector &b2 = this->vertices[ppIBData[b+2]];

											this->normals[a] = (b1-b0).cross(b2-b0);
											this->normals[a].normalize();
										}
										ppIB->Unlock();

										this->otimizarEixosSeparadores();

										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return false;
}

MeshColisao::~MeshColisao(void)
{ }
