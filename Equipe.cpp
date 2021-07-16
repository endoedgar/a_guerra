#include "stdafx.h"
#include "3DTexture.h"
#include "Equipe.h"
#include "GenericFunctions.h"
#include "INI Parser.h"
#include "GameGlobals.h"
#include "ManterDir.h"

EquipeDataManager *eDM;

void EquipeDataManager::OnPostLoad() {
	for(register unsigned int a = 0; a < this->getSize(); ++a) {
		(*this)[a]->setID(a);
	}
}
CEquipe *EquipeDataManager::LoadData(const Smart::String &filename)
{
	INIParser::INIFile myfile;
	CManterDir cmd;

	if(myfile.open(filename)) {
		Smart::String tmp;
		CEquipe *equipe = new CEquipe;
		SetCurrentDirectory(gamedir.c_str());

		equipe->setNome(myfile.GetString("equipe", "nome", "Sem nome"));
		tmp = "imagens\\";
		tmp += "bandeiras\\";
		tmp += myfile.GetString("equipe", "bandeira");
		equipe->setFlag(CTexture::Load(tmp));
		tmp = myfile.GetString("equipe", "cor", "255 255 255");
		equipe->setColor(D3DCOLOR_XRGB(GetSub<int>(tmp, 0), GetSub<int>(tmp, 1), GetSub<int>(tmp, 2)));
		return equipe;
	}
	return NULL;
}

/*CEquipe *FindNation(const Smart::String &nation)
{
	if(g_Equipes) {
		for(register Smart::LinkedList<CEquipe>::LinkedNode *pPtr = g_Equipes->GetFirst(); pPtr; pPtr = pPtr->GetNext()) { 
			if(!(*pPtr)->getNome().cmpi(nation))
				return &pPtr->GetData();
		}
	}
	return NULL;
}*/