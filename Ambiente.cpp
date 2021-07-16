#include "stdafx.h"
#include "Ambiente.h"
#include "INI Parser.h"
#include "GenericFunctions.h"

AmbienteDataManager *adm;

Ambiente::Ambiente(void):ambientlight(0), ambientlightdir(0, 0, 0), skyboxname(""), skyboxext("")
{ }

Ambiente::~Ambiente(void)
{ }

AmbienteDataManager::AmbienteDataManager():DataManager("AmbienteDataManager", "ambientes", "txt")
{ }

Ambiente *AmbienteDataManager::LoadData(const Smart::String &filename)
{
	Ambiente *amb = NULL;
	INIParser::INIFile ini;
	if(ini.open(filename)) {
		const Smart::String SecaoGeral("geral"), SecaoLuz("luz");
		Smart::String tmp;
		amb = new Ambiente;
		amb->setNome(ini.GetString(SecaoGeral, "nome", "AmbienteSemNome"));
		tmp = ini.GetString(SecaoGeral, "ambiente", "200 200 200");
		amb->setAmbientLight(D3DCOLOR_XRGB(GetSub<unsigned int>(tmp, 0), GetSub<unsigned int>(tmp, 1), GetSub<unsigned int>(tmp, 2)));
		tmp = ini.GetString(SecaoLuz, "cor", "255 255 255");
		amb->setAmbientLightColor(D3DCOLOR_XRGB(GetSub<unsigned int>(tmp, 0), GetSub<unsigned int>(tmp, 1), GetSub<unsigned int>(tmp, 2)));
		tmp = ini.GetString(SecaoLuz, "amb", "20 20 20");
		amb->setAmbientLightAmb(D3DCOLOR_XRGB(GetSub<unsigned int>(tmp, 0), GetSub<unsigned int>(tmp, 1), GetSub<unsigned int>(tmp, 2)));
		tmp = ini.GetString(SecaoLuz, "dir", "0 0 0");
		amb->setAmbientLightDir(Vector(GetSub<real>(tmp, 0), GetSub<real>(tmp, 1), GetSub<real>(tmp, 2)));
		tmp = ini.GetString(SecaoGeral, "skybox", "skyboxpadrao jpg");
		amb->setSkyboxName(GetSubString(tmp, 0));
		amb->setSkyboxExt(GetSubString(tmp, 1));
		tmp = ini.GetString(SecaoGeral, "gravidade", "0 -0.0234 0");
		amb->setGravityVel(Vector(GetSub<real>(tmp, 0), GetSub<real>(tmp, 1), GetSub<real>(tmp, 2)));

		return amb;
	}
	SafeDelete(amb);
	return amb;
}

//Ambiente