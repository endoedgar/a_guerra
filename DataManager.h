#ifndef _DATAMANAGER_H
#define _DATAMANAGER_H

#include "SmartString.h"
#include "NamedEntity.h"

#pragma once

class ExternalData : public NamedEntity {
public:
	ExternalData()
	{ }

	virtual bool Unload(void) = 0;
	virtual ~ExternalData()
	{ TRACE("%p ~ExternalData() called!\n", this); }
};

class DataManager : public NamedEntity{
	std::vector<ExternalData*> lista;
	DataManager(DataManager &);
	DataManager &operator=(DataManager &);
	Smart::String path;
	Smart::String ext;
	
	void LoadFilesFromDir(const Smart::String &dir);
public:
	DataManager(const Smart::String &name, const Smart::String &path, const Smart::String &ext);
	virtual ~DataManager();
	void Unload();

	virtual ExternalData *LoadData(const Smart::String &filename) = 0;
	ExternalData *find(const Smart::String &name) const;
	void LoadAllData(void);
	virtual void OnPostLoad()
	{ }

	inline std::vector<ExternalData*> &getLista()
	{ return this->lista; }

	inline const std::vector<ExternalData*> &getLista() const
	{ return this->lista; }

	inline size_t getSize() const
	{ return this->getLista().size(); }

	inline ExternalData *operator[](const size_t i) const
	{ return this->getLista()[i]; }
};

class MasterDataManager {
	std::vector<DataManager*> lista;
public:
	MasterDataManager();
	~MasterDataManager();
	DataManager *find(const Smart::String &name) const;
	HRESULT add(DataManager *dm);
	HRESULT LoadAll();
	void Unload();
};

#endif