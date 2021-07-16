#ifndef LOADER_H
#define LOADER_H
#include "3DTexture.h"
#include "CMesh.h"
#include "3DSound.h"
#include "INI Parser.h"
#include "DataManager.h"
#include "Plane.h"
#include "MeshColisao.h"

class CMissaoDados;
typedef Vector COORD3D;

enum Tipos { 
	TIPO_CANHAO,
	TIPO_PESADO, // Artilharia
	TIPO_MISSEL,
	TIPO_LASER,
	TIPO_BOMBA,
	TIPO_MIRV,

	TIPO_NENHUM,
	TIPO_AERONAVE,
	TIPO_VEICULO,
	TIPO_HELICOPTERO,

	GTIPO_MESH,
	GTIPO_SPRITE,
	GTIPO_FEIXE,
	GTIPO_NONE
};

enum Tipo_Musica {
	MUSICA_TITULO,
	MUSICA_JOGO,
	MUSICA_MAX
};
#define ALVO_CHAO 1
#define ALVO_AR 2
#define ALVO_HELICOPTERO 4

class CPhysicEntityData {
	real massa, inertiaTensor;
	Vector com; // Center of mass (Centro da massa)
public:
	bool carregarAtributosFisicos(const INIParser::INIFile &myfile);

	inline real getMassa(void) const
	{ return this->massa; }

	inline real getInertiaTensor(void) const
	{ return this->inertiaTensor; }

	inline const Vector &getCOM(void) const
	{ return this->com; }
};

class CProjectileData : public ExternalData, public CPhysicEntityData {
public:
	//Smart::String fullname;//, *Mesh;
	int gtipo, subtipo, gas, shootdelay, targetType;
	real acel, macel, size, sizesq, damage, tforca;
	C3DSound *SLaunch, *SImpact;
	CTipoDeSom *SLaunchT, *SImpactT;
	CTexture *hudicon;
	D3DXMATRIX mMatrix;

	CVisibleEntity *mesh;
	//COORD3D *turretpos, *p1, *p2, *p3, *p4, *shootspot;
	class DadosDoSpark *ExplodDisparo, *ExplodImpacto, *ExplodFim, *ExplodRastro;

	CProjectileData();
	~CProjectileData() {
		this->Unload();
		TRACE("%p ~CProjectileData() called!\n", this);
	}
	virtual bool carregaEspecifico(const INIParser::INIFile &myfile)
	{ return  myfile.isOpened(); }
	virtual bool Unload();
};

class CHotSeekingProjectileData : public CProjectileData {
	real anglehoming, anglehomingsq;
public:
	int disthoming, homingtime, lockOnTime, mirvno;
	real tvalue, trate;

	inline void setAngleHoming(const real anglehoming)
	{
		this->anglehoming = anglehoming;
		this->anglehomingsq = this->anglehoming;
		this->anglehomingsq *= this->anglehomingsq;
	}

	inline real getAngleHoming(void) const
	{ return this->anglehoming; }

	inline real getAngleHomingSq(void) const
	{ return this->anglehomingsq; }

	bool carregaEspecifico(const INIParser::INIFile &myfile);
};

class ProjectileDataManager : public DataManager {
public:
	ProjectileDataManager():DataManager("ProjectileDataManager", "objetos", "txt")
	{ }
	CProjectileData *LoadData(const Smart::String &filename);
	CProjectileData *find(const Smart::String &name) const
	{ return static_cast<CProjectileData*>(DataManager::find(name)); }

	CProjectileData *operator[](const size_t i) const
	{ return static_cast<CProjectileData*>(this->getLista()[i]); }
};


class DadosDoSpark : public ExternalData {
public:
	//Smart::String nome;
	unsigned int ninteracoes;
	real dano, raio, kiloForca;
	class SparkUnit **sparks;

	DadosDoSpark():sparks(NULL)
	{ }

	~DadosDoSpark()
	{ this->Unload(); }

	bool Unload();
};

class ExplodDataManager : public DataManager {
public:
	ExplodDataManager():DataManager("ExplodDataManager", "explods", "txt")
	{ }
	DadosDoSpark *LoadData(const Smart::String &myfile);

	inline DadosDoSpark *find(const Smart::String &nome) const
	{ return static_cast<DadosDoSpark*>(DataManager::find(nome)); }

	inline DadosDoSpark *operator[](const size_t i) const
	{ return static_cast<DadosDoSpark*>(this->getLista()[i]); }
};

class DadosDaUnidade : public ExternalData, public CPhysicEntityData {
	real radar, radarsq;
	real radius, radiusSq;
	bool imovel;
public:
	Smart::String simbolo, arman, descricao;// *Mesh, *Turret;
	int tipo, pontos, stock, AIFiringRange, nstealth, hud;
	real impactabsorb, suspLength, suspInicio, shootdelay;
	real armour, acel, macel, hv, vv, recoil, AILockOnAngle, suspensionK, suspensionD, fRecoilForceAbsorb;
	unsigned int qCarcaca, crew;
	Vector fpcamera, scale;
	MeshColisao meshCol;

