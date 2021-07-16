#include "stdafx.h"
#include "GameGlobals.h"
#include "CMesh.h"
#include "CTerrain.h"
#include "3DEntity.h"
#include "3DFunctions.h"
#include "3DParticles.h"
#include "3DSkybox.h"
#include "3DSound.h"
#include "3DPlane.h"
#include "GerenciadorDeRecursos.h"
#include "loader.h"
#include "mciplayer.h"
#include "objeto.h"
#include "projetil.h"
#include "3DCamera.h"
#include "Jogador.h"
#include "GenericFunctions.h"
#include "radio.h"
#include "resource.h"
#include "tship.h"
#include "testrutura.h"
#include "CTeclado.h"
#include "3DFisica.h"
#include "missao.h"
#include "GameHUD.h"
#include "SmartPtr.h"
#include "D3DObject.h"
#include "Bandeira.h"
#include "Equipe.h"
#include "Carcaca.h"
#include "MiniMapa.h"
#include "Helicoptero.h"
#include "Ambiente.h"
#include "LineRender.h"
#include "MeshColisao.h"
#include "Base.h"

//RigidBody body;
bool paused;
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//#define NUM_TREES 300

//#define AdjustRand( rand ) rand = rand < .5f ? -rand : rand;


const float period = 1000.0f/FPS;
int g_Wireframe = 0, g_Wait = 0;
bool g_pressed;
unsigned int GameTicksAlphaStarted = 0, ObjetivoMudou=0;
HINSTANCE g_hInst;


//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd);

HRESULT Render();

//HRESULT SetupOcclusion();

HRESULT SetupMatrices();
HRESULT SetupStates();

HRESULT Reset();
HRESULT SizeChange();

//HRESULT UpdateStats();

//float GetTime();
float GetRandom();
//inline float Distance( float x1, float y1, float z1, float x2, float y2, float z2 );
//void DistanceSort( std::vector<CObject> objectArray, int lBound, int uBound );



//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
// Variables

int frameCount = 1;	// Set to 1 to avoid any possible devide by 0
WNDCLASSEX wc;


//CCamera camera;					// Camera structure

//CMesh* terrain   = NULL;		// The terrain mesh
//CMesh* tree      = NULL;		// The tree mesh
//CMesh* treeBound = NULL;		// Bounding mesh for tree
//CMesh* skybox    = NULL;
//CTexture *tex = NULL;
//CTexture *pt = NULL;
CSkybox *skybox = NULL;
ParticleSystem *ps = NULL;
CTerrain *terrain = NULL;
CTexture *martex = NULL;

CPlane *mar = NULL;
BandeiraMesh *bmesh = NULL;
CMesh *MeshCubo = NULL;
unsigned int menuInicialCursor = 0;

std::stringstream	  ss;					// String writer stream



//-----------------------------------------------------------------------------
// Name: SetupOcclusion()
// Desc: Create the objects needed for the occlusion culling
//-----------------------------------------------------------------------------
/*HRESULT SetupOcclusion()
{
	// Create the query
	d3dDevice->CreateQuery( D3DQUERYTYPE_OCCLUSION, &d3dQuery );

	// Get the display mode to obtain the format
	D3DDISPLAYMODE mode;
	d3dDevice->GetDisplayMode( 0, &mode );

	return S_OK;
}*/

//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initialize graphics
//-----------------------------------------------------------------------------
HRESULT InitD3D(HWND hWnd)
{
	HRESULT hr;
	RECT rect;

	mywrapper = new D3DWrapper;
	hr = mywrapper->InitObject();
	if(SUCCEEDED(hr)) {
		hr = mywrapper->SetGFXMode(hWnd, gc->IsZBuffer(), gc->IsZBuffer32(), gc->GetSwapEffect());
		if(SUCCEEDED(hr)) {
			d3dDevice = mywrapper->GetDevice();

			//if( FAILED( SetupOcclusion() ) )
			//	return E_FAIL;

			FontPosition.top = 0;
			FontPosition.left = 0;
			FontPosition.right = mywrapper->d3dpp.BackBufferWidth;
			FontPosition.bottom = mywrapper->d3dpp.BackBufferHeight;
			D3DXCreateFontIndirect(d3dDevice,&FontDesc,&g_Font);

			//terrain = new CTerrain;
			//terrain->LoadTerrain("C:\\Projeto\\texturas\\mapa.png", d3dDevice);

			TRACE("Inicialização Direct3D feita com sucesso!\n\n");
		} else {
			RuntimeError("Deu alguma zebra na inicialização do D3D!\nCódigo do erro: %d", HRESULT_CODE(hr));
		}
	} else {
		RuntimeError("Deu alguma zebra na criação do objeto D3D!\nCódigo do erro: %d", HRESULT_CODE(hr));
	}
	return hr;
}


LRESULT CALLBACK SelectWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	switch(Message) {

		case WM_INITDIALOG:
		{
			//SetScrollRange(GetDlgItem(hwnd, IDC_SCROLLBAR1), odados.size(), 3, odados.size(), FALSE);
			//SendDlgItemMessage(hwnd, IDC_SCROLLBAR1, 0, SBM_SETRANGEREDRAW, odados.size());
			EndDialog(hwnd, 1);
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
					DestroyWindow(hwnd);
					break;
				case IDCANCEL:
					DestroyWindow(hwnd);
					break;
			}
			break;

		case WM_PAINT:
			hdc = BeginPaint(hwnd, &ps);
			//SetWindowText(hwnd, odados[GetDlgItemInt(hwnd, IDC_SCROLLBAR1, NULL, FALSE)]->codename.c_str());
			EndPaint(hWnd, &ps);
			break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;

		case WM_DESTROY:
			return 0;
	}
	return 0;
	//return DefDlgProc(hwnd, Message, wParam, lParam);
	//return DefWindowProc(hwnd, Message, wParam, lParam);
}

void DebugJournal::LOG(const char *msg, ...)
{
	static char buf[2048];
	ASSERT(msg)
	ASSERT(strlen(msg) <= sizeof(buf))

	/* todo, some day: use vsnprintf (C99) */
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end(ap);

	if(!t_output)
		t_output = new std::fstream();
	if(t_output)
	{
		if(!t_output->is_open()) {
			if(this->IsRecording)
				t_output->open("replayw.log", std::ios_base::out);
			else
				t_output->open("replayr.log", std::ios_base::out);
		}

		if(t_output->is_open())
		{
			if(g_Unidades->GetNumActive())
				*t_output << "Enemypos " << g_Unidades->GetFirstActive()->GetPositionX() << ' ' << g_Unidades->GetFirstActive()->GetPositionY() << ' ' << g_Unidades->GetFirstActive()->GetPositionZ() << ' ';
			*t_output << buf;
			t_output->flush();
		}
	}
}

#include "missao.h"

#ifdef UNICODE
	typedef std::wostringstream missionstream;
#else
	typedef std::ostringstream missionstream;
#endif


