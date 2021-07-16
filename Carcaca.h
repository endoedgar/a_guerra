#pragma once
#include "3dentity.h"
#include "cmesh.h"

class Carcaca :
	public Entity
{
	CMesh *mesh;
	unsigned int lifetime;
	D3DCOLOR cor;
	bool gravity;
public:
	Carcaca(void);
	inline void setCor(const D3DCOLOR cor) 
	{ this->cor = cor; }
	inline D3DCOLOR getCor() const
	{ return this->cor; }
	inline void setGravity(const bool gravity)
	{ this->gravity = gravity; }
	inline bool getGravity() const
	{ return this->gravity; }
	inline void setMesh(CMesh *mesh)
	{ this->mesh = mesh; }
	inline CMesh *getMesh()
	{ return this->mesh; }
	inline void setLifetime(unsigned int lifetime)
	{ this->lifetime = GameTicks + lifetime; }
	inline unsigned int getLifetime()
	{ return this->lifetime - GameTicks; }
	bool Update();
	void Render();
	~Carcaca(void);
};