	inline bool getImovel(void) const
	{ return this->imovel; }

	inline void setImovel(const bool imovel)
	{ this->imovel = imovel; }

	inline void setRadius(const real radius) {
		this->radius = radius;
		this->radiusSq = this->radius;
		this->radiusSq *= this->radiusSq;
	}

	inline real getRadius(void) const
	{ return this->radius; }

	inline real getRadiusSq(void) const
	{ return this->radiusSq; }

	CTexture** meshTextures;
	CMesh *Mesh;
	CTexture *iconehud, *radarimg;
	COORD3D *p, *shootspot, *midpoint;
	CProjectileData *arma;
	DadosDoSpark *ExplodDestruido, *ExplodMorrendo, *ExplodFumacaCarcaca, *ExplodCarcacaVoando, *ExplodArmourUp, *ExplodColisao;

	DadosDaUnidade();
	virtual bool carregarEspecifico(const INIParser::INIFile &ini)
	{ return ini.isOpened(); }

	virtual ~DadosDaUnidade()
	{ 
		this->Unload();
		TRACE("%p ~DadosDaUnidade() called!\n", this);
	}
	void carregarTexturas(const INIParser::INIFile &ini);
	virtual bool Unload();
	inline void setRadar(const real radar) {
		this->radar = radar;
		this->radarsq = this->radar;
		this->radarsq *= this->radarsq;
	}
	inline real getRadar(void) const
	{ return this->radar; }
	inline real getRadarSq(void) const
	{ return this->radarsq; }

	class CObjetoT *ctipo;


};

class DadosDaUnidadeTerrestre : public DadosDaUnidade {
public:
	CMesh *Turret;
	COORD3D turretpos;
	real tminpitch, tmaxpitch, tforce, tmove;
	real resistenciaAr;
	Vector friccaoVectorial;

	DadosDaUnidadeTerrestre();

	virtual bool carregarEspecifico(const INIParser::INIFile &ini);
	virtual bool Unload();
};

class DadosDaUnidadeVoadora : public DadosDaUnidade {
	int AISafeAlt;
public:
	inline void setAISafeAlt(const int AISafeAlt)
	{ this->AISafeAlt = AISafeAlt; }

	inline int getAISafeAlt(void) const
	{ return this->AISafeAlt; }

	virtual bool carregarEspecifico(const INIParser::INIFile &ini);
};

class UnitDataManager : public DataManager {
public:
	UnitDataManager():DataManager("UnitDataManager", "objetos", "txt")
	{ }
	DadosDaUnidade *LoadData(const Smart::String &filename);
	inline DadosDaUnidade *find(const Smart::String &name)
	{ return static_cast<DadosDaUnidade*>(DataManager::find(name)); }
	void OnPostLoad();
	inline DadosDaUnidade *operator[](const size_t i) const
	{ return static_cast<DadosDaUnidade*>(this->getLista()[i]); }
};

#include "missao.h"

class MissionDataManager : public DataManager {
public:
	MissionDataManager():DataManager("MissionDataManager", "missoes", "txt")
	{ }
	CMissaoDados *LoadData(const Smart::String &filename);
	void OnPostLoad();
	CMissaoDados *operator[](const size_t i);
};


class DadosDoEmissor {
private:
	~DadosDoEmissor()
	{ }
public:
	int lifetime, intervalo;
	unsigned int nRef;

	DadosDoEmissor():nRef(0)
	{ }

	void AddRef()
	{ ++this->nRef; }

	HRESULT Release()
	{
		if(this->nRef)
			--this->nRef;
		else
			delete this;
		return S_OK;
	}
};

class SparkUnit {
public:
	int tipo, fxflags, blendflags, cor;
	COORD3D min, max, minv, maxv;
	real alpha_vel, alpha_acel, minstartscale, maxstartscale, minscalevel, maxscalevel;
	real cor_vel, cor_acel;
	Vector startuv, enduv;

	DadosDoEmissor *emissor;
	CTexture *textura;

	SparkUnit():emissor(NULL), textura(NULL)
	{ }

	~SparkUnit();
};

typedef struct tagMusica {
	char *filepath;
	Tipo_Musica tipo;
} MUSICA;

//void CarregarTSonsDaPasta(const Smart::String &);
//void CarregarEquipesDaPasta(const Smart::String &);
//void CarregarExplodsDaPasta(const Smart::String &);
//void CarregarArquivosDaPasta(const Smart::String &);
//void LiberarExplods();
//void LiberarObjetos();
//void LiberarProjeteis();
//void AtualizaObjetos();

void TocarMusica(const Tipo_Musica tipo);
MUSICA *CarregarMusica(const Smart::String &filename, const Tipo_Musica tipo);
void CarregarMusicas(const Smart::String &path, const Tipo_Musica tipo);

extern ExplodDataManager *eDataManager;
extern UnitDataManager *oDataManager;
extern MissionDataManager *mDataManager;

#endif