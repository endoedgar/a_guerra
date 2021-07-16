#include "stdafx.h"
#include "GameGlobals.h"
#include "loader.h"
#include "mciplayer.h"
#include "GenericFunctions.h"
#include "objeto.h"
#include "3DSprite.h"
#include "INI Parser.h"
#include "missao.h"
#include "ManterDir.h"

ExplodDataManager *eDataManager = NULL;
UnitDataManager *oDataManager = NULL;
ProjectileDataManager *pDataManager = NULL;
MissionDataManager *mDataManager = NULL;

bool CPhysicEntityData::carregarAtributosFisicos(const INIParser::INIFile &myfile)
{
	if(myfile.isOpened()) {
		const Smart::String SecaoFisica("física");

		this->massa = myfile.Get<real>(SecaoFisica, "massa", 1);
		if(equal(this->massa, 0))
			this->massa = 99999999;
		this->inertiaTensor = myfile.Get<real>(SecaoFisica, "inertiaTensor", 1);
		if(equal(this->inertiaTensor, 0))
			this->inertiaTensor = 99999999;
		this->inertiaTensor *= this->massa;

		GetVector(&this->com, myfile, SecaoFisica, "centerOfMass");
		return true;
	}
	return false;
}
//std::vector<CProjectileData *> CProjectileData::pdados;

/*int CExplodCallback(const Smart::String &filename, int attrib, void *param)
{
	INIParser::INIFile myini;
	if(myini.open(filename)) {
		SPARKDADOS *tmp = SPARKDADOS::Load(myini);
		if(tmp)
			explods.push_back(tmp);
	}
	return 0;
}*/

bool DadosDaUnidade::Unload()
{
	TRACE("Apagando dados da unidade %s %s...\n", this->simbolo.c_str(), this->getNome().c_str());
	if(this->meshTextures) {
		for(register unsigned int a = 0; a < eDM->getSize()+1; ++a) {
			SafeRelease(this->meshTextures[a]);
		}
		SafeDelete(this->meshTextures);
	}
	SafeRelease(this->Mesh);
	
	SafeRelease(this->iconehud);
	SafeRelease(this->radarimg);
	SafeDelete(this->midpoint);
	SafeDeleteArray(this->p);
	
	SafeDelete(this->shootspot);
	return true;
}

bool DadosDaUnidadeTerrestre::Unload() {
	SafeRelease(this->Turret);
	return DadosDaUnidade::Unload();
}

void DadosDaUnidade::carregarTexturas(const INIParser::INIFile &ini)
{
	const Smart::String texturasdir("texturas\\"), tequipesdir("texturas\\equipe\\"), SecaoTexturas("texturas");
	Smart::String tmp;
	CTexture *texturapadraomesh = NULL;
	CEquipe *equipeatual;
	register unsigned int a;

	this->meshTextures = new CTexture*[eDM->getSize()+1];

	tmp = texturasdir;
	tmp += ini.GetString(SecaoTexturas, "mesh");

	texturapadraomesh = CTexture::Load(tmp);

	this->meshTextures[0] = texturapadraomesh;
	if(texturapadraomesh)
		texturapadraomesh->AddRef();

	for(a = 0; a < eDM->getSize(); ++a) {
		equipeatual = (*eDM)[a];
		tmp = tequipesdir;
		tmp += equipeatual->getNome();
		tmp += '\\';
		tmp += ini.GetString(SecaoTexturas, "mesh");

		CTexture *pTex = NULL;
		pTex = pTex->Load(tmp);
		if(pTex) {
			this->meshTextures[a+1] = pTex;
			pTex->AddRef();
		} else {
			this->meshTextures[a+1] = texturapadraomesh;
			if(texturapadraomesh)
				texturapadraomesh->AddRef();
		}
		SafeRelease(pTex);
	}
	SafeRelease(texturapadraomesh);
}

DadosDaUnidade::DadosDaUnidade():meshTextures(NULL), Mesh(NULL), iconehud(NULL), p(NULL),
shootspot(NULL), midpoint(NULL), arma(NULL), ExplodDestruido(NULL), ExplodMorrendo(NULL), ExplodFumacaCarcaca(NULL), ExplodCarcacaVoando(NULL), ExplodArmourUp(NULL),
ctipo(NULL), radarimg(NULL)
{ }

DadosDaUnidadeTerrestre::DadosDaUnidadeTerrestre():DadosDaUnidade(), Turret(NULL)
{ }

