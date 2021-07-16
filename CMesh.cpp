//-----------------------------------------------------------------------------
// Name: CMesh.cpp
// Desc: Creates, renders, and transforms meshes
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "CMesh.h"
#include "defines.h"
#include "GameGlobals.h"
#include "GerenciadorDeRecursos.h"
#include "globalheader.h"
#include "D3DObject.h"

const DWORD CMesh::MESHFVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

//-----------------------------------------------------------------------------
// Name: GetPath()
// Desc: Returns the path of a file (excluding the filename, but with a \ on the end)
//-----------------------------------------------------------------------------
Smart::String &GetPath(Smart::String &dest, const Smart::String &filename )
{
	const size_t ssize = filename.GetLength();
	size_t tmp;

	for(tmp = ssize; tmp > 0; tmp--)
	{
		if(filename[tmp] == '\\' || filename[tmp] == '/')
			break;
	}

	dest = filename;
	
	dest[tmp+1] = '\0';

	return dest;
}



//-----------------------------------------------------------------------------
// Name: CMesh()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMesh::CMesh():centered(false),meshObject(NULL), meshMaterials(NULL), meshTextures(NULL), numFaces(0L), numVerts(0L), numMaterials(0L), scaled(1, 1, 1), centerDisplacement(0, 0, 0), ExternalResource(CLASSE_VIDEO)
{
}

const Vector &CMesh::getCenterDisplacement(void) const {
	return this->centerDisplacement;
}

HRESULT CMesh::center() {
	Vector min, max;
	HRESULT hr = E_FAIL;
	if(this->centered)
		return S_OK;
	if(GetMeshMinMaxDimensions(&min, &max)) {
		this->centerDisplacement = max;
		this->centerDisplacement += min;
		this->centerDisplacement /= 2;
		this->centerDisplacement *= -1;
		hr = this->Translate(&this->centerDisplacement);
		this->centered = true;
	}
	return hr;
}

CMesh::~CMesh()
{
	this->Cleanup();
}

// Vertex declaration
D3DVERTEXELEMENT9 VERTEX_DECL[] =
{
    { 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0},
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};

HRESULT CMesh::OnLostDevice()
{
	if(this->meshTextures )
	{
		for(register DWORD i = 0; i < this->numMaterials; ++i )
			SafeRelease( this->meshTextures[i] );

		SafeDeleteArray(this->meshTextures);
	}

	SafeRelease( this->meshObject );
	return S_OK;
}

HRESULT CMesh::OnResetDevice()
{
	if(this->_Load(this->getFilename()))
		return S_OK;
	return E_FAIL;
}

HRESULT CMesh::Cleanup()
{
	TRACE("Limpando mesh %s...\n", this->getFilename().c_str());
	SafeDeleteArray(this->meshMaterials);
	this->setFilename("");

	if( this->meshTextures )
	{
		
		for( register DWORD i = 0; i < this->numMaterials; ++i )
			SafeRelease( this->meshTextures[i] );

		SafeDeleteArray(this->meshTextures);
	}

	SafeRelease( this->meshObject );
	return S_OK;
}

