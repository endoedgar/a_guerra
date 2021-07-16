#pragma once

#include "3DMath.h"
#include "Vector.h"
#include "Quaternion.h"

class CMesh;

class MeshColisao
{
	Smart::Array<Matematica::Vector> vertices;
	Smart::Array<Matematica::Vector> normals;
	bool getVerticesFromMesh(LPD3DXMESH msh);
	void otimizarEixosSeparadores(void);

public:
	MeshColisao(void);

	void subirVertices(const float r, const float y);
	void cubo(void);
	void cubo(const Matematica::Vector &vMin, const Matematica::Vector &vMax);

	inline const Matematica::Vector &getVertice(const unsigned int i) const
	{ return this->vertices[i]; }

	inline MeshColisao &operator=(const MeshColisao &b)
	{
		this->normals.SetSize(b.normals.GetCapacity());
		this->vertices.SetSize(b.vertices.GetCapacity());

		for(register unsigned int a = 0; a < this->normals.GetCapacity(); ++a) {
			this->normals[a] = b.normals[a];	
		}

		for(register unsigned int a = 0; a < this->vertices.GetCapacity(); ++a) {
			this->vertices[a] = b.vertices[a];	
		}

		return *this;
	}

	void transformed(const D3DXMATRIX &m, const Matematica::CQuaternion &qt, MeshColisao &mc) const;
	bool getMesh(const CMesh *mesh);

	~MeshColisao(void);
};
