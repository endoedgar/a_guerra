#include "stdafx.h"
#include "GameGlobals.h"
#include "Jogador.h"
#include "D3DObject.h"

void D3DWrapper::SetLight(const D3DCOLOR lightcolor, const Vector &lightdir) {
	D3DLIGHT9 l;
	ZeroMemory(&l, sizeof(D3DLIGHT9));
	l.Type = D3DLIGHT_DIRECTIONAL;
	l.Diffuse.a = ((lightcolor >> 24) & 0xFF)/255.0f;
	l.Diffuse.r = ((lightcolor >> 16) & 0xFF)/255.0f;
	l.Diffuse.g = ((lightcolor >> 8) & 0xFF)/255.0f;
	l.Diffuse.b = (lightcolor & 0xFF)/255.0f;
	//l.Specular = l.Diffuse;
	l.Direction.x = lightdir.x;
	l.Direction.y = lightdir.y;
	l.Direction.z = lightdir.z;

	l.Attenuation0 = 1;
	l.Attenuation1 = 0;
	l.Attenuation2 = 0;

	l.Position.y = 600;

	*this->light->light = l;

//	d3dDevice->SetLight( 0, this->light->light);
//	d3dDevice->LightEnable( 0, TRUE);
//	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
//	d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
}

D3DWrapper::CLight::CLight(LPDIRECT3DDEVICE9 d3dDevice, D3DLIGHTTYPE type)
{
	this->light = new D3DLIGHT9;
	ZeroMemory(this->light, sizeof(D3DLIGHT9));
	D3DXVECTOR3 vecDir;
	vecDir = D3DXVECTOR3(-0.5, -0.5, 1 );
	D3DXVec3Normalize( static_cast<D3DXVECTOR3*>(&this->light->Direction), &vecDir );
	this->light->Type = type;
	//this->light->Diffuse.r = gc->AmbientLightR/255.0f;
	//this->light->Diffuse.g = gc->AmbientLightG/255.0f;
	//this->light->Diffuse.b = gc->AmbientLightB/255.0f;
	//this->light->Diffuse.a = 1.0f;
	//this->light->Specular.r = gc->AmbientLightR/255.0f;
	//this->light->Specular.g = gc->AmbientLightG/255.0f;
	//this->light->Specular.b = gc->AmbientLightB/255.0f;
	//this->light->Specular.a = 1.0f;

	this->light->Attenuation0 = 1;
	this->light->Attenuation1 = 0;
	this->light->Attenuation2 = 0;

	this->light->Position.y = 600;
	//d3dDevice->SetLight( 0, this->light);
	//d3dDevice->LightEnable( 0, TRUE);
	//d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	//d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
}

//-----------------------------------------------------------------------------
// Name: SizeChange()
// Desc: Reset the device and handle the potential size change
//-----------------------------------------------------------------------------
HRESULT D3DWrapper::SizeChange()
{
	HRESULT hr = S_OK;

	RECT newRect;
	GetClientRect( hWnd, &newRect );

		// Check to see if the dimensions are different
	if( d3dpp.BackBufferWidth  != static_cast<unsigned>(newRect.right - newRect.left ) ||
		d3dpp.BackBufferHeight != static_cast<unsigned>(newRect.bottom - newRect.top ) )
	{
		d3dpp.BackBufferWidth  = newRect.right - newRect.left;
		d3dpp.BackBufferHeight = newRect.bottom - newRect.top;

		D3DVIEWPORT9 vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = d3dpp.BackBufferWidth;
		vp.Height = d3dpp.BackBufferHeight;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;
		g_Jogador->SetViewport(vp);
	}
	else
	{
		// Dimensions didn't change, we're ok
		return S_OK;
	}
	
	if( d3dDevice )
		hr = Reset();
	
	return hr;
}

HRESULT D3DWrapper::SetGFXMode(HWND hWnd, const bool UseZBuffer, const bool UseZ32Bits, const D3DSWAPEFFECT SwapEffect)
{
	HRESULT hr;
	RECT rect;

	GetClientRect(hWnd, &rect);
	ZeroMemory( &d3dpp, sizeof(d3dpp) );

	if(UseZBuffer)
		zbuffer = D3DZB_TRUE;
	else
		zbuffer = D3DZB_USEW;
	d3dpp.Windowed					= TRUE;
	d3dpp.SwapEffect				= SwapEffect;
	std::ostringstream sw;

	d3dpp.BackBufferFormat			= D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil	= TRUE;
	if(UseZ32Bits)
		d3dpp.AutoDepthStencilFormat	= D3DFMT_D24X8;
	else
		d3dpp.AutoDepthStencilFormat	= D3DFMT_D16;
	d3dpp.BackBufferWidth			= rect.right-rect.left;
	d3dpp.BackBufferHeight			= rect.bottom-rect.top;
	d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags						= 0;
	d3dpp.hDeviceWindow				= hWnd;

	hr = d3dObject->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice );
	if( SUCCEEDED( hr ) ) {
		this->hWnd = hWnd;
		//SetupStates();

		SafeDelete(this->light);
		this->light = new CLight(d3dDevice, D3DLIGHT_DIRECTIONAL);
	}
	return hr;
}