CMesh *CMesh::_Load( const Smart::String &filename )
{
	// Buffer to hold the materials in
	LPD3DXBUFFER mtrlBuffer = NULL;
	DWORD *rgdwAdjacency = NULL, *rgdwAdjacencyOptimized = NULL;
	HRESULT hr;
	Smart::String file;
	file = filename;

	this->Cleanup();

	TRACE("\nCarregando mesh %s...\n", file.charstr());

	// Load the mesh from the specified file
	if( FAILED( D3DXLoadMeshFromX(	filename.c_str(), 0, d3dDevice,
									NULL, &mtrlBuffer, NULL, &this->numMaterials,
									&this->meshObject ) ) )
	{
		TRACE("\tNão foi possível carregar o mesh!\n");
		goto fail;
	}

	// Get stats
	this->numFaces = this->meshObject->GetNumFaces();
	this->numVerts = this->meshObject->GetNumVertices();

	// Extract material properties and texture names
	D3DXMATERIAL* materials = static_cast<D3DXMATERIAL*>(mtrlBuffer->GetBufferPointer());
	this->meshMaterials = new D3DMATERIAL9[ this->numMaterials ];
	this->meshTextures	= new CTexture*[ this->numMaterials ];

	rgdwAdjacency = new DWORD[this->numFaces * 3];
	
	if(rgdwAdjacency)
	{
		hr = this->meshObject->GenerateAdjacency( 0.01f, rgdwAdjacency );

		if(SUCCEEDED(hr))
		{
			DWORD *rgdwAdjacency2 = new DWORD[this->numFaces * 3];
			ID3DXMesh* pTempMesh;
			hr = D3DXCleanMesh(D3DXCLEAN_SIMPLIFICATION, meshObject, rgdwAdjacency, &pTempMesh, rgdwAdjacency2, NULL);
			if(SUCCEEDED(hr)) {
				rgdwAdjacencyOptimized = new DWORD[this->numFaces * 3];
				SafeRelease(this->meshObject);
				this->meshObject = pTempMesh;
				hr = this->meshObject->OptimizeInplace( D3DXMESHOPT_DEVICEINDEPENDENT | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT |  D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency2, rgdwAdjacencyOptimized, NULL, NULL );
				if(SUCCEEDED(hr))
					TRACE("\tMesh otimizado!\n");
				else
					TRACE("\tNão foi possível otimizar! (Código do Erro: %d)\n", HRESULT_CODE(hr));
			}
			else
				TRACE("\tNão foi possível limpar mesh! (Código do erro: %d)\n", HRESULT_CODE(hr));
			SafeDeleteArray(rgdwAdjacency2);
		}
		else
			TRACE("\tProblema ao gerar adjacencia! (Código do Erro: %d)\n", hr);
		SafeDeleteArray(rgdwAdjacency);
	}
	else
		TRACE("\tSem Memória para alocar %d bytes!\n", this->numFaces * 3);

	if( !(this->meshObject->GetFVF()  & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh;
		TRACE("\tO mesh não tem normals, gerando normals...\n");
		
		hr = this->meshObject->CloneMeshFVF( this->meshObject->GetOptions(),
								this->meshObject->GetFVF() | D3DFVF_NORMAL,
								d3dDevice, &pTempMesh );
		if(SUCCEEDED(hr))
		{
			hr = D3DXComputeNormals( pTempMesh, rgdwAdjacencyOptimized );
			if(FAILED(hr))
				TRACE("\tFalha ao gerar normals! (Código do Erro: %d)\n", HRESULT_CODE(hr));

			SafeRelease( this->meshObject );
			this->meshObject = pTempMesh;
		}
		else
			TRACE("\tHouve um problema ao tentar clonar o mesh! (Código do erro: %d)\n", HRESULT_CODE(hr));
    }

	for( DWORD i = 0; i < this->numMaterials; ++i )
	{
		// Copy the material
		this->meshMaterials[i] = materials[i].MatD3D;

		// Set ambient colors
		this->meshMaterials[i].Ambient = this->meshMaterials[i].Diffuse;

		this->meshTextures[i] = NULL;

		/*if( materials[i].pTextureFilename != NULL && strlen(materials[i].pTextureFilename) > 0 )
		{
			// Find full texture path
			const size_t ssize = this->path.GetLength() + strlen(materials[i].pTextureFilename);
			Smart::String texturePath;
			texturePath = this->path;
			texturePath += materials[i].pTextureFilename;

			// Load texture
			this->meshTextures[i] = this->meshTextures[i]->Load(texturePath);
		}*/
	}

	// Done with the material buffer
	SafeRelease( mtrlBuffer );
	SafeDeleteArray(rgdwAdjacencyOptimized);

	TRACE("\tMesh carregado com sucesso!\n");

	this->Scale(&this->scaled);
	
	return this;
fail:
	this->Release();

	SafeDeleteArray(rgdwAdjacency);
	SafeDeleteArray(rgdwAdjacencyOptimized);
	SafeRelease(mtrlBuffer);
	TRACE("\n");
	return NULL;
}

//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads the mesh from file, creates mesh object, materials and textures
//-----------------------------------------------------------------------------
CMesh *CMesh::Load( const Smart::String &file )
{
	CMesh *mymesh;

	mymesh = dynamic_cast<CMesh*>(mywrapper->FindResource(file, TR_MESH));

	if(mymesh) {
		TRACE("Usando mesh pré carregado!\n");
		mymesh->AddRef();
		return mymesh;
	}

	mymesh = new CMesh();
	mymesh = mymesh->_Load(file);
	if(mymesh) {
		mymesh->Catalog(file, TR_MESH);
	}
	return mymesh;
}

/*HRESULT CMesh::Texture(const char *file)
{
	for( DWORD i = 0; i < numMaterials; i++ )
	{
		SafeRelease( meshTextures[i] );
		D3DXCreateTextureFromFile( d3dDevice, file, &meshTextures[i] );
	}
	return S_OK;
}*/


HRESULT CMesh::Texture(CTexture *tex)
{
	for( register DWORD i = 0; i < numMaterials; ++i )
	{
		SafeRelease( meshTextures[i] );
		meshTextures[i] = tex;
		tex->AddRef();
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Tranforms the mesh, then renders it
//-----------------------------------------------------------------------------
HRESULT CMesh::Render()
{
	d3dDevice->SetFVF(meshObject->GetFVF());
	// Now render

	if(meshObject->GetFVF() & D3DFVF_SPECULAR)
		d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

	for( register DWORD i = 0; i < numMaterials; ++i )
	{
		//if(meshTextures[i])
		//	d3dDevice->SetTexture( 0, meshTextures[i]->Get() );
		//else
		//	d3dDevice->SetTexture( 0, NULL );
		// Set the material and texture for this subset
		d3dDevice->SetMaterial( &meshMaterials[i] );
		if(meshObject->GetFVF() & D3DFVF_SPECULAR)
			d3dDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);


		// Draw the subset
		meshObject->DrawSubset( i );
	}

	if(meshObject->GetFVF() & D3DFVF_SPECULAR)
		d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Tranforms the mesh, then renders it
//-----------------------------------------------------------------------------
HRESULT CMesh::RenderWM()
{
	d3dDevice->SetFVF(meshObject->GetFVF());
	// Now render

	if(meshObject->GetFVF() & D3DFVF_SPECULAR)
		d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

	for( register DWORD i = 0; i < numMaterials; ++i )
	{
		//if(meshTextures[i])
		//	d3dDevice->SetTexture( 0, meshTextures[i]->Get() );
		//else
		//	d3dDevice->SetTexture( 0, NULL );
		// Set the material and texture for this subset
		if(meshObject->GetFVF() & D3DFVF_SPECULAR)
			d3dDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);


		// Draw the subset
		meshObject->DrawSubset( i );
	}

	if(meshObject->GetFVF() & D3DFVF_SPECULAR)
		d3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

	return S_OK;
}