/*DadosDoObjeto *ObjectDataManager::find(const Smart::String &name) {
	for(register size_t a = 0; a < this->getLista().size(); ++a) {
		DadosDoObjeto *dO = static_cast<DadosDoObjeto*>(this->getLista()[a]);
		if(dO->codename == name)
			return dO;
	}
	return NULL;
}*/

DadosDaUnidade *UnitDataManager::LoadData(const Smart::String &filename)
{
	DadosDaUnidade *obj = NULL;
	INIParser::INIFile ini;
	CManterDir cmd;

	if(ini.open(filename)) {
		SetCurrentDirectory(gamedir.c_str());
		const Smart::String SecaoGeral("geral"), SecaoGraficos("graficos"), SecaoMovimento("movimento"), SecaoArsenal("arsenal"), SecaoTexturas("texturas"), SecaoIa("ia"), SecaoExplods("explods");
		const Smart::String tipo = ini.GetString(SecaoGeral, "subtipo").lowcase(), SecaoCaixaDeColisao("caixadecolisao");
		Smart::String MeuMesh, tmp, tmp2;

		if(ini.Get<int>(SecaoGeral, "disable") || ini.GetString(SecaoGeral, "tipo") != "Objeto")
			goto fail;
		
		if(tipo == "aeronave") {
			obj = new DadosDaUnidadeVoadora;
			obj->ctipo = g_Objtypes[0];
			obj->tipo = TIPO_AERONAVE;
		} else if(tipo == "veiculo") {
			obj = new DadosDaUnidadeTerrestre;
			obj->ctipo = g_Objtypes[1];
			obj->tipo = TIPO_VEICULO;
		} else if(tipo == "helicoptero") {
			obj = new DadosDaUnidadeVoadora;
			obj->ctipo = g_Objtypes[2];
			obj->tipo = TIPO_HELICOPTERO;
		} else {
			RuntimeError("Objeto %s com tipo desconhecido! (%s)", ini.GetFilename().c_str(), tipo.c_str());
		}
		TRACE("Carregando %s...\n", filename.c_str());
		obj->setRadar(ini.Get<real>(SecaoGeral, "radar", 1000));
		obj->crew = ini.Get<unsigned int>(SecaoGeral, "crew", 1);
		obj->armour = ini.Get<real>(SecaoGeral, "armour", 100.0f);
		obj->setNome(ini.GetString(SecaoGeral, "nome"));
		obj->pontos = ini.Get<int>(SecaoGeral, "pontos", 1);
		obj->nstealth = ini.Get<int>(SecaoGeral, "nstealth", 0);
		obj->simbolo = ini.GetString(SecaoGeral, "simbolo");
		obj->descricao = ini.GetString(SecaoGeral, "texto");
		obj->impactabsorb = ini.Get<real>(SecaoGeral, "impact.absorb", 1);
		obj->hud = ini.Get<int>(SecaoGeral, "hud", 1);
		GetVector(&obj->fpcamera, ini, SecaoGeral, "fpcamera");
		obj->recoil = D3DXToRadian(ini.Get<real>(SecaoArsenal, "varia", real(0.0)));
		obj->shootspot = new COORD3D;
		GetVector(obj->shootspot, ini, SecaoArsenal, "posicao");
		tmp = ini.GetString(SecaoGeral, "tamanho");
		obj->setRadius(GetSub<real>(tmp.c_str(), 0)*r(1.2));
		obj->suspLength = ini.Get<real>(SecaoGeral, "suspensao.comp", 2);
		obj->suspInicio = ini.Get<real>(SecaoGeral, "suspensao.inicio", 1);
		obj->suspensionK = ini.Get<real>(SecaoGeral, "suspensao", r(0.02));
		//obj->suspensionDamping = ini.Get<real>(SecaoGeral, "suspensaoD", r(0.95));
		obj->suspensionD = ini.Get<real>(SecaoGeral, "suspensaoEL", r(0.95));
		if(!obj->carregarAtributosFisicos(ini))
			goto fail;
		if(obj->getRadius() <= 0)
			goto fail;

		MeuMesh = "modelos\\";
		MeuMesh += ini.GetString(SecaoGraficos, "mesh");
		obj->Mesh = obj->Mesh->Load(MeuMesh);
		obj->carregarTexturas(ini);

		tmp = "imagens\\";
		tmp += "icones\\";
		tmp += ini.GetString(SecaoGraficos, "icone");
		obj->iconehud = obj->iconehud->Load(tmp);
		tmp = "imagens\\";
		tmp += "radar\\";
		tmp += ini.GetString(SecaoGraficos, "radarimg");
		obj->radarimg = obj->iconehud->Load(tmp);
		/*obj->Mesh->Texture( tmp.c_str());
		if(obj->tipo == TIPO_ESTRUTURA)
		{
			obj->Turret->Texture(tmp2.c_str());
		}*/

		//EntityShininess Obj\mesh, Float GetIniString(file$, "graficos", "brilho", "0")
		switch(obj->tipo)
		{
			case TIPO_AERONAVE:
				obj->p = new COORD3D[4];
				obj->p[0].x = ini.Get<real>(SecaoGraficos, "tx1");
				obj->p[0].y = ini.Get<real>(SecaoGraficos, "ty1");
				obj->p[0].z = ini.Get<real>(SecaoGraficos, "tz1");
				obj->p[1].x = ini.Get<real>(SecaoGraficos, "tx2");
				obj->p[1].y = ini.Get<real>(SecaoGraficos, "ty2");
				obj->p[1].z = ini.Get<real>(SecaoGraficos, "tz2");
				GetVector(&obj->p[2], ini, SecaoGraficos, "asa1");
				GetVector(&obj->p[3], ini, SecaoGraficos, "asa2");
				break;
			case TIPO_VEICULO:
			case TIPO_HELICOPTERO:
				obj->p = new COORD3D[4];
				GetVector(&obj->p[0], ini, SecaoMovimento, "a1");
				GetVector(&obj->p[1], ini, SecaoMovimento, "a2");
				GetVector(&obj->p[2], ini, SecaoMovimento, "a3");
				GetVector(&obj->p[3], ini, SecaoMovimento, "a4");
				break;
		};
		obj->midpoint = new Vector(sqrt(obj->getRadius())/2, sqrt(obj->getRadius())/2,sqrt(obj->getRadius())/2);
		if(obj->Mesh) {
			D3DXMATRIX mTmp;
			Vector vMin, vMax;
			obj->Mesh->center();
			obj->Mesh->GetMeshMinMaxDimensions(&vMin, &vMax);
			obj->meshCol.cubo(vMin, vMax);
			obj->Mesh->GetMeshDimensions(obj->midpoint);
			*obj->midpoint /= 2;
		}
		
		obj->acel = ini.Get<real>(SecaoMovimento, "acel", r(0.1))*2;
		obj->macel = ini.Get<real>(SecaoMovimento, "max_acel", r(0.1))*2;
		if(equal(obj->acel, 0) || equal(obj->macel, 0))
			obj->setImovel(true);
		else
			obj->setImovel(false);
		obj->hv = D3DXToRadian(ini.Get<real>(SecaoMovimento, "turno_h", 1))/10;
		obj->vv =  D3DXToRadian(ini.Get<real>(SecaoMovimento, "turno_v", 1))/10;

		GetVector(&obj->scale, ini, SecaoGraficos, "scale", Vector(1, 1, 1));

		if(obj->carregarEspecifico(ini)) {
			//obj->AISafeAlt = ini.Get<int>(SecaoIa, "altitude_segura", 1000);
			obj->AIFiringRange = ini.Get<int>(SecaoIa, "min_dist", 0);
			obj->AIFiringRange *= obj->AIFiringRange;
			obj->AILockOnAngle = D3DXToRadian(ini.Get<real>(SecaoIa, "shootonangle", 45));

			obj->arman = ini.GetString(SecaoArsenal, "arma").lowcase();
			obj->stock = ini.Get<int>(SecaoArsenal, "stock", 1);
			obj->shootdelay = ini.Get<int>(SecaoArsenal, "shootdelay", 1);
			obj->ExplodDestruido = eDataManager->find(ini.GetString(SecaoExplods, "destruicao"));
			obj->ExplodMorrendo = eDataManager->find(ini.GetString(SecaoExplods, "destruindo"));
			obj->ExplodFumacaCarcaca = eDataManager->find(ini.GetString(SecaoExplods, "carcaca"));
			obj->ExplodCarcacaVoando = eDataManager->find(ini.GetString(SecaoExplods, "carcacavoando"));
			obj->ExplodArmourUp = eDataManager->find(ini.GetString(SecaoExplods, "conserto"));
			obj->ExplodColisao = eDataManager->find(ini.GetString(SecaoExplods, "colisao"));
			obj->qCarcaca = ini.Get<unsigned int>(SecaoExplods, "qCarcaca", 0);
			const real mysqrt = obj->scale.length();
			if(obj->Mesh) {
				D3DXMATRIX mTmp;
				CQuaternion qt;
				MeshColisao mcTemp;

				obj->Mesh->Scale(&obj->scale);
				
				D3DXMatrixScaling(&mTmp, obj->scale.x, obj->scale.y, obj->scale.z);
				
				qt.identity();
				obj->meshCol.transformed(mTmp, qt, mcTemp);

				obj->meshCol = mcTemp;
				obj->meshCol.subirVertices(obj->suspLength, obj->suspInicio);
			}
			if(obj->p)
				for(register unsigned int a = 0; a < 4; ++a) {
					obj->p[a].y -= 1.8;
					obj->p[a] *= obj->scale;
				}
			obj->setRadius(obj->getRadius() * mysqrt);
			//obj->radar *= mysqrt;
			//obj->acel *= mysqrt;
			//obj->macel *= mysqrt;
			//obj->AISafeAlt *= mysqrt;
			*obj->midpoint *= obj->scale;
			*obj->shootspot *= obj->scale;
			TRACE(" - OK\n");
			return obj;
		}
	}
fail:
	SafeDelete(obj);
	TRACE(" - Falha\n");
	return NULL;
}