HRESULT CarregarMissao(const CMissaoDados *mdata) {
	HRESULT hr;
	Smart::String filepath = mdata->terrainPath;
	const Smart::String folder = "mapas\\" + mdata->mapaFilename + '\\';
	SafeRelease(skybox);
	SafeDelete(terrain);
	SafeDelete(mymission);
	mymission = new CMissao();

	terrain = new CTerrain();

	hr = terrain->LoadTerrain(filepath);
	if(SUCCEEDED(hr)) {
		filepath = mdata->terrainTexturePath;
		CTexture *tmp = CTexture::Load(filepath);
		if(tmp) {
			terrain->setTexture(tmp);
			tmp->Release();
			filepath = mdata->terrainDetailTexturePath;//folder+fmap.GetString(secaoMapas, "d");
			tmp = CTexture::Load(filepath);
			if(tmp) {
				terrain->setDetailTexture(tmp);
				tmp->Release();

				skybox = new CSkybox();
				hr = skybox->Load(mdata->getAmbiente()->getSkyboxName(), mdata->getAmbiente()->getSkyboxExt());
				if(FAILED(hr))
				{
					RuntimeError("Skybox %s não carregado!\nCódigo do Erro: %d", mdata->getAmbiente()->getSkyboxName().charstr(), HRESULT_CODE(hr));
					return hr;
				}
				mywrapper->SetAmbientColor(mdata->getAmbiente()->getAmbientLight());
				mywrapper->SetAmbColor(mdata->getAmbiente()->getAmbientLightAmb());
				terrain->Scale(&mdata->terrainScale);
				terrain->Position(&mdata->terrainPosition);
				terrain->friction = mdata->terrainFriction;
				mywrapper->SetLight(mdata->getAmbiente()->getAmbientLightColor(),  mdata->getAmbiente()->getAmbientLightDir());
				g_Gravidade = mdata->getAmbiente()->getGravityVel();

				mymission->Start(mdata);
				mywrapper->SetupStates();

				return S_OK;
			} else {
				missionstream ss;
				ss << "Textura de detalhe \"" << filepath.c_str() << "\" não encontrada!";
				CMissaoDados::lastError.set(ss.str().c_str());
			}
		} else {
			missionstream ss;
			ss << "Textura \"" << filepath.c_str() << "\" não encontrada!";
			CMissaoDados::lastError.set(ss.str().c_str());
		}
	} else {
		missionstream ss;
		ss << "Heightmap \"" << filepath.c_str() << "\" não encontrado!";
		CMissaoDados::lastError.set(ss.str().c_str());
	}
	return hr;
}

HRESULT LoadEverything()
{
	//CQuaternion tst;
	//tst.Euler(20, 20, 20);
	//D3DXQUATERNION eae;
	//D3DXQuaternionRotationYawPitchRoll(&eae, 20, 20, 20);
	//tst = tst + tst;
	//eae = eae + eae;
	HRESULT hr;
	TRACE("\n\n");
	TRACE("-------------------------\n");
	TRACE("Iniciando carregamento...\n");
	TRACE("-------------------------\n\n");
	unsigned int seed;
	seed = static_cast<unsigned>(time( NULL ));
	srand( seed );

	GlobalIniBuffer.Reserve(256);
	gamedir.Reserve(1024);
	GetCurrentDirectory(1024, gamedir.GetWritableBuffer());
	gamedir.UpdateLength();

		//TFormVector(&out, &out, &dummy1, NULL); 

	// Register the window class
	TRACE("Criando classe da janela...\n");
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = MsgProc;
	wc.lpszClassName = APP_TITLE.c_str();
	wc.hInstance = g_hInst;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	TRACE("Registrando classe da janela...\n");
	RegisterClassEx( &wc );

	// Create the application's window
	TRACE("Criando janela...\n");

	gc = new GameConfig();
	gc->LoadConfig("config.ini");
	hWnd = CreateWindow( APP_TITLE.c_str(), APP_TITLE.c_str(), 
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 0, 0, gc->GetGraphicsWidth(), gc->GetGraphicsHeight(),
						GetDesktopWindow(), NULL, wc.hInstance, NULL );

	// Show the window
	ShowWindow( hWnd, SW_SHOWDEFAULT );

	//g_UnidadesPtr = new CSimpleListAP<CObjeto*>();
	g_piv = new Entity;
#ifdef _ENABLEDEBUGPOINTS
	g_PointOne = new Entity;
	g_PointTwo = new Entity;
	g_TerrainPoints = new Entity[NTERRAINPOINTS];
#endif
#ifdef _PROFILE
	g_ProfileInfo = new ProfileInfo;
#endif
	D3DXMatrixIdentity(&mIdentity);

	hr = InitSoundSystem(hWnd);
	if(FAILED(hr))
		TRACE("Sistema de Som falhou...\n\tCódigo do Erro: %d\n", HRESULT_CODE(hr));
	hr = InicializaDInput();
	if(FAILED(hr))
		RuntimeError("Não foi possível iniciar o DirectInput!");

	g_World = new Entity();
	g_World->Position(0, 0, 0);

	//g_World->Rotate(D3DXToRadian(60), D3DXToRadian(-60), D3DXToRadian(-80));
	//g_World->Turn(D3DXToRadian(6), -D3DXToRadian(-6), -D3DXToRadian(-8));
	//TRACE("pitch %f yaw %f roll %f\n", D3DXToDegree(g_World->Pitch()), D3DXToDegree(g_World->Yaw()), D3DXToDegree(g_World->Roll()));

 	g_Objtypes = new CObjetoT*[3];
	g_Objtypes[0] = new CAirplane;
	g_Objtypes[1] = new CEstrutura;
	g_Objtypes[2] = new CHelicoptero;

	if( SUCCEEDED( InitD3D(hWnd) ) )
	{
		TRACE("Criando sprite 3D (fontes)...\n");
		hr = D3DXCreateSprite(d3dDevice, &fontsprlayer);
		if(FAILED(hr))
		{
			RuntimeError("Falha ao criar camada 2D de texto!\nCódigo do erro: %d", HRESULT_CODE(hr));
			return hr;
		}
		mar = new CPlane();

		mdm.add(new TSomDataManager);
		mdm.add(new EquipeDataManager);
		mdm.add(new ExplodDataManager);
		mdm.add(new UnitDataManager);
		mdm.add(new ProjectileDataManager);
		mdm.add(new AmbienteDataManager);
		mdm.add(new MissionDataManager);

		TSDataManager = static_cast<TSomDataManager*>(mdm.find("TSomDataManager"));
		eDM = static_cast<EquipeDataManager*>(mdm.find("EquipeDataManager"));
		eDataManager = static_cast<ExplodDataManager*>(mdm.find("ExplodDataManager"));
		oDataManager = static_cast<UnitDataManager*>(mdm.find("UnitDataManager"));
		pDataManager = static_cast<ProjectileDataManager*>(mdm.find("ProjectileDataManager"));
		adm = static_cast<AmbienteDataManager*>(mdm.find("AmbienteDataManager"));
		mDataManager = static_cast<MissionDataManager*>(mdm.find("MissionDataManager"));

		mdm.LoadAll();

		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, (DLGPROC)SelectWinProc);
		//oDataManager.OnPostLoad();
		//eDM.OnPostLoad();
		if(oDataManager->getLista().size() <= 0)
		{
			RuntimeError("Não há objetos carregados no jogo!\nVerifique se o executável está na pasta correta!");
			return E_UNEXPECTED;
		}
		g_Projeteis = new CObjectManager<CProjetil>();
		g_Projeteis->Precache(250);
		g_Unidades = new CObjectManager<CObjeto>();
		g_UnidadesPtr = new CSimpleListAP<CObjeto*>();
		g_Carcacas = new CSimpleListAP<Carcaca>();

		g_Font->GetTextMetrics(&gTxtmetrics);

		//g_Unidades->Precache(40);

		D3DVIEWPORT9 viewport;
		viewport.MaxZ = 1.0;
		viewport.MinZ = 0.0;
		viewport.Height = mywrapper->d3dpp.BackBufferHeight;
		viewport.Width = mywrapper->d3dpp.BackBufferWidth;
		viewport.X = 0;
		viewport.Y = 0;
		g_Jogador = new CJogador();
		g_Jogador->Criar(viewport, CONTROLE_TIPO_TECLADO);
		//mm = new MiniMapa(0, 0, 200, 200);
		
		mymission = new CMissao();

		mywrapper->SetupStates();
		g_Wait = 0;

		//terrain->Position(0, r(-(239/3)*2), 0);
		//terrain->Scale(19000, r(600), 19000);
		//ctex = ctex->Load("texturas\\texturas mapa.jpg");
		//detailtex = detailtex->Load("texturas\\detailmap.jpg");
		martex = martex->Load("texturas\\water.jpg");

#ifdef _ENABLEDEBUGPOINTS
		/*register unsigned int x = 0, y = 0;
		for(register unsigned int a = 0; a < NTERRAINPOINTS; ++a) {
			g_TerrainPoints[a].Position(x*2, 0, y*2);
			//g_TerrainPoints[a].Position(g_TerrainPoints[a].GetPositionX(), terrain->GetTerrainHeight(g_TerrainPoints[a].GetPosition()), g_TerrainPoints[a].GetPositionZ());
			g_TerrainPoints[a].temp = static_cast<unsigned char>(terrain->lasttri);
			++x;
			if(x >= 100) {
				x = 0;
				++y;
			}
		}*/
#endif
		gh = new GameHUD();
		gh->Setup(100);
		InicializaSomRadios();

		bmesh = new BandeiraMesh();
		bmesh->Setup(10, 6);
		bmesh->SetXFactor(10);
		bmesh->SetYFactor(4);
		bmesh->SetZFactor(r(0.05));
		bmesh->SetTFactor(5);

		MeshCubo = MeshCubo->Load("modelos\\cubo.x");
		MeshCubo->Scale(&Vector(0.01, 1.24, 0.01));

		MeshColisao mc;
		mc.getMesh(MeshCubo);

		radio_start = radio_start->Load("sons\\radio_start.wav");
		radio_end = radio_end->Load("sons\\radio_end.wav");
		ui_move = radio_start->Load("sons\\ui\\change.wav");
		ui_select = radio_end->Load("sons\\ui\\select.wav");

		D3DXCreateLine(d3dDevice, &lr);

		crosshairicon = CTexture::Load("imagens\\mira.png");
		imgMenu = CTexture::Load("imagens\\hud\\menu\\inicial\\tela.png");
		imgMenuItemIniciar = CTexture::Load("imagens\\hud\\menu\\inicial\\itemIniciar.png");
		imgMenuItemSobre = CTexture::Load("imagens\\hud\\menu\\inicial\\itemSobre.png");
		imgMenuItemSair = CTexture::Load("imagens\\hud\\menu\\inicial\\itemSair.png");
		imgSobreTexto = CTexture::Load("imagens\\hud\\menu\\sobre\\textoSobre.png");
		imgSobrePortraits[0] = CTexture::Load("imagens\\hud\\menu\\sobre\\logo_centro_paula_souza.jpg");
		imgSobrePortraits[1] = CTexture::Load("imagens\\hud\\menu\\sobre\\minas2_bigger.jpg");
		imgSelEquipeTexto = CTexture::Load("imagens\\hud\\menu\\equipe\\equipe.png");
		targeticon = CTexture::Load("imagens\\target.png");
		limpact = CTexture::Load("imagens/radar.png");
		basecptimg = CTexture::Load("imagens/hud/basecapt.png");
		bomtiro = CTexture::Load("imagens/hud/bomtiro.png");
		baseperdimg = CTexture::Load("imagens/hud/baseperd.png");
		estamortoimg = CTexture::Load("imagens/hud/estamorto.png");
		esquadraoimg = CTexture::Load("imagens/hud/maisumproesquadrao.png");
		menosesquadraoimg = CTexture::Load("imagens/hud/menosumproesquadrao.png");
		imgHudPanel = CTexture::Load("imagens/hud/hud.png");
		imgHudBar = CTexture::Load("imagens/hud/bar.png");
		imgDamage = CTexture::Load("imagens/hud/dano.png");
		imgLockingOn = CTexture::Load("imagens/lockingon.png");
		imgLockOn = CTexture::Load("imagens/lockon.png");

		imgGround = CTexture::Load("imagens/targets/chao.png");
		imgHeli = CTexture::Load("imagens/targets/heli.png");
		imgAero = CTexture::Load("imagens/targets/aero.png");
		imgImobile = CTexture::Load("imagens/targets/imovel.png");

		mshCuboid = mshCuboid->Load("modelos/cuboide.x");
		if(!crosshairicon || !targeticon || !limpact || !basecptimg || !bomtiro || !baseperdimg || !estamortoimg || !esquadraoimg || !menosesquadraoimg || !imgDamage || !imgLockingOn || !imgLockOn ||
			!imgGround || !imgHeli || !imgAero || !imgImobile || !imgHudPanel || !imgHudBar ||
			!mshCuboid || !imgMenu || !imgMenuItemIniciar || !imgMenuItemSobre || !imgMenuItemSair || !imgSobreTexto || !imgSobrePortraits[0] || !imgSobrePortraits[1] || !imgSelEquipeTexto) {
			RuntimeError("Falha ao carregar algumas frescuras!");
			return E_FAIL;
		}
	
		ps = new ParticleSystem();
		mci = new MCIPlayer(256);
		explodRuim = eDataManager->find("rostoruim");

		TRACE("\n");
		TRACE("------------------------\n");
		TRACE("Carregamento Finalizado!\n");
		TRACE("------------------------\n\n");
		return S_OK;
	}

	TRACE("\nCarregamento Falhou!\n");
	return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Destroy all the objects