HRESULT D3DWrapper::SetupStates()
{
	d3dDevice->SetRenderState( D3DRS_DITHERENABLE,   FALSE );
	d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	d3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	d3dDevice->SetRenderState( D3DRS_ZENABLE, zbuffer  ); 

	d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	d3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	d3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	d3dDevice->SetLight( 0, this->light->light);
	d3dDevice->LightEnable( 0, TRUE);
	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
	d3dDevice->SetRenderState( D3DRS_AMBIENT, this->amb );
	
	return S_OK;	
}

void D3DWrapper::RemoveResource(CResource *t) {
	for(std::vector<CResource*>::iterator i = this->Resources.begin(); i < this->Resources.end(); ++i) {
		if(t == *i) {
			this->Resources.erase(i);
			return;
		}
	}
}

ExternalResource *D3DWrapper::FindResource(const Smart::String &filename, const CRECURSO_TIPO tipo)
{
	for(register unsigned int a = 0; a < this->Resources.size(); ++a) {
		CResource *res = this->Resources[a];
		if(res->isExternal()) {
			ExternalResource *er = static_cast<ExternalResource*>(res);
			if(er->GetType() == tipo && er->getFilename() == filename)
				return er;
		}
	}
	return NULL;
}

HRESULT D3DWrapper::OnLostDevice()
{
	for(std::vector<CResource*>::iterator i = this->Resources.begin(); i < this->Resources.end(); ++i) {
		CResource *r = *i;
		if(r->getClass() == CLASSE_VIDEO) {
			VideoResource *rv = dynamic_cast<VideoResource*>(r);
			rv->OnLostDevice();
		}
	}
	return S_OK;
}

HRESULT D3DWrapper::OnResetDevice()
{
	for(register unsigned int a = 0; a < this->Resources.size(); ++a) {
		CResource *r = this->Resources[a];
		if(r->getClass() == CLASSE_VIDEO) {
			VideoResource *rv = dynamic_cast<VideoResource*>(r);
			rv->OnResetDevice();
		}
	}
	return S_OK;
}

void D3DWrapper::AddResource(CResource *r)
{
	if(!IsResourceAllocated(r)) {
		this->Resources.push_back(r);
		r->AddRef();
	}
}

bool D3DWrapper::IsResourceAllocated(const CResource *resource) {
	register unsigned int a = 0;
	for(std::vector<CResource*>::iterator i = this->Resources.begin(); i < this->Resources.end(); ++i) {
		if(resource == *i)
			return true;
		a++;
	}
	return false;
}

HRESULT D3DWrapper::InitObject()
{
	TRACE("\nIniciando Direct3D 9\n\n");
	if( NULL == ( d3dObject = Direct3DCreate9( D3D_SDK_VERSION ) ) ) {	
		TRACE("Falha!\n");
		return E_FAIL;
	}
	TRACE("Inicializado!");
	return S_OK;
}

HRESULT D3DWrapper::Reset()
{
	HRESULT hr;

	this->OnLostDevice();
	fontsprlayer->OnLostDevice();

	MessageBoxA(this->hWnd, "Já", "Já", 0);
	Sleep(3000);

	hr = d3dDevice->Reset(&d3dpp);

	if( SUCCEEDED( hr ) ) {
		fontsprlayer->OnResetDevice();
		this->OnResetDevice();
		SetupStates();
	}

	return hr;
}

D3DWrapper::D3DWrapper():d3dObject(NULL), d3dDevice(NULL), hWnd(NULL), light(NULL)
{ }

D3DWrapper::~D3DWrapper()
{
	while(this->Resources.size()) {
		CResource *r = this->Resources.back();

		ASSERT(!r->getRefs())
		r->Release();

		//this->Resources.pop_back();
	}
	SafeDelete( light );
	SafeRelease( d3dDevice );
	SafeRelease( d3dObject );
}