bool DadosDaUnidadeTerrestre::carregarEspecifico(const INIParser::INIFile &ini) {
	Smart::String MeuTurret, SecaoTorre("torre"), SecaoMovimento("movimento");

	this->tforce = ini.Get<real>(SecaoTorre, "forca", 1);
	this->fRecoilForceAbsorb = ini.Get<real>(SecaoTorre, "RecoilForceAbsorb", r(0.05));

	MeuTurret = "modelos\\";
	MeuTurret += ini.GetString("graficos", "turret");
	
	this->tminpitch = D3DXToRadian(ini.Get<real>(SecaoTorre, "minpitch", 0));
	this->tmaxpitch = D3DXToRadian(ini.Get<real>(SecaoTorre, "maxpitch", 0));
	this->tmove = D3DXToRadian(ini.Get<real>(SecaoTorre, "movimentacao", 0));
	this->resistenciaAr = ini.Get<real>(SecaoMovimento, "resAr", static_cast<real>(0.005));
	this->friccaoVectorial = GetVector(&this->friccaoVectorial, ini, SecaoMovimento, "friccao", Vector(1, 1, 0.05));
	if(this->tmove == 0)
		this->tmove = this->hv;

	if(MeuTurret.GetLength() > strlen("modelos\\")+1)
	{
		if(this->tipo == TIPO_VEICULO)
		{
			Smart::String tmp;
			this->Turret = this->Turret->Load(MeuTurret.charstr());
			GetVector(&this->turretpos, ini, SecaoTorre, "pos");
		}
	}
	else
	{
		// Carregar Child
	}

	if(this->Turret)
		this->Turret->Scale(&this->scale);
	if(this->Mesh)
		this->turretpos += this->Mesh->getCenterDisplacement();
	this->turretpos *= this->scale;

	return true;
}