//-----------------------------------------------------------------------------
HRESULT Cleanup(WNDCLASSEX *wc)
{
	TRACE("\n\n");
	TRACE("----------------\n");
	TRACE("Iniciando Faxina\n");
	TRACE("----------------\n\n");

	SafeDelete(g_piv);
	FinalizaDInput();
#ifdef _ENABLEDEBUGPOINTS
	SafeDeleteArray(g_TerrainPoints);
	SafeDelete(g_PointOne);
	SafeDelete(g_PointTwo);
#endif
	SafeRelease(g_Font);
#ifdef _PROFILE
	SafeDelete(g_ProfileInfo);
#endif
	SafeDelete(terrain);
	//SafeDelete(tex);
	//skybox->Destroy();
	SafeDelete( mymission );
	//SafeDelete( mm );
	SafeRelease( MeshCubo );
	SafeDelete( gc );
	SafeDelete( g_UnidadesPtr );
	//TSDataManager.Unload();
	SafeDelete( g_Carcacas );
	for(int x = 0; x < NPORTRAITS; ++x)
		SafeRelease(imgSobrePortraits[x]);
	SafeRelease(crosshairicon);
	SafeRelease( targeticon );
	SafeRelease(limpact);
	SafeRelease(mshCuboid);
	//SafeRelease(detailtex);
	SafeRelease(martex);
	SafeRelease(basecptimg);
	SafeRelease(bomtiro);
	SafeRelease(baseperdimg);
	SafeRelease(estamortoimg);
	SafeRelease(esquadraoimg);
	SafeRelease(menosesquadraoimg);
	SafeRelease(imgDamage);
	SafeRelease(imgMenu);
	SafeRelease(imgMenuItemIniciar);
	SafeRelease(imgMenuItemSobre);
	SafeRelease(imgMenuItemSair);
	SafeRelease(imgSobreTexto);
	SafeRelease(imgSelEquipeTexto);
	SafeRelease(imgHudPanel);
	SafeRelease(imgHudBar);
	//SafeRelease(ctex);
	SafeRelease(radio_start);
	SafeRelease(radio_end);
	SafeRelease(ui_move);
	SafeRelease(ui_select);
	SafeRelease(ps);
	SafeRelease(lr);
	SafeRelease(skybox);
	SafeRelease(gh);
	SafeDelete( g_World );
	SafeDelete( mci );
	SafeRelease(mar);
	SafeRelease(bmesh);
	SafeRelease(imgLockingOn);
	SafeRelease(imgLockOn);
	SafeRelease(imgAero);
	SafeRelease(imgHeli);
	SafeRelease(imgGround);
	SafeRelease(imgImobile);

	//if(jet)
	//	jet->Destroy();
	//SafeDelete( jet );

	//terrain->Destroy();

	SafeDelete(g_Jogador);
	//SafeDelete(pt);

	//tree->Destroy();
	//SafeDelete( tree );

	//treeBound->Destroy();
	//SafeDelete( treeBound );
	//SafeRelease( d3dQuery );

	//eDataManager.Unload();
	///oDataManager.Unload();
	//pDataManager.Unload();
	//LiberarProjeteis();
	//LimparRecursos();

	//eDM.Unload();
	mdm.Unload();
	SafeDelete(g_Projeteis);
	SafeDelete(g_Unidades);
	LiberarSomRadios();
	CleanSoundSystem();
	SafeRelease( fontsprlayer );
	SafeDelete(mywrapper);

	if(g_Objtypes) {
		for(register unsigned int i = 0; i < 3; ++i) {
			SafeDelete(g_Objtypes[i]);
		}
		delete [] g_Objtypes;
	}
	TRACE("\n-----------------\n");
	TRACE("Faxina Finalizada\n");
	TRACE("-----------------\n\n");

	TRACE("Destruindo janela...\n");
	CloseWindow(hWnd);
	DestroyWindow(hWnd);

	TRACE("Removendo registro da janela...\n");
	UnregisterClass( APP_TITLE.c_str(), wc->hInstance );

	TRACE("Fim!\n");

#ifdef _BETA
	if(t_output->is_open())
		t_output->close();
#endif

	return S_OK;
}

