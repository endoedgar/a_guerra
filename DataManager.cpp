#include "stdafx.h"
#include "ManterDir.h"
#include "DataManager.h"

DataManager::DataManager(const Smart::String &name, const Smart::String &path, const Smart::String &ext):NamedEntity(name), path(path), ext(ext)
{ }

DataManager::~DataManager()
{ this->Unload(); }

void DataManager::Unload() {
	for(register unsigned int a = 0; a < this->lista.size(); ++a)
	{
		if(this->lista[a]) {
			ExternalData *data = this->lista[a];
			data->Unload();
			delete data;
			this->lista[a] = NULL;
		}
	}
}

void DataManager::LoadAllData(void) {
	this->LoadFilesFromDir(this->path);
}

void DataManager::LoadFilesFromDir(const Smart::String &dir) {
#ifdef UNICODE
	std::wstring buffer;
	std::wostringstream stream;
#else
	std::string buffer;
	std::ostringstream stream;
#endif
	HANDLE h;
	WIN32_FIND_DATA info;
	CManterDir cmd;

	SetCurrentDirectory(dir.c_str());
	stream << "*." << ext.c_str();
	buffer = stream.str();

	h = FindFirstFile(buffer.c_str(), &info);

	if(h != INVALID_HANDLE_VALUE) {
		do {
			if(!(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ExternalData *ed = this->LoadData(info.cFileName);
				if(ed)
					this->lista.push_back(ed);
			} else
				this->LoadFilesFromDir(info.cFileName);
		} while(FindNextFile(h, &info));
	}
	FindClose(h);
}

ExternalData *DataManager::find(const Smart::String &name) const {
	for(register unsigned int a = 0; a < this->getSize(); ++a) {
		if(!((*this)[a])->getNome().cmpi(name))
			return (*this)[a];
	}
	return NULL;
}

MasterDataManager::MasterDataManager()
{ }

MasterDataManager::~MasterDataManager()
{
	this->Unload();
}

void MasterDataManager::Unload() {
	DataManager *dm;
	while(this->lista.size()) {
		dm = this->lista.back();
		delete dm;
		this->lista.pop_back();
	}
}

DataManager *MasterDataManager::find(const Smart::String &name) const {
	for(register unsigned int a = 0; a < this->lista.size(); ++a) {
		if(!this->lista[a]->getNome().cmpi(name))
			return this->lista[a];
	}
	return NULL;
}

HRESULT MasterDataManager::add(DataManager *dm)
{
	if(dm) {
		this->lista.push_back(dm);
	}
	return S_OK;
}

HRESULT MasterDataManager::LoadAll() {
	for(std::vector<DataManager*>::iterator i = lista.begin(); i < lista.end(); ++i) {
		(*i)->LoadAllData();
	}
	for(std::vector<DataManager*>::iterator i = lista.begin(); i < lista.end(); ++i) {
		(*i)->OnPostLoad();
	}
	return S_OK;
}