bool CProjectileData::Unload(void)
{
	if(this) {
		TRACE("Apagando dados do projetil %s...\n", this->getNome().c_str());
		SafeRelease(this->SLaunch);
		SafeRelease(this->SImpact);
		SafeRelease(this->hudicon);
		SafeRelease(this->mesh);
		return true;
	}
	return false;
}

CProjectileData::CProjectileData():SLaunch(NULL), SImpact(NULL), hudicon(NULL), mesh(NULL)
{
}

CProjectileData *ProjectileDataManager::LoadData(const Smart::String &filename) {
	CProjectileData *obj = NULL;
	INIParser::INIFile myfile;
	CManterDir cmd;

	if(myfile.open(filename)) {
		SetCurrentDirectory(gamedir.c_str());
		const Smart::String SecaoGeral("geral"), SecaoGraficos("graficos"), SecaoMovimento("movimento"), SecaoExplods("explods"), SecaoSons("sons"), SecaoIA("ia");
		const Smart::String subtipo = myfile.GetString(SecaoGeral, "subtipo").lowcase();
		const Smart::String tipo = myfile.GetString(SecaoGraficos, "tipo").lowcase();
		Smart::String tmp, lshell;
		Smart::String myscale;
		int subtipoi;

		if(subtipo == "canhao" || subtipo == "canhão")
			subtipoi = TIPO_CANHAO;
		else if(subtipo == "missel" || subtipo == "míssel")
			subtipoi = TIPO_MISSEL;
		else if(subtipo == "laser")
			subtipoi = TIPO_LASER;
		else if(subtipo == "bomba")
			subtipoi = TIPO_BOMBA;
		else if(subtipo == "mirv")
			subtipoi = TIPO_MIRV;
		else if(subtipo == "pesado")
			subtipoi = TIPO_PESADO;
		else
			goto fail;

		switch(subtipoi) {
			case TIPO_MISSEL:
			case TIPO_MIRV:
				obj = new CHotSeekingProjectileData;
				break;
			default:
				obj = new CProjectileData;
		}

		obj->setNome(myfile.GetString(SecaoGeral, "nome"));
		obj->shootdelay = myfile.Get<int>(SecaoGeral, "demora");

		obj->size = myfile.Get<real>(SecaoGeral, "tamanho");
		obj->sizesq = obj->size * obj->size;
		//obj->size *= obj->size;
		obj->gas = myfile.Get<int>(SecaoGeral, "gas");
		obj->damage = myfile.Get<real>(SecaoGeral, "dano");

		tmp = "sons\\";
		tmp += myfile.GetString(SecaoSons, "disparo");
		obj->SLaunch = obj->SLaunch->Load(tmp);
		obj->SLaunchT = TSDataManager->find(myfile.GetString(SecaoSons, "disparo.tipo"));
		tmp = "sons\\";
		tmp += myfile.GetString(SecaoSons, "impacto");
		obj->SImpact = obj->SImpact->Load(tmp);
		obj->SImpactT = TSDataManager->find(myfile.GetString(SecaoSons, "impacto.tipo"));
		tmp = "imagens\\";
		tmp += myfile.GetString(SecaoGraficos, "iconehud");
		obj->hudicon = obj->hudicon->Load(tmp);

		if(!obj->carregarAtributosFisicos(myfile))
			goto fail;

		obj->subtipo = subtipoi;

		if(tipo == "mesh" || tipo == "feixe") {
			if(tipo == "mesh")
				obj->gtipo = GTIPO_MESH;
			else
				obj->gtipo = GTIPO_FEIXE;
			lshell = myfile.GetString(SecaoGraficos, "mesh");
			tmp = "modelos\\";
		} else if(tipo == "sprite") {
			obj->gtipo = GTIPO_SPRITE;
			lshell = myfile.GetString(SecaoGraficos, "sprite");
			tmp = "texturas\\";
		} else
			obj->gtipo = GTIPO_NONE;

		
		tmp += lshell;

		switch(obj->gtipo)
		{
			case GTIPO_FEIXE:
			case GTIPO_MESH: {
				CMesh *mymesh = NULL;
				mymesh = mymesh->Load(tmp);
				obj->mesh = mymesh;//CMesh::Load(tmp.c_str());
				break;
			}
			case GTIPO_SPRITE: {
				CSprite *myspr = NULL;
				myspr = myspr->Load(tmp.c_str());
				if(!myspr)
					goto fail;
				obj->mesh = myspr;//CMesh::Load(tmp.c_str());
				break;
			}
		}

		// UNDONE: Scale mesh
		myscale = myfile.GetString(SecaoGraficos, "scale", "1 1 1");
		D3DXMatrixScaling(&obj->mMatrix, GetSub<FLOAT>(myscale.c_str(), 0), GetSub<FLOAT>(myscale.c_str(), 1), GetSub<FLOAT>(myscale.c_str(), 2)); 

		if(obj->gtipo == GTIPO_MESH || obj->gtipo == GTIPO_FEIXE)
		{
			Smart::String textura = "texturas\\";
			textura += myfile.GetString(SecaoGraficos, "textura");
			CTexture *tex = NULL;
			tex = tex->Load(textura) ;

			if(tex)
				obj->mesh->Texture(tex);
		}

		obj->acel = myfile.Get<real>(SecaoMovimento, "acel", 1);
		obj->macel = myfile.Get<real>(SecaoMovimento, "max_acel", 1);
		if(obj->carregaEspecifico(myfile)) {
			obj->tforca = myfile.Get<real>(SecaoMovimento, "forca", 0);
			obj->ExplodDisparo = eDataManager->find(myfile.GetString(SecaoExplods, "disparo"));
			obj->ExplodImpacto = eDataManager->find(myfile.GetString(SecaoExplods, "impacto"));
			obj->ExplodFim = eDataManager->find(myfile.GetString(SecaoExplods, "fim"));
			obj->ExplodRastro = eDataManager->find(myfile.GetString(SecaoExplods, "rastro"));
			obj->targetType = 0;

			if(myfile.Get<int>(SecaoIA, "alvochao", 1))
				obj->targetType |= ALVO_CHAO;
			if(myfile.Get<int>(SecaoIA, "alvoar", 1))
				obj->targetType |= ALVO_AR;
			if(myfile.Get<int>(SecaoIA, "alvoheli", 1))
				obj->targetType |= ALVO_HELICOPTERO;
			return obj;
		}
	}
fail:
	SafeDelete(obj);
	return NULL;
}