void UpdateListener()
{
	DS3DLISTENER dl;
	PROFILE_START;
	Vector tmp;
	dl.dwSize = sizeof(DS3DLISTENER);
	gListener->GetAllParameters(&dl);
	dl.vPosition.x = g_Jogador->GetCamera()->GetPosition(true)->x;
	dl.vPosition.y = g_Jogador->GetCamera()->GetPosition(true)->y;
	dl.vPosition.z = g_Jogador->GetCamera()->GetPosition(true)->z;
	dl.flDistanceFactor = 10;
	dl.vVelocity.x = g_Jogador->GetCamera()->GetVelocity()->x;
	dl.vVelocity.y = g_Jogador->GetCamera()->GetVelocity()->y;
	dl.vVelocity.z = g_Jogador->GetCamera()->GetVelocity()->z;
	tmp = Vector(0,0,1);
	tmp = tmp.Transformed(&g_Jogador->GetCamera()->mWorld);
	dl.vOrientFront.x = tmp.x;
	dl.vOrientFront.y = tmp.y;
	dl.vOrientFront.z = tmp.z;
	tmp = Vector(0,1,0);
	tmp = tmp.Transformed(&g_Jogador->GetCamera()->mWorld);
	dl.vOrientTop.x = tmp.x;
	dl.vOrientTop.y = tmp.y;
	dl.vOrientTop.z = tmp.z;
	//dl.vOrientFront.x = g_Jogador->GetCamera()->GetFrontVector()->x;
	//dl.vOrientFront.y = g_Jogador->GetCamera()->GetFrontVector()->y;
	//dl.vOrientFront.z = g_Jogador->GetCamera()->GetFrontVector()->z;
	gListener->SetAllParameters(&dl, DS3D_DEFERRED);
	gListener->CommitDeferredSettings();
	PROFILE_END;
}

void SpawnMissionObjects()
{
	for(register size_t w = 0; w < mymission->GetDados()->pGObjetos.GetCapacity(); ++w) {
		GrupoObjetoMissaoDados *gom = &mymission->GetDados()->pGObjetos[w];
		if(gom->objetivo == CurrentObjective) {
			for(register size_t b = 0; b < gom->nUnidades; ++b) {
				CObjeto *c = g_Unidades->New();
				Vector position = gom->unidades[b].getPosicao();
				position += mymission->GetDados()->hotspot;
				position.y += terrain->GetTerrainHeight(&position);
				c->Criar(&position, 0, gom->team, gom->unidades[b].getUD());
				c->ogm = &mymission->GetGOM(w);
				++(*c->ogm);
			}
		}
	}
}

void SpawnPlayer() {
	size_t count = 0;
	Vector pos;
	CObjeto *a;

	if(!g_Jogador->IsObjectValid()) {
		a = g_Unidades->New();
		if(g_Jogador->team) {
			for(register size_t a = 0; a < mymission->GetBases().GetCapacity(); ++a) {
				if(mymission->GetBases()[a].Dona() == g_Jogador->team)
					count++;
			}
		}
		if(count) {
			count = random(count);
			size_t b = 0;
			for(register size_t a = 0; a < mymission->GetBases().GetCapacity(); ++a) {
				if(mymission->GetBases()[a].Dona() == g_Jogador->team) {
					if(b == count) {
						pos = mymission->GetBases()[a].GetPosition();
						break;
					}
					b++;
				}		
			}
		} else
			pos = mymission->GetDados()->p1pos+mymission->GetDados()->hotspot;
		const real terrainy = terrain->GetTerrainHeight(&pos);
		if(pos.y < terrainy)
			pos.y = terrainy+10;
		DadosDaUnidade *dados = (*oDataManager)[g_Jogador->odados];

		if(dados->tipo == TIPO_AERONAVE) {
			pos.y += 3000;
		} else if(dados->tipo == TIPO_HELICOPTERO) {
			pos.y += 500;
		}
		pos.y += 20;

		if(g_Jogador->team)
			a->Criar(&pos, 0, g_Jogador->team, dados);
		else
			a->Criar(&pos, 0, NULL, dados);
			
		a->ogm = NULL;

		g_Jogador->SetObject(a);
	}
}

int missaoID = 0;

