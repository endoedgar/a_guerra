#pragma once

#include "DataManager.h"
#include "Vector.h"

using namespace Matematica;
class Ambiente : public ExternalData
{
	D3DCOLOR ambientlightcolor, ambientlight,ambientlightamb;
	Vector ambientlightdir, gravityVel;
	Smart::String skyboxname, skyboxext;
public:
	inline void setGravityVel(const Vector &gravityVel)
	{ this->gravityVel = gravityVel; }
	inline const Vector &getGravityVel(void) const
	{ return this->gravityVel; }
	inline void setAmbientLightAmb(const D3DCOLOR ambientlight)
	{ this->ambientlightamb = ambientlight; }
	inline D3DCOLOR getAmbientLightAmb(void) const
	{ return this->ambientlightamb; }
	inline void setAmbientLightColor(const D3DCOLOR ambientlight)
	{ this->ambientlightcolor = ambientlight; }
	inline D3DCOLOR getAmbientLightColor(void) const
	{ return this->ambientlightcolor; }
	inline void setAmbientLight(const D3DCOLOR ambientlight)
	{ this->ambientlight = ambientlight; }
	inline D3DCOLOR getAmbientLight(void) const
	{ return this->ambientlight; }
	inline void setAmbientLightDir(const Vector &ambientlightdir)
	{
		this->ambientlightdir = ambientlightdir;
		this->ambientlightdir.normalize();
	}
	inline const Vector &getAmbientLightDir(void) const
	{ return this->ambientlightdir; }
	inline void setSkyboxName(const Smart::String skyboxname)
	{ this->skyboxname = skyboxname; }
	inline const Smart::String &getSkyboxName(void) const
	{ return this->skyboxname; }
	inline void setSkyboxExt(const Smart::String skyboxext)
	{ this->skyboxext = skyboxext; }
	inline const Smart::String &getSkyboxExt(void) const
	{ return this->skyboxext; }
	bool Unload(void)
	{ return true; }
	Ambiente(void);
	~Ambiente(void);
};

class AmbienteDataManager : public DataManager {
public:
	AmbienteDataManager();
	Ambiente *LoadData(const Smart::String &filename);
	Ambiente *find(const Smart::String &name) const
	{ return static_cast<Ambiente*>(DataManager::find(name)); }

	Ambiente *operator[](const size_t i) const
	{ return static_cast<Ambiente*>(this->getLista()[i]); }
	//~AmbienteDataManager();
};

extern AmbienteDataManager *adm;