bool CHotSeekingProjectileData::carregaEspecifico(const INIParser::INIFile &myfile)
{
	const Smart::String SecaoMovimento("movimento");
	this->mirvno = max(1, myfile.Get<int>("geral", "mul", 2));
	this->tvalue = D3DXToRadian(myfile.Get<real>(SecaoMovimento, "turno"));
	this->trate = D3DXToRadian(this->tvalue/360)/1000;
	this->disthoming = myfile.Get<int>(SecaoMovimento, "disthoming");
	this->disthoming *= this->disthoming; // Para não precisar usar sqrt
	this->setAngleHoming(D3DXToRadian(myfile.Get<real>(SecaoMovimento, "anglehoming", 45)));
	this->homingtime = myfile.Get<int>(SecaoMovimento, "homingtime", 0);
	this->lockOnTime = myfile.Get<int>(SecaoMovimento, "lockontime", 120);
	return true;
}

CMissaoDados *MissionDataManager::operator[](const size_t i)
{ return static_cast<CMissaoDados*>(this->getLista()[i]); }

/*void CarregarArquivo(const Smart::String &filename)
{
	INIParser::INIFile myfile;
	if(myfile.open(filename)) {
		Smart::String tipo = myfile.GetString("geral", "tipo").lowcase();

		if(tipo == "objeto")
		{
			DadosDoObjeto *tmp = DadosDoObjeto::Load(myfile);
			if(tmp)
				odados.push_back(tmp);
		}
	}
}*/

