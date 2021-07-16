#include "stdafx.h"
#include "TipoDeSom.h"
#include "GameGlobals.h"
#include "ManterDir.h"

TSomDataManager *TSDataManager;

/*CTipoDeSom *encontrarTSom(const Smart::String &nome)
{
	static CTipoDeSom padrao;
	if(g_TSons) {
		for(register Smart::LinkedList<CTipoDeSom>::LinkedNode *pPtr = g_TSons->GetFirst(); pPtr; pPtr = pPtr->GetNext()) { 
			if(!(*pPtr)->getNome().cmpi(nome))
				return &pPtr->GetData();
		}
	}
	return &padrao;
}*/

CTipoDeSom::CTipoDeSom()
{
	this->buffer.dwSize = sizeof(DS3DBUFFER);
	this->buffer.dwMode = DS3DMODE_NORMAL;
	this->buffer.flMinDistance = 150;
	this->buffer.flMaxDistance = 1000;
}

bool CTipoDeSom::Unload() {
	return true;
}

CTipoDeSom *TSomDataManager::LoadData(const Smart::String &filepath) { 
	INIParser::INIFile fp;
	CManterDir cmd;

	if(fp.open(filepath)) {
		CTipoDeSom *ts = NULL;
		const Smart::String geral = "geral";
		const Smart::String cone = "cone";
		const Smart::String vazio = "";
		Smart::String tmp;

		SetCurrentDirectory(gamedir.c_str());

		ts = new CTipoDeSom;

		ts->setNome(fp.GetString(geral, "nome", vazio));
		if(ts->getNome().GetLength() > 0) {
			tmp = fp.GetString(geral, "distancia", "0 0");
			ts->getBuffer().flMinDistance = GetSub<D3DVALUE>(tmp, 0);
			ts->getBuffer().flMaxDistance = GetSub<D3DVALUE>(tmp, 1);

			tmp = fp.GetString(cone, "angulos", "0 2");
			ts->getBuffer().dwInsideConeAngle = GetSub<DWORD>(tmp, 0);
			ts->getBuffer().dwOutsideConeAngle = GetSub<DWORD>(tmp, 1);

			ts->getBuffer().lConeOutsideVolume = fp.Get<LONG>(cone, "volume", 1);

			fp.close();
			return ts;
		}
		fp.close();
	}
	return NULL;
}