#ifndef _TIPO_DE_SOM_H
#define _TIPO_DE_SOM_H

#include "INI Parser.h"
#include "GenericFunctions.h"
#include "DataManager.h"

#pragma once

class CTipoDeSom : public ExternalData {
private:
	DS3DBUFFER buffer;
public:
	CTipoDeSom();

	inline DS3DBUFFER &getBuffer()
	{ return this->buffer; }

	inline const DS3DBUFFER &getBuffer() const
	{ return this->buffer; }
	
	bool Unload();
};

class TSomDataManager : public DataManager {
public:
	TSomDataManager():DataManager("TSomDataManager", "sons\\tipos", "ini")
	{ }
	CTipoDeSom *LoadData(const Smart::String &filename);
	inline CTipoDeSom *find(const Smart::String &name)
	{
		static CTipoDeSom padrao;
		CTipoDeSom *ctds = static_cast<CTipoDeSom*>(DataManager::find(name));
		return (ctds) ? (ctds) : &padrao;
	}
	inline CTipoDeSom *operator[](const size_t i) const
	{ return static_cast<CTipoDeSom*>(this->getLista()[i]); }
};

//CTipoDeSom *encontrarTSom(const Smart::String &nome);

extern TSomDataManager *TSDataManager;

#endif