/*void LiberarObjetos()
{
	for(register unsigned int i = 0; i < odados.size(); ++i)
	{
		DadosDoObjeto *o = odados[i];
		o->Unload();
	}
}*/

/*void LiberarProjeteis()
{
	for(register unsigned int i = 0; i < CProjectileData::pdados.size(); ++i)
	{
		CProjectileData *p = CProjectileData::pdados[i];
		p->Unload();
	}
}*/

/*int CTSonsCallBack(const Smart::String &filename, int attrib, void *param)
{
	Smart::LinkedList<CTipoDeSom>::LinkedNode *ce = g_TSons->LInsert(new CTipoDeSom);
	if(!(*ce)->carregar(filename))
		g_TSons->LRemove();
	return 0;
}

void CarregarTSonsDaPasta(const Smart::String &pastapath)
{
#ifdef UNICODE
	std::wstring buffer;
	std::wostringstream stream;
#else
	std::string buffer;
	std::ostringstream stream;
#endif
	TRACE("Procurando tipos de sons na pasta: %s\n", pastapath.c_str());
	stream << pastapath.c_str() << "\\*.ini";
	buffer = stream.str();
	for_each_file_ex(buffer.c_str(),  0, FA_DIREC, CTSonsCallBack, NULL); 
}*/

#include "Equipe.h"

/*int CEquipeCallback(const Smart::String &filename, int attrib, void *param)
{
	Smart::LinkedList<CEquipe>::LinkedNode *ce = g_Equipes->LInsert(new CEquipe);
	if(!(*ce)->load(filename))
		g_Equipes->LRemove();
	return 0;
}

void CarregarEquipesDaPasta(const Smart::String &pastapath)
{
#ifdef UNICODE
	std::wstring buffer;
	std::wostringstream stream;
#else
	std::string buffer;
	std::ostringstream stream;
#endif
	TRACE("Procurando equipes da pasta: %s\n", pastapath.c_str());
	stream << pastapath.c_str() << "\\*.ini";
	buffer = stream.str();
	for_each_file_ex(buffer.c_str(),  0, FA_DIREC, CEquipeCallback, NULL); 
}*/

//int CArqCallback(const Smart::String &filename, int attrib, void *param)
//{
//	CarregarArquivo(filename);
//	return 0;
//}

/*void CarregarExplodsDaPasta(const Smart::String &pastapath)
{
#ifdef UNICODE
	std::wstring buffer;
	std::wostringstream stream;
#else
	std::string buffer;
	std::ostringstream stream;
#endif
	TRACE("Procurando explods da pasta: %s\n", pastapath.c_str());
	stream << pastapath.c_str() << "\\*.txt";
	buffer = stream.str();
	for_each_file_ex(buffer.c_str(),  0, FA_DIREC, CExplodCallback, NULL); 
}*/