HRESULT Logic(const float t, const float dt)
{
	float xvar, zvar;
	CProjetil *pProj;
	CObjeto *pObj;
	CSimpleNode<Carcaca> *pCar;

	PollKeyboard();
	EfeitoRadio();
	//if(GameTicks % 100 == 0)
	//	g_Jogador->GetCamera()->Shake(100);
	
	//if(GameTicks % 100 == 0)
	//{
	//	sound->Play();
	//}
	if(keys[DIK_F5] == GameTicks)
	{
		g_Wireframe = !g_Wireframe;
	}

	switch(g_GameState) {
		case GS_START:
			mywrapper->SetClearColor(0x00000000);
			g_GameState = GS_MENU;
			break;
		case GS_MENU:
			mywrapper->SetClearColor(0x00000000);
			if(keys[DIK_DOWN] == GameTicks) {
				if(menuInicialCursor < 2) {
					++menuInicialCursor;
					if(ui_move)
						ui_move->Play();
				}
			} else if(keys[DIK_UP] == GameTicks) {
				if(menuInicialCursor > 0) {
					--menuInicialCursor;
					if(ui_move)
						ui_move->Play();
				}
			}
			if(keys[DIK_SPACE] == GameTicks) {
				if(ui_select)
					ui_select->Play();
				switch(menuInicialCursor) {
					case 0:
						g_GameState = GS_SELECTTEAM;
						break;
					case 1:
						g_GameState = GS_SOBRE;
						break;
					case 2:
						SendMessage(hWnd, WM_CLOSE, 0, 0);
						break;
				}
			}
			if(keys[DIK_ESCAPE] == GameTicks)
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case GS_SOBRE:
			mywrapper->SetClearColor(0x00000000);
			if(keys[DIK_ESCAPE] == GameTicks) {
				g_GameState = GS_MENU;
				if(ui_select)
					ui_select->Play();
			}
			break;
		case GS_SELECTOBJ:
			if(keys[DIK_ESCAPE] == GameTicks) {
				if(ui_select)
					ui_select->Play();
				g_GameState = GS_SELECTTEAM;
				break;
			}
			if(keys[DIK_SPACE] == GameTicks) {
				if(ui_select)
					ui_select->Play();
				g_GameState = GS_SELECIONARMISSAO;
			}
			if(keys[DIK_RIGHT] == GameTicks) {
				if(ui_move)
					ui_move->Play();
				++g_Jogador->odados;
			} else if(keys[DIK_LEFT] == GameTicks) {
				if(ui_move)
					ui_move->Play();
				--g_Jogador->odados;
			}
			if(g_Jogador->odados < 0)
				g_Jogador->odados = oDataManager->getLista().size()-1;
			if(g_Jogador->odados >= oDataManager->getLista().size())
				g_Jogador->odados = 0;
			break;
		case GS_SELECTTEAM: {
				static int equipese = -1;
				if(keys[DIK_ESCAPE] == GameTicks) {
					if(ui_select)
						ui_select->Play();
					g_GameState = GS_MENU;
					break;
				}
				if(keys[DIK_SPACE] == GameTicks) {
					HRESULT hr;
					if(ui_select)
						ui_select->Play();
					g_GameState = GS_SELECTOBJ;
				}

				if(keys[DIK_RIGHT] == GameTicks) {
					if(ui_move)
						ui_move->Play();
					++equipese;
				}
				if(keys[DIK_LEFT] == GameTicks) {
					if(ui_move)
						ui_move->Play();
					--equipese;
				}

				if(equipese < -1)
					equipese = eDM->getSize()-1;
				else if(equipese >= eDM->getSize())
					equipese = -1;

				if(equipese <= -1)
					g_Jogador->team = NULL;
				else
					g_Jogador->team = (*eDM)[equipese];
			}
			break;
		case GS_SELECIONARMISSAO: {
				if(keys[DIK_ESCAPE] == GameTicks) {
					if(ui_select)
						ui_select->Play();
					g_GameState = GS_SELECTOBJ;
					break;
				}

				if(keys[DIK_RIGHT] == GameTicks) {
					if(ui_move)
						ui_move->Play();
					++missaoID;
				}
				else if(keys[DIK_LEFT] == GameTicks) {
					if(ui_move)
						ui_move->Play();
					--missaoID;
				}
				if(missaoID < 0)
					missaoID = mDataManager->getSize()-1;
				else if(missaoID >= mDataManager->getSize())
					missaoID = 0;
				if(keys[DIK_SPACE] == GameTicks) {
					HRESULT hr;
					if(ui_select)
						ui_select->Play();
					hr = CarregarMissao((*mDataManager)[missaoID]);
					if(FAILED(hr))
						RuntimeError("Erro ao carregar missão!");
					g_GameState = GS_GAMESTARTUP;
				}
			}
			break;
		case GS_GAMESTARTUP: {
			if(!GameTicksAlphaStarted) {
				g_Unidades->DeleteAll();
				g_Projeteis->DeleteAll();
				g_Jogador->SetObject(NULL);
				ps->m_pParticles->DeleteAll();
				ps->m_pEmitters->DeleteAll();
				ps->m_pPackList->DeleteAll();
				mymission->Reset();
				mission_complete = 0;
				g_Jogador->GetCamera()->SetupPhysics();
				paused = false;
				ObjetivoMudou = GameTicks;
				//myband->Position(&(mymission->p1pos+mymission->hotspot));

				SpawnPlayer();
				CurrentObjective = 0;
				casualities = 0;

				SpawnMissionObjects();
				GameTicksAlphaStarted = GameTicks;
			}

			if(GameTicks-GameTicksAlphaStarted > 128) {
				mywrapper->SetClearColor(mymission->GetDados()->getAmbiente()->getAmbientLight());
				ObjetivoMudou = GameTicks;
				objetivoStartSec = GetTime();
				g_GameState = GS_GAME;
				g_Jogador->mostrarTarefas(mymission);
			}
			
			break;
			}
		case GS_GAME:
			if(keys[DIK_ESCAPE] == GameTicks) {
				g_GameState = GS_MENU;
				GameTicksAlphaStarted = 0;
				game_over = 0;
				break;
			}
			if(rmouse_click == GameTicks) {
				paused = !paused;
			}
			if(!game_over) {
				if(keys[DIK_R] == GameTicks)
					SpawnPlayer();
				else if(keys[DIK_O] == GameTicks)
					ObjetivoMudou = GameTicks;
			}
			//body.UpdatePhysics(t, dt);
			/*if(g_Unidades->m_pActiveList->GetNodeNum() < gc->GetDbgUnits())
			{
				CObjeto *obj = g_Unidades->New();
				obj->Criar(&Vector(1000+randomf(2000), 2000+randomf(1000),1000+randomf(2000)), 0, rand()%3, odados[rand()%odados.size()]);
				//obj->Criar(&D3DXVECTOR3(randomf(terrain->GetWidth()), 2000+randomf(1000), randomf(terrain->GetDepth())), 0, rand()%2, odados[rand()%odados.size()]);
			}*/

			g_Jogador->Atualizar();
			bmesh->CalcNormals();
			if(!paused) {
				mymission->Update(dt);

				if(game_over) {
					if(GameTicks - game_over > 240) {
						g_GameState = GS_MENU;
						GameTicksAlphaStarted = 0;
						game_over = 0;
						break;
					}
				}
				else if(CurrentObjective < mymission->GetNObjetivos()) {
				unsigned int nUnits = mymission->getNBases(g_Jogador->team);
					pObj = g_Unidades->GetFirstActive();
					while(pObj)
					{
						if(pObj->team == g_Jogador->team)
							++nUnits;
						pObj = g_Unidades->GetNext(pObj);
					}

					if(!nUnits)
						game_over = GameTicks;

					CObjetivo *objetivo = &mymission->GetDados()->pObjetivos[CurrentObjective];

				if(objetivo->arquivoDeMusicaValido()) {
					if((!mci->Aberto() || ((objetivo->szMusic != mci->GetCurrentMusic())))) {
						mci->Fechar();
						//mci->Abrir("H:\\zeze di camargo & luciano-tapa na cara201.mp3", "mpegvideo");
						mci->Abrir(objetivo->szMusic, "mpegvideo");
					}
					if(!mci->Tocando())
						mci->Tocar();
				}

				unsigned int TarefasCompletas = 0;
				bool passa_direto = true;

				for(register size_t a = 0; a < objetivo->GetSize() && !game_over; ++a) {
					CTarefaDados *tarefa = &objetivo->pTarefas[a];
					switch(tarefa->GetType()) {
						case TAREFA_DESTRUIR:
							if(mymission->GetGOM(tarefa).GetActive() <= 0)
								++TarefasCompletas;
							break;
						case TAREFA_PROTEGER:
							if(mymission->GetGOM(tarefa).GetActive() <= 0)
								game_over = GameTicks;
							else
								++TarefasCompletas;
							passa_direto = false;
							break;
						case TAREFA_DOMINACAO:
							++TarefasCompletas;
							for(register size_t a = 0; a < mymission->GetBases().GetCapacity(); ++a) {
								CBase *atual = &mymission->GetBases()[a];
								if(atual->Dona() && atual->Dona() != g_Jogador->team) {
									--TarefasCompletas;
									break;
								}
							}
							break;
						case TAREFA_ESPERAR:
							++TarefasCompletas;
							passa_direto = false;
							break;
						case TAREFA_CAPTURAR: {
							CBase *base = &mymission->GetBases()[tarefa->getBase()];
							if(base->Dona() == g_Jogador->team && base->Influencia() >= 1)
								++TarefasCompletas;
							break;
						}
						case TAREFA_DEFENDER_BASE:
							if(mymission->GetBases()[tarefa->getBase()].Dona() != g_Jogador->team)
								game_over = GameTicks;
							else
								++TarefasCompletas;
							passa_direto = false;
							break;
					}
				}

				if(objetivo->maxSec) {
					bool estouro_tempo = ((GetTime() - objetivoStartSec) >= objetivo->maxSec);
					bool passou = estouro_tempo || passa_direto;

					if(passou)
					{
						if(passa_direto) {
							if(TarefasCompletas >= objetivo->GetSize()) {
								CurrentObjective++;
								SpawnMissionObjects();
								objetivoStartSec = GetTime();
								ObjetivoMudou = GameTicks;
								g_Jogador->mostrarTarefas(mymission);
							} else if(estouro_tempo)
								game_over = GameTicks;
						} else {
							if(TarefasCompletas < objetivo->GetSize())
								game_over = GameTicks;
							else if(TarefasCompletas >= objetivo->GetSize() && estouro_tempo) {
								CurrentObjective++;
								SpawnMissionObjects();
								objetivoStartSec = GetTime();
								ObjetivoMudou = GameTicks;
								g_Jogador->mostrarTarefas(mymission);
							}
						}
					}
				} else if(TarefasCompletas >= objetivo->GetSize()) {		
					CurrentObjective++;
					SpawnMissionObjects();
					objetivoStartSec = GetTime();
					ObjetivoMudou = GameTicks;
					g_Jogador->mostrarTarefas(mymission);
				}

			} else {
				if(!mission_complete) {
					mission_complete = GameTicks;
				}
				if(GameTicks - mission_complete > 240) {
					g_GameState = GS_MENU;
					GameTicksAlphaStarted = 0;
					mission_complete = 0;
				}
			}


/*#ifdef _DEBUG
			if(g_Jogador->IsObjectValid())
			{
				register unsigned int x = 0, y = 0;
				for(register unsigned int a = 0; a < NTERRAINPOINTS; ++a) {

					g_TerrainPoints[a].Position(g_Jogador->GetObjectA()->GetPositionX()+x*40, 0, g_Jogador->GetObjectA()->GetPositionZ()+y*40);
					g_TerrainPoints[a].Position(g_TerrainPoints[a].GetPositionX(), terrain->GetTerrainHeight(g_TerrainPoints[a].GetPosition()), g_TerrainPoints[a].GetPositionZ());
					g_TerrainPoints[a].temp = static_cast<unsigned char>(terrain->lasttri);
					++x;
					if(x >= 50) {
						x = 0;
						++y;
					}
				}
			}
#endif*/
			/*for(register unsigned int a = 0; a < NTERRAINPOINTS; ++a) {
				g_TerrainPoints[a].Translate(1, 0, 0);
				g_TerrainPoints[a].Position(g_TerrainPoints[a].GetPositionX(), terrain->GetTerrainHeight(g_TerrainPoints[a].GetPosition()), g_TerrainPoints[a].GetPositionZ());
				g_TerrainPoints[a].temp = static_cast<unsigned char>(terrain->lasttri);
			}*/
			
			bmesh->Update(dt);
		
			/*/camera.Move(xvar, 0, zvar);*/

			//camera.AlignToVector(&D3DXVECTOR3(8, -6, -4), 2, static_cast<float>(GameTicks)/10000.0f);

			xvar = zvar = 0;
			if( keys['J'] )
				xvar = 0.01f;
			else if( keys['L'] )
				xvar = -0.01f;

			if( keys['K'] )
				zvar = 0.01f;
			else if( keys['I'] )
				zvar = -0.01f;

			/*if(xvar || zvar)
				camera.Turn(zvar, xvar, 0);*/
			//D3DXVECTOR3 vec, dir, res, in;
			//D3DXVECTOR3 result;
			//result = objects[0].pos - camera.pos;
			//D3DXVec3Normalize(&result, &result);
			//PointVector(&vec, &camera.pos, &objects[0].pos);
			//camera.Rotate(-result.y, result.x, 0);
			//camera.Rotate(vec.x, vec.y, 0);
			//objects[0].Turn(0.01f, 0 , 0);
			/*dir.x = objects[0].pitch;
			dir.y = objects[0].yaw;
			dir.z = objects[0].roll;
			in.x = 0;
			in.y = 0;
			in.z = 1;

			D3DXVECTOR3 v;
			v.x = 20;
			v.y = 200;
			v.z = 210;*/
			//float pu = D3DXToDegree(DeltaPitch(Yaw(&v));
			/*D3DXVECTOR3 in, res;
			in.x = 80;
			in.y = 510;
			in.z = 1;
			Entity *obj = new Entity();
			//Entity *wld = new Entity();
			//wld->Position(0, 0, 0);
			//wld->Turn(0, 0, 0);
			//obj->Move(1, 2, 3);
			//obj->Scale(122, 519, 88);
			obj->Position(10,20,30);
			//obj->Turn(82, 0, 0);
			//obj->Turn(82, -36, 34);
			//obj->Position(0, 0, 0);
			obj->Turn(D3DXToRadian(-51), D3DXToRadian(-150), D3DXToRadian(250));
			//wld->Position(400, 0, -30);
			//wld->Turn(D3DX_PI, 0, 0); 
			//float ua = D3DXToDegree(DeltaYaw(obj, wld));
			TFormVector(&res, &in, 0, obj);
			delete obj;*/
			//delete wld;
			//PointVector(&vec, &objects[0].pos, &dir, &camera.pos, 0.001f);
			//camera.Turn(DeltaPitch(&camera, objects[0])/8.0f, DeltaYaw(&camera, objects[0])/8.0f, 0);
			//camera.Rotate(camera.Pitch(), camera.Yaw(), camera.Roll());
			//objects[0]->ent->Move(0, 0, 0.5f);
			//objects[0]->ent->Turn(0.010f, 0, 0);
			/*D3DXVECTOR3 vel;
			vel.x = float(rand()%8000);
			vel.y = 0;
			vel.z = (float)(-rand()%8000);*/
			//ps->CreateParticle(&objects[0].pos, &vel, 1.0f, 1.0f/(250+rand()%250), 5.0f, 0, pt->texture);

			if(gListener)
				UpdateListener();

			/*if(GameTicks % 150 == 0)
			{
				ps->CriarExplod(&vel, explods[7]);
			}*/

			/*if(GameTicks == 150 == 0)
			{
				sound->Play3D(objects[0], 0, 0, 0);
			}*/

			ps->Update();
			//camera.Update();
			//terrain->Position(0, 0, 0);
			//terrain->Scale(1, 20, 1);..
			
			PROFILE_START;
			pObj = g_Unidades->GetFirstActive();
			while(pObj)
			{
				if(pObj->UpdateObject())
					g_Unidades->Delete(&pObj);
				else
					pObj = g_Unidades->GetNext(pObj);
			}
			PROFILE_END;


			pProj = g_Projeteis->GetFirstActive();
			while(pProj)
			{
				if(pProj->UpdateProjetil())
					g_Projeteis->Delete(&pProj);
				else
					pProj = g_Projeteis->GetNext(pProj);
			}

			pCar = g_Carcacas->GetFirstActive();
			while(pCar)
			{
				if((*pCar).GetPtr()->Update())
					g_Carcacas->Delete(&pCar);
				else
					pCar = pCar->GetNext();
			}
			}

			break;
		default:
			g_GameState = GS_START;
			break;
	}
	
	return S_OK;
}

