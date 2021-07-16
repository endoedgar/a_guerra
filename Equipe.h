#ifndef EQUIPE_H
#define EQUIPE_H

#pragma once

#include "3DTexture.h"
#include "SmartLinkedList.h"
#include "DataManager.h"

class CEquipe : public ExternalData {
	unsigned int id;
	CTexture *bandeira;
	D3DCOLOR color;
public:
	CEquipe():bandeira(NULL)
	{ }

	void setID(const unsigned int id)
	{ this->id = id; }

	unsigned int getID() const
	{ return this->id; }

	D3DCOLOR getColor() const
	{ return this->color; }

	void setColor(const D3DCOLOR color)
	{ this->color = color; }

	const CTexture *getFlag() const
	{ return this->bandeira; }

	void setFlag(CTexture *bandeira)
	{ this->bandeira = bandeira; }

	bool Unload()
	{
		SafeRelease(bandeira); 
		return true;
	}

	~CEquipe()
	{ this->Unload(); }

	//bool load(const Smart::String &file);
};

class EquipeDataManager : public DataManager {
public:
	EquipeDataManager():DataManager("EquipeDataManager", "equipes", "ini")
	{ }
	CEquipe *LoadData(const Smart::String &filename);
	inline CEquipe *find(const Smart::String &name)
	{ return static_cast<CEquipe*>(DataManager::find(name)); }
	inline CEquipe *operator[](const size_t i) const
	{ return static_cast<CEquipe*>(this->getLista()[i]); }

	void OnPostLoad();
};

extern EquipeDataManager *eDM;
//extern Smart::LinkedList<CEquipe> *g_Equipes;
//CEquipe *FindNation(const Smart::String &nation);
#endif