/*void CarregarArquivosDaPasta(const Smart::String &pastapath)
{
#ifdef UNICODE
	std::wstring buffer;
	std::wostringstream stream;
#else
	std::string buffer;
	std::ostringstream stream;
#endif
	TRACE("Procurando arquivos da pasta: %s\n", pastapath.c_str());
	stream << pastapath.c_str() << "\\*.txt";
	buffer = stream.str();
	for_each_file_ex(buffer.c_str(),  0, FA_DIREC, CArqCallback, NULL); 
}*/

HRESULT SistemaDeCarregamentoDeSpark(const INIParser::INIFile &myini, const int i, SparkUnit *spark)
{
	Smart::String imagem, secao, buffer, tmp;

	secao = IntToStr(i);

	spark->fxflags = myini.Get<int>(secao, "fxflags", 3);
	spark->blendflags = myini.Get<int>(secao, "blendflags", 1);

	//LoadTexture(imagem);
	imagem = myini.GetString(secao, "img");
	buffer = "texturas\\";
	buffer += imagem.c_str();
	spark->textura = spark->textura->Load(buffer);
	if(!spark->textura)
	{
		RuntimeError("O spark %s não conseguiu a imagem %s!", myini.GetFilename().c_str(), imagem.c_str());
		return E_FAIL;
	}

	const Smart::String dvalor("0 0");
	tmp = myini.GetString(secao, "x", dvalor);
	spark->min.x = GetSub<real>(tmp, 0);
	spark->max.x = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "y", dvalor);
	spark->min.y = GetSub<real>(tmp, 0);
	spark->max.y = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "z", dvalor);
	spark->min.z = GetSub<real>(tmp, 0);
	spark->max.z = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "xv", dvalor);
	spark->minv.x = GetSub<real>(tmp, 0);
	spark->maxv.x = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "yv", dvalor);
	spark->minv.y = GetSub<real>(tmp, 0);
	spark->maxv.y = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "zv", dvalor);
	spark->minv.z = GetSub<real>(tmp, 0);
	spark->maxv.z = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "startuv", "0 0");
	spark->startuv.x = GetSub<real>(tmp, 0);
	spark->startuv.y = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "enduv", "1 1");
	spark->enduv.x = GetSub<real>(tmp, 0);
	spark->enduv.y = GetSub<real>(tmp, 1);

	spark->alpha_vel = myini.Get<real>(secao, "animvel");
	spark->alpha_vel /= 255.0f;

	spark->alpha_acel = myini.Get<real>(secao, "animacel");
	spark->alpha_acel /= 255.0f;

	spark->cor_vel = myini.Get<real>(secao, "cor_vel", 1);
	spark->cor_acel = myini.Get<real>(secao, "cor_acel");

	tmp = myini.GetString(secao, "startscale", dvalor);
	spark->minstartscale = GetSub<real>(tmp, 0);
	spark->maxstartscale = GetSub<real>(tmp, 1);

	tmp = myini.GetString(secao, "scalevel", dvalor);
	spark->minscalevel = GetSub<real>(tmp, 0);
	spark->maxscalevel = GetSub<real>(tmp, 1);

	unsigned int r, g, b;

	tmp = myini.GetString(secao, "cor", "255 255 255");
	r = GetSub<unsigned int>(tmp, 0) & 0xFF;
	g = GetSub<unsigned int>(tmp, 1) & 0xFF;
	b = GetSub<unsigned int>(tmp, 2) & 0xFF;

	spark->cor = (r << 16) + (g << 8) + b;
	return S_OK;
}