DWORD VectorToRGB(Vector* NormalVector)
{
	DWORD dwR = (DWORD)(127 * NormalVector->x + 128);
	DWORD dwG = (DWORD)(127 * NormalVector->y + 128);
	DWORD dwB = (DWORD)(127 * NormalVector->z + 128);

	return (DWORD)(0xff000000 + (dwR << 16) + (dwG << 8) + dwB);
} //VectorToRGB

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }

//-----------------------------------------------------------------------------
// Name: Render
// Desc: Render a frame
//-----------------------------------------------------------------------------
HRESULT Render(CJogador *j)
{
	if(g_Wireframe)
		d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		d3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	/*std::string output;
	std::ostringstream stream;*/

	HRESULT hr =  d3dDevice->BeginScene();
	if( SUCCEEDED(hr ) )
	{
		// Clear
#ifdef DIA
		d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(47,36,40), 1.0f, 0 );
#else
		d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0 );
#endif
		switch(g_GameState) {
			case GS_MENU:
				gh->SetDrawMode(GameHUD::F_ALPHABLEND | GameHUD::F_TEXTURIZED);
					
				gh->RenderQuad(310, 250, imgMenuItemIniciar->GetWidth()*(640.0/gc->GetGraphicsWidth())*1.2, imgMenuItemIniciar->GetHeight()*(480.0/gc->GetGraphicsHeight())*1.2, imgMenuItemIniciar, (menuInicialCursor == 0) ? 0xFFFFFFFF : 0xFFAAAAFF);
				gh->RenderQuad(310, 330, imgMenuItemSobre->GetWidth()*(640.0/gc->GetGraphicsWidth())*1.2, imgMenuItemSobre->GetHeight()*(480.0/gc->GetGraphicsHeight())*1.2, imgMenuItemSobre, (menuInicialCursor == 1) ? 0xFFFFFFFF : 0xFFAAAAFF);
				gh->RenderQuad(310, 400, imgMenuItemSair->GetWidth()*(640.0/gc->GetGraphicsWidth())*1.2, imgMenuItemSair->GetHeight()*(480.0/gc->GetGraphicsHeight())*1.2, imgMenuItemSair, (menuInicialCursor == 2) ? 0xFFFFFFFF : 0xFFAAAAFF);

				gh->RenderQuad(0, 0, gc->GetGraphicsWidth(), gc->GetGraphicsHeight(), imgMenu);
					
				gh->Render();
				//g_Font->DrawTextA(fontsprlayer, "Menu", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
				break;
			case GS_SOBRE: {
					static int state = 0;
					static int alpha = 255;
					static int x = 0, y = 0, pid = 0;
					switch(state) {
						case 0:
							alpha = 0;
							state = 1;
							x = random(150);
							y = random(200);
							pid = (pid+1)%NPORTRAITS;
							break;
						case 1:
							if(alpha < 255)
								++alpha;
							else
								state = 2;
							break;
						case 2:
							if(alpha > 0)
								--alpha;
							else
								state = 0;
							break;
					}
					gh->SetDrawMode(GameHUD::F_ALPHABLEND | GameHUD::F_TEXTURIZED);
					
					gh->RenderQuad(x, 140+y, imgSobrePortraits[pid]->GetWidth()/2, imgSobrePortraits[pid]->GetHeight()/2, imgSobrePortraits[pid], D3DCOLOR_RGBA(255, 255, 255, alpha));
					gh->RenderQuad(430, 60, imgSobreTexto->GetWidth()*(640.0/gc->GetGraphicsWidth())*1.2, imgSobreTexto->GetHeight()*(480.0/gc->GetGraphicsHeight())*1.2, imgSobreTexto, 0xFFFFFFFF);

					gh->RenderQuad(0, 0, gc->GetGraphicsWidth(), gc->GetGraphicsHeight(), imgMenu);
					
					gh->Render();
				}
				break;
			case GS_SELECTOBJ:
				if(SUCCEEDED(fontsprlayer->Begin(D3DXSPRITE_ALPHABLEND))) {
					FontPosition.top = 0;
					g_Font->DrawTextA(fontsprlayer, "Seleção de Veículos", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));

					DadosDaUnidade *du = (*oDataManager)[j->odados];
					CTexture *unittex = NULL;
					FontPosition.top = 60;
					g_Font->DrawText(fontsprlayer, du->getNome().c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					FontPosition.top = 80;
					g_Font->DrawTextA(fontsprlayer, "Tipo de Unidade", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					FontPosition.top = 200;
					
					if(du->arma) {
						g_Font->DrawTextA(fontsprlayer, "Possíveis Alvos", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
						if(du->arma->subtipo == TIPO_MISSEL || du->arma->subtipo == TIPO_MIRV) {
							FontPosition.top = 220;
							g_Font->DrawTextA(fontsprlayer, "Lockon", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
						}
					} else
						g_Font->DrawTextA(fontsprlayer, "Desarmado", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					
					FontPosition.top = 400;
					g_Font->DrawText(fontsprlayer, du->descricao.c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					fontsprlayer->End();

					switch(du->tipo) {
						case TIPO_AERONAVE:
							unittex = imgAero;
							break;
						case TIPO_HELICOPTERO:
							unittex = imgHeli;
							break;
						case TIPO_VEICULO:
							if(du->getImovel())
								unittex = imgImobile;
							else
								unittex = imgGround;
							break;
					}

					gh->SetDrawMode(GameHUD::F_ALPHABLEND | GameHUD::F_TEXTURIZED);
					gh->RenderQuad(0, 110, 1, unittex);

					if(du->arma) {
						int x = 0;
						D3DCOLOR cor = D3DCOLOR_XRGB(200+static_cast<int>(55*(sin(GameTicks/r(20)))), 0, 0);
						if(du->arma->targetType & ALVO_CHAO) {
							gh->RenderQuad(x, 230, 1, imgGround, cor);
							x += imgGround->GetWidth();
						}
						if(du->arma->targetType & ALVO_HELICOPTERO) {
							gh->RenderQuad(x, 230, 1, imgHeli, cor);
							x += imgHeli->GetWidth();
						}
						if(du->arma->targetType & ALVO_AR) {
							gh->RenderQuad(x, 230, 1, imgAero, cor);
							x += imgAero->GetWidth();
						}
					}
					gh->Render();
				}
				break;
			case GS_SELECTTEAM: {
					CEquipe *eq = j->team;

					if(eq) {
							gh->SetDrawMode(GameHUD::F_TEXTURIZED | GameHUD::F_CENTER);
							gh->RenderQuad(gc->GetGraphicsWidth()*0.8, 400, 1, eq->getFlag());
					}
					gh->SetDrawMode(GameHUD::F_ALPHABLEND | GameHUD::F_TEXTURIZED);
					gh->RenderQuad(gc->GetGraphicsWidth()-imgSelEquipeTexto->GetWidth(), gc->GetGraphicsHeight()-imgSelEquipeTexto->GetHeight()-20, imgSelEquipeTexto->GetWidth(), imgSelEquipeTexto->GetHeight(), imgSelEquipeTexto);
					gh->RenderQuad(0, 0, gc->GetGraphicsWidth(), gc->GetGraphicsHeight(), imgMenu);
					gh->Render();

					if(SUCCEEDED(fontsprlayer->Begin(D3DXSPRITE_ALPHABLEND))) {

						FontPosition.top = gc->GetGraphicsHeight()-imgSelEquipeTexto->GetHeight()-10;
						FontPosition.left = gc->GetGraphicsWidth()-280;
					
						if(eq)
							g_Font->DrawText(fontsprlayer, eq->getNome().c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
						else
							g_Font->DrawTextA(fontsprlayer, "Nenhuma (Você não irá sobreviver)", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
	
						FontPosition.top = FontPosition.left = 0;
						fontsprlayer->End();
					}
					break;
				}
			case GS_SELECIONARMISSAO:
				if(SUCCEEDED(fontsprlayer->Begin(D3DXSPRITE_ALPHABLEND))) {
					CEquipe *eq = j->team;
					FontPosition.top = 0;
					g_Font->DrawTextA(fontsprlayer, "Seleção de Missão", -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					FontPosition.top += 40;
					g_Font->DrawText(fontsprlayer, (*mDataManager)[missaoID]->getNome().c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,255));
					FontPosition.top = 0;
					fontsprlayer->End();
				}
				break;
			case GS_GAMESTARTUP:
				if(SUCCEEDED(fontsprlayer->Begin(D3DXSPRITE_ALPHABLEND))) {
					const float a = ((GameTicks-GameTicksAlphaStarted)/128.0f)*D3DX_PI;
					g_Font->DrawText(fontsprlayer, mymission->GetDados()->getNome().c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(255,255,255,static_cast<unsigned int>(sinf(a)*255)));
					fontsprlayer->End();
				}
				break;
			case GS_GAME:
				// Render the skybox first
				// Set sampler state to clamp
				d3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
				d3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

				skybox->Render(g_Jogador->GetCamera());

				//myband->Position(g_Jogador->GetCamera()->GetPosition());
				//skybox->Render( d3dDevice, D3DXVECTOR3( 0.0f, 0.0f, 0.0f ) );
				//terrain->TransformToRender();
				//d3dDevice->SetTransform( D3DTS_WORLD, &terrain->mWorld );
				//terrain->Render( d3dDevice );

				// Turn clamping off, back to wrap
				d3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
				d3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

				terrain->Render(g_Jogador->GetCamera());
				mymission->Render();

				PROFILE_START;
				for(CObjeto *pObj = g_Unidades->GetFirstActive(); pObj; pObj = g_Unidades->GetNext(pObj))
				{
					if(g_Jogador->GetCamera()->SphereVisible(pObj->GetPosition(), pObj->dados->getRadiusSq()))
					{
						pObj->UpdateMatrix();
						pObj->RenderObject();
					}
				}

				for(CProjetil *pProj = g_Projeteis->GetFirstActive(); pProj; pProj = g_Projeteis->GetNext(pProj))
				{
					if(pProj->tipo->gtipo != GTIPO_NONE) {
						if(pProj->tipo->gtipo == GTIPO_FEIXE) {
							D3DXMATRIX m, m2;
							CQuaternion qt;
							
							Vector dir = *pProj->GetPosition()-pProj->getOldState().getPosition();

							qt.setEulerAngles(dir.pitch(), dir.yaw(), 0);
							qt.rotationMatrix(&m);
							D3DXMatrixScaling(&m2, pProj->tipo->size, pProj->tipo->size, dir.length());
							D3DXMatrixMultiply(&m, &m2, &m);
							D3DXMatrixTranslation(&m2, pProj->GetPosition()->x, pProj->GetPosition()->y, pProj->GetPosition()->z);
							D3DXMatrixMultiply(&m, &m, &m2);
							d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
							d3dDevice->SetTexture(0, NULL);
							d3dDevice->SetTransform( D3DTS_WORLD, &m );
							pProj->tipo->mesh->Render();
							d3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
						} else if(g_Jogador->GetCamera()->SphereVisible(pProj->GetPosition(), pProj->tipo->size)) {
							pProj->UpdateMatrix(&pProj->tipo->mMatrix);
							d3dDevice->SetTransform( D3DTS_WORLD, &pProj->mWorld );
							pProj->tipo->mesh->Render();
						}
					}
				}

				for(CSimpleNode<Carcaca> *pCar = g_Carcacas->GetFirstActive(); pCar; pCar = pCar->GetNext())
				{
					if(g_Jogador->GetCamera()->SphereVisible(pCar->GetPtr()->GetPosition(), 50))
					{
						pCar->GetPtr()->Render();
					}
				}

				PROFILE_END;

				d3dDevice->SetTexture(0, martex->Get());
				mar->Render(g_Jogador->GetCamera());
				ps->Render(g_Jogador->GetCamera());


				d3dDevice->SetTexture(0, NULL);
				d3dDevice->SetIndices(NULL);
				d3dDevice->SetStreamSource(0, NULL, 0, 0);
				d3dDevice->SetFVF(0);

				break;
		}

		if(SUCCEEDED(hr))
			hr = d3dDevice->EndScene();
	}
	/*g_Font->DrawText(NULL,
					output.c_str(),
					-1,
					&FontPosition,
					DT_CENTER,
					0xffffffff); //draw text*/

	//UpdateStats();
	if(SUCCEEDED(hr) && g_GameState == GS_GAME)
		j->AtualizarHUD();
	hr = d3dDevice->Present( NULL, NULL, NULL, NULL );
	return hr;
}

//-----------------------------------------------------------------------------
// Name: UpdateStats()
// Desc: Update the frame stats, such as FPS
//-----------------------------------------------------------------------------
/*HRESULT UpdateStats()
{
	static float lastTime = GetTime();
	
	// Only update FPS once per second
	float timeDif = GetTime() - lastTime;

	if( timeDif > 1.0f )
	{
		fps = (float)frameCount;

		frameCount = 1;
		lastTime   = GetTime();
	}

	frameCount++;

	return S_OK;
}*/

//-----------------------------------------------------------------------------
// Name: GetRandom()
// Desc: Gets positive or negative random number (remember to seed using srand)
//-----------------------------------------------------------------------------
float GetRandom()
{
	float random = static_cast<float>(rand() / RAND_MAX);

	if( random < 0.5f )
		random = -random * 2;
	else
		random = (1.0f - random) * 2;

	return random;
}



//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_KILLFOCUS:
			g_Wait = 40;
			break;
		case WM_SETFOCUS:
			g_Wait = 0;
			break;
		case WM_LBUTTONDOWN:
			if(!mouse_click)
				mouse_click = GameTicks+1;
			break;
		case WM_LBUTTONUP:
			mouse_click = 0;
			break;
		case WM_RBUTTONDOWN:
			if(!rmouse_click)
				rmouse_click = GameTicks+1;
			break;
		case WM_RBUTTONUP:
			rmouse_click = 0;
			break;
		case WM_SETCURSOR:
			SetCursor(NULL);
			return TRUE;
			break;
			
		case WM_SIZE:
			if(d3dDevice)
				if( wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED )
					mywrapper->SizeChange();
			break;

		case WM_EXITSIZEMOVE:
			mywrapper->SizeChange();
			break;

		case WM_DESTROY:
			//SendMessage(hWnd, WM_QUIT, NULL, NULL);
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

void HandleSegmentFault(int num)
{
	EXCEPTION_RECORD *excption= (EXCEPTION_RECORD*)_pxcptinfoptrs;

	TRACE("Erro de Segmentação!\nEndereço: %x\n", excption->ExceptionAddress);
	MessageBoxA(NULL, "Erro de segmentação!\nO programa será agora finalizado!", APP_TITLE.charstr(), MB_OK | MB_ICONERROR);
	Cleanup(&wc);

#ifndef _DEBUG
	exit(1);
#endif
}

#include "CrashHandler.h"

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

#ifndef _DEBUG
	InitCrashHandler();
	//signal(SIGSEGV, HandleSegmentFault);
#endif
	g_hInst = hInst;
	TRACE("Inicializando software...\n");

	// Initialize Direct3D

	if(SUCCEEDED( LoadEverything() ) )
	{
		// Enter the message loop
		MSG msg; 
		ZeroMemory( &msg, sizeof(msg) );

		UpdateWindow( hWnd );

		real t = real(0.0);
		const real dt = 1/r(60.0);

		real currentTime = GetTime();
		real accumulator = real(0.0);

		while( msg.message!=WM_QUIT )
		{
			if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				real newTime = GetTime();
				real deltaTime = newTime - currentTime;
				real t;
				currentTime = newTime;
					
				accumulator += deltaTime;

				dbgcounter = 0;

				t = GetTime();

				while(accumulator >= dt) {
					++GameTicks;
					Logic(t, dt);
					t += dt;
					accumulator -= dt;
				}

				logic_t = GetTime()-t;
				t = GetTime();
				g_Jogador->Atualizar3D();
				render_t = GetTime()-t;
				//Render();
			}
			if(g_Wait)
				Sleep(g_Wait); // Usar um pouco de CPU
		}
	}


	Cleanup(&wc);
	return 0;
}