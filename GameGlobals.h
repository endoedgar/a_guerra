#ifndef _GAMEGLOBALS_H
#define _GAMEGLOBALS_H

using namespace Matematica;

#define NOMENUS
#define NOICONS
#define NOKANJI
#define NOHELP
#define NOMCX

#pragma once

class ParticleSystem;
class CTerrain;
class CJogador;
class Entity;
class MCIPlayer;
class CSound;
class D3DWrapper;
class Carcaca;
class CTexture;
class LineRender;
class CMesh;
class DadosDoSpark;

extern class CMissao *mymission;
extern unsigned int GameTicks, game_over, mission_complete;
extern LPDIRECT3DDEVICE9 d3dDevice;
extern LPDIRECT3D9 d3dObject;
extern D3DWrapper *mywrapper;
extern HWND hWnd;	
extern CTerrain *terrain;
extern ParticleSystem *ps;
extern class CPlane *mar;
extern unsigned int keys[256];				// Buffer for keys
extern CJogador *g_Jogador;
extern Entity *g_World;
extern Entity *g_piv;
extern real objetivoStartSec;
extern size_t CurrentObjective;
extern CSimpleListAP<Carcaca> *g_Carcacas;
extern unsigned int mouse_click, rmouse_click, casualities;
//extern class MiniMapa *mm;

#ifdef _DEBUG
//#define _ENABLEDEBUGPOINTS
#endif

#ifdef _ENABLEDEBUGPOINTS
extern Entity *g_PointOne, *g_PointTwo;
#define NTERRAINPOINTS 2200
extern Entity *g_TerrainPoints;
#endif

extern unsigned int dbgcounter;
extern MCIPlayer *mci;
extern D3DXMATRIX mIdentity;
extern real logic_t, render_t;
extern unsigned int HardwareThreads;
extern LPD3DXFONT g_Font; //font object
extern const D3DXFONT_DESC FontDesc;
extern RECT FontPosition;
extern LPD3DXSPRITE fontsprlayer;
extern LPD3DXLINE lr;
extern HINSTANCE g_hInst;
extern CTexture *limpact, *basecptimg, *baseperdimg, *estamortoimg, *esquadraoimg, *menosesquadraoimg;
extern CTexture *imgDamage, *imgLockingOn, *imgLockOn;
extern CTexture *imgGround, *imgHeli, *imgAero, *imgImobile;
extern CTexture *imgMenu, *imgMenuItemIniciar, *imgMenuItemSobre, *imgMenuItemSair, *imgSobreTexto, *imgSelEquipeTexto, *imgHudPanel, *imgHudBar;
#define NPORTRAITS 2
extern CTexture *imgSobrePortraits[NPORTRAITS];
extern DadosDoSpark *explodRuim;
extern CMesh *mshCuboid;
extern TEXTMETRIC gTxtmetrics;
extern Smart::String gamedir;
extern unsigned int ObjetivoMudou;

extern class CMissao *mymission;

enum _KEY_STATE{
	KEY_UNPRESSED,
	KEY_HIT,
	KEY_PRESSED
};

enum _GAMESTATE {
	GS_START,
	GS_MENU,
	GS_SOBRE,
	GS_SELECTOBJ,
	GS_SELECIONARMISSAO,
	GS_SELECTTEAM,
	GS_GAMESTARTUP,
	GS_GAME,
	GS_EXIT
};

extern _GAMESTATE g_GameState;

real GetTime();
//inline clock_t GetTime()
//{
//	return clock();
//}
extern CTexture *crosshairicon, *targeticon, *bomtiro;

class GameConfig {
private:
	unsigned int graphics_width, graphics_height;
	unsigned int precache_particlesn, precache_emittersn, precache_particlespackn;
	unsigned int nthreads, maxterraintri;
	real rdt;
	size_t nUnits;
	real terrainpriority, tdistfactor;
	bool zbuffer, zbuffer32;
	_D3DSWAPEFFECT sweffect;
public:
	//unsigned char AmbientLightR, AmbientLightG, AmbientLightB;
	GameConfig();
	~GameConfig();

	inline real getRazaoDeDanoDeTerreno(void) const
	{ return this->rdt; }

	inline unsigned int GetGraphicsWidth() const
	{ return this->graphics_width; }

	inline unsigned int GetGraphicsHeight() const
	{ return this->graphics_height; }

	inline unsigned int GetNThreads() const
	{ return this->nthreads; }

	inline unsigned int GetPrecacheEmitters() const
	{ return this->precache_emittersn; }

	inline unsigned int GetPrecacheParticles() const
	{ return this->precache_particlesn; }

	inline unsigned int GetPrecacheParticlesPack() const
	{ return this->precache_particlespackn; }

	inline unsigned int GetMaxTerrainTri() const
	{ return this->maxterraintri; }

	inline real GetTerrainPriority() const
	{ return this->terrainpriority; }

	inline real GetTerrainDistFactor() const
	{ return this->tdistfactor; }

	inline size_t GetMaxUnits() const
	{ return this->nUnits; }

	inline _D3DSWAPEFFECT GetSwapEffect() const
	{ return this->sweffect; }

	inline bool IsZBuffer() const
	{ return this->zbuffer; }

	inline bool IsZBuffer32() const
	{ return this->zbuffer32; }

	HRESULT LoadConfig(const Smart::String &filename);
};

extern D3DZBUFFERTYPE zbuffer;
extern GameConfig *gc;
extern class GameHUD *gh;
extern bool ShowHUD;
extern CSound *radio_start, *radio_end;
extern CSound *ui_move, *ui_select;
extern class MasterDataManager mdm;
//extern LineRender *lr;
HRESULT Render(CJogador *);
char *fixslashs(char *);
Smart::String &fixslashs(Smart::String &str);
extern Vector g_Gravidade;

#endif