SparkUnit *CarregarSpark(const INIParser::INIFile &myini, const int i, DadosDoSpark *sparkdados)
{
	char secao[128], tipo[128];
	SparkUnit *spark;
	_itoa_s(i, secao, sizeof(secao), 10);
	strcpy_s(tipo, sizeof(tipo), myini.GetString(secao, "t").charstr());
	_strlwr_s(tipo, strlen(tipo)+1);
	

	spark = new SparkUnit;

	if(!strcmp(tipo, "spark")) spark->tipo = 0;
	else if(!strcmp(tipo, "copiar")) spark->tipo = 1;
	else if(!strcmp(tipo, "emissor")) spark->tipo = 2;
	else {
		RuntimeError("Spark %d do arquivo %s tem um tipo desconhecido! (%s)", i, myini.GetFilename().c_str(), tipo);
		return NULL;
	}

	if(spark->tipo == 0) SistemaDeCarregamentoDeSpark(myini, i, spark);
	else if(spark->tipo == 1) {
		int c = myini.Get<int>(secao, "c");
		SparkUnit *original = sparkdados->sparks[c];
		spark->fxflags = original->fxflags;
		spark->blendflags = original->blendflags;
		spark->textura = original->textura;
		if(spark->textura)
			spark->textura->AddRef();
		spark->cor = original->cor;
		spark->min = original->min;
		spark->max = original->max;
		spark->minv = original->minv;
		spark->maxv = original->maxv;
		spark->cor_vel = original->cor_vel;
		spark->cor_acel = original->cor_acel;
		spark->alpha_vel = original->alpha_vel;
		spark->alpha_acel = original->alpha_acel;
		spark->minstartscale = original->minstartscale;
		spark->maxstartscale = original->maxstartscale;
		spark->minscalevel = original->minscalevel;
		spark->maxscalevel = original->maxscalevel;
		spark->emissor = original->emissor;
		spark->startuv = original->startuv;
		spark->enduv = original->enduv;
		if(spark->emissor)
			spark->emissor->AddRef();
		spark->tipo = original->tipo;
	}
	else if(spark->tipo == 2) {
		SistemaDeCarregamentoDeSpark(myini, i, spark);
		spark->emissor = new DadosDoEmissor;
		spark->emissor->lifetime = myini.Get<int>(secao, "lifetime");
		spark->emissor->intervalo = myini.Get<int>(secao, "intervalo");
	}
	else
	{
		RuntimeError("Tipo de elemento não conhecido em %s (%d)", myini.GetFilename().c_str(), i);
	}
	return spark;
}

DadosDoSpark *ExplodDataManager::LoadData(const Smart::String &filename)
{
	INIParser::INIFile myini;
	CManterDir cmd;

	if(myini.open(filename)) {
		const int ninteracoes = myini.Get<int>("Explod", "n");
		Smart::String nome = myini.GetString("Explod", "nome").lowcase();
		TRACE("Carregando explod %s...\n", nome.c_str());
		SetCurrentDirectory(gamedir.c_str());
		if(ninteracoes <= 0)
		{
			RuntimeError("O explod %s não contém um número de sparks válido! n = %d", nome.charstr(), ninteracoes);
			return NULL;
		}
		DadosDoSpark *sparkdados = new DadosDoSpark;
		if(sparkdados) {
			sparkdados->setNome(nome);
			sparkdados->ninteracoes = ninteracoes;
			sparkdados->dano = myini.Get<real>("Explod", "dano");
			sparkdados->raio = myini.Get<real>("Explod", "raio");
			sparkdados->raio *= sparkdados->raio;
			sparkdados->kiloForca = myini.Get<real>("Explod", "kiloForca", 0);
			sparkdados->sparks = new SparkUnit*[sparkdados->ninteracoes];
			for(register unsigned int i = 0; i < sparkdados->ninteracoes; ++i) {
				sparkdados->sparks[i] = CarregarSpark(myini, i, sparkdados);
			}
			return sparkdados;
		}
	}
	return NULL;
}

SparkUnit::~SparkUnit()
{
	if(this) {
		TRACE("\tApagando Spark %p\n", this);
		SafeRelease(this->textura);
		SafeRelease(this->emissor);
	}
}

bool DadosDoSpark::Unload()
{
	if(this->sparks) {
		TRACE("Apagando Explod %s...\n", this->getNome().c_str());
		for(register unsigned int i = 0; i < this->ninteracoes; ++i) {
			delete this->sparks[i];
		}
		delete [] this->sparks;
		this->sparks = NULL;
	}
	return true;
}

/*void LiberarExplods()
{
	for(register unsigned int i = 0; i < explods.size(); ++i)
	{
		ApagarExplod(explods[i]);
	}
}*/

bool DadosDaUnidadeVoadora::carregarEspecifico(const INIParser::INIFile &ini)
{
	this->setAISafeAlt(ini.Get<int>("ia", "altitude_segura", 1000));
	return true;
}

void UnitDataManager::OnPostLoad()
{
	TRACE("Procurando armas para as unidades: \n");
	for(register unsigned int a = 0; a < this->getLista().size(); ++a)
	{
		DadosDaUnidade *obj = static_cast<DadosDaUnidade*>(this->getLista()[a]);
		if(obj->arman.GetSize())
			obj->arma = pDataManager->find(obj->arman);
		else
			TRACE("%s %s...\t[SEM ARMA]\n", obj->simbolo.c_str(), obj->getNome().c_str());
	}
}

/*CProjectileData *ProjectileDataManager::find(const Smart::String &name)
{
	for(register unsigned int a = 0; a < this->getLista().size(); ++a)
	{
		CProjectileData *pro = static_cast<CProjectileData*>(this->getLista()[a]);
		if(!pro->fullname.cmpi(name))
			return pro;
	}
	return NULL;
}*/