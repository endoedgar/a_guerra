#include "stdafx.h"
#include "3DSound.h"
#include "GameGlobals.h"
#include "defines.h"
#include "3DMath.h"
#include "GenericFunctions.h"
#include "globalheader.h"
#include "GameHUD.h"
#include "SmartPtr.h"
#include "INI Parser.h"

//LineRender *lr = NULL;
LPD3DXLINE lr = NULL;
GameConfig *gc = NULL;
real objetivoStartSec = 0;
size_t CurrentObjective = 0;
D3DZBUFFERTYPE zbuffer = D3DZB_USEW;
_GAMESTATE g_GameState = GS_START;
GameHUD *gh;
CSound *radio_start = NULL, *radio_end = NULL;
CSound *ui_move = NULL, *ui_select = NULL;

LPD3DXFONT g_Font = NULL; //font object
const D3DXFONT_DESC FontDesc = {
						  20,
                          12,
                          0,
                          0,
                          false,
                          DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS,
                          DEFAULT_PITCH,
						  "Arial"};
RECT FontPosition;

Vector g_Gravidade(0, -0.0234, 0);

LPD3DXSPRITE fontsprlayer = NULL;
CSimpleListAP<Carcaca> *g_Carcacas;
TEXTMETRIC gTxtmetrics;

unsigned int HardwareThreads = 2;
bool ShowHUD = false;
// Objects
LPDIRECT3D9			  d3dObject = NULL;			// Direct3D Object
LPDIRECT3DDEVICE9	  d3dDevice = NULL;			// Direct3D Device
unsigned int dbgcounter = 0;
unsigned int mouse_click = 0, rmouse_click = 0;
unsigned int casualities = 0;

//LPDIRECT3DQUERY9		d3dQuery;				// The query
D3DWrapper *mywrapper;
Smart::String gamedir;
MasterDataManager mdm;

real logic_t = 0, render_t = 0;

HWND hWnd = NULL;									// HWND of the window

CJogador *g_Jogador = NULL;
Entity *g_World = NULL;

#ifdef _ENABLEDEBUGPOINTS
Entity *g_PointOne = NULL, *g_PointTwo = NULL;
Entity *g_TerrainPoints = NULL;
#endif

Entity *g_piv = NULL;
//MiniMapa *mm = NULL;

unsigned int GameTicks = 0;
unsigned int keys[256];				// Buffer for keys
MCIPlayer *mci = NULL;
D3DXMATRIX mIdentity;
CMissao *mymission = NULL;
unsigned int game_over = 0, mission_complete = 0;
CTexture *crosshairicon = NULL, *targeticon = NULL;
CTexture *limpact = NULL, *basecptimg = NULL, *bomtiro = NULL, *baseperdimg = NULL, *estamortoimg = NULL,
	*esquadraoimg = NULL, *menosesquadraoimg = NULL;
CTexture *imgDamage = NULL, *imgLockingOn = NULL, *imgLockOn = NULL;
CTexture *imgGround = NULL, *imgHeli = NULL, *imgAero = NULL, *imgImobile = NULL;
CTexture *imgMenu = NULL, *imgMenuItemIniciar = NULL, *imgMenuItemSobre = NULL, *imgMenuItemSair = NULL, *imgSobreTexto = NULL, *imgSobrePortraits[NPORTRAITS] = {NULL, NULL}, *imgSelEquipeTexto = NULL;
CMesh *mshCuboid = NULL;
CTexture *imgHudPanel = NULL, *imgHudBar = NULL;
DadosDoSpark *explodRuim = NULL;
//-----------------------------------------------------------------------------
// Name: GetTime()
// Desc: Returns the current program time, using QueryPerformanceCounter
//-----------------------------------------------------------------------------
real GetTime()
{
	real time;

	static bool		counterCreated = false;
	static LONGLONG ticksPerSec = 0;
	LARGE_INTEGER	absTime;

	if( counterCreated == false )
	{
		LARGE_INTEGER qwTicksPerSec;
		QueryPerformanceFrequency( &qwTicksPerSec );
		ticksPerSec = qwTicksPerSec.QuadPart;

		counterCreated = true;
	}
	
	QueryPerformanceCounter( &absTime );

	time = absTime.QuadPart / static_cast<real>(ticksPerSec);
	
	return time;
}

Smart::String &fixslashs(Smart::String &str)
{
	for(register size_t i = 0; i < str.GetLength(); ++i) {
		if(str[i] == '/')
			str[i] = '\\';
	}
	return str;
}

char *fixslashs(char *str)
{
	ASSERT(str)
	for(register char *t = str; *t; ++t) {
		if(*t == '/')
			*t = '\\';
	}
	return str;
}

GameConfig::GameConfig():graphics_width(0), graphics_height(0), nthreads(0)
{
}

GameConfig::~GameConfig()
{
}

HRESULT GameConfig::LoadConfig(const Smart::String &filename)
{
	Smart::String tmp;
	INIParser::INIFile myfile;

	if(myfile.open(filename)) {
		int tmpi;
		tmp = myfile.GetString("geral", "resolucao", "640 480");
		this->graphics_width = max(1, GetSub<unsigned int>(tmp.c_str(), 0));
		this->graphics_height = max(1, GetSub<unsigned int>(tmp.c_str(), 1));

		this->nthreads = max(1, myfile.Get<int>("geral", "nthreads", 2));
		this->precache_particlesn = max(0, myfile.Get<int>("precache", "particulas", 2000));
		this->precache_emittersn = max(0, myfile.Get<int>("precache", "emissores", 200));
		this->precache_particlespackn = max(0, myfile.Get<int>("precache", "particulaspack", 50));
		this->rdt = max(0, myfile.Get<real>("geral", "razaoDeDanoDeTerreno", 1000));
		this->nUnits = max(0, myfile.Get<size_t>("geral", "maxUnits", 20));
		//this->AmbientLightR = static_cast<unsigned char>(myfile.Get<int>("ambientlight", "r", 255));
		//this->AmbientLightG = static_cast<unsigned char>(myfile.Get<int>("ambientlight", "g", 255));
		//this->AmbientLightB = static_cast<unsigned char>(myfile.Get<int>("ambientlight", "b", 127));
	
		this->maxterraintri = max(0, myfile.Get<int>("terreno", "maxtri", 5000));
		this->terrainpriority = max(0, myfile.Get<real>("terreno", "prioridadenodulo", r(0.1000)));
		this->tdistfactor = myfile.Get<real>("terreno", "distfator", r(1));
		tmpi = myfile.Get<int>("geral", "sweffect", 1);
		tmpi = max(1, min(5, tmpi));
		this->sweffect = static_cast<_D3DSWAPEFFECT>(tmpi);
		this->zbuffer = myfile.Get<bool>("geral", "zbuffer", true);
		this->zbuffer32 = myfile.Get<bool>("geral", "zbuffer32", true);
		return S_OK;
	}

	return E_FAIL;
}