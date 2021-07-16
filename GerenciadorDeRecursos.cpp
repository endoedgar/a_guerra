#include "stdafx.h"
#include "GerenciadorDeRecursos.h"
#include "defines.h"
#include "3DTexture.h"
#include "GameGlobals.h"
#include "D3DObject.h"
#include "globalheader.h"

const Smart::String CRECURSO_TIPO_STR[] = {
	"Inválido",
	"Textura",
	"Som",
	"Som3D",
	"Mesh"
};

CResource::CResource(const CLASSE_DO_RECURSO tipo, const bool ehExterno):SharedResource(), m_Class(tipo), ehExterno(ehExterno) 
{
}

CResource::~CResource()
{
	TRACE("%p::~CResource() called!\n", this);
}

/*CRecurso *Recurso_Na_Memoria(const char *filename, const CRECURSO_TIPO tipo)
{
	CRecurso *r = rlist;
	while(r)
	{
		if(!_stricmp(r->ObterNome(), filename) && r->ObterTipo() == tipo)
			return r;
		r = r->ObterProximo();
	}
	return NULL;
}

CRecurso *InserirRecurso(const char *filename, const CRECURSO_TIPO tipo, void *ptr)
{
	CRecurso *r = new CRecurso(filename, tipo, ptr);
	TRACE("Catalogando Recurso\n");
	TRACE("\tNome: %s\n", filename);
	TRACE("\tTipo: %d\n", tipo);
	TRACE("\tDados: %p\n", ptr);
	r->SetarProximo(rlist);
	rlist = r;
	return r;
}

void LimparRecursos()
{
	TRACE("\nLimpando recursos...\n");
	CRecurso *r = rlist;
	while(r)
	{
		CRecurso *proximo = r->ObterProximo();
		delete r;
		r = proximo;
	}
	TRACE("Recursos limpados!\n\n");
}

CRecurso::CRecurso(const char *filename, const CRECURSO_TIPO tipo, void *ptr)
{
	const rsize_t size = strlen(filename)+1;
	this->filename = new char[size];
	strcpy_s(this->filename, size, filename);
	this->tipo = tipo;
	this->ptr = ptr;
	this->ref = 0;
}

unsigned int CRecurso::AddRef()
{
	TRACE("Recurso %s (+Ref)\n", this->filename);
	return ++this->ref;
}

CRecurso::~CRecurso()
{
	if(!this->ref)
	{
		TRACE("\tRecurso: %p...\n", this);
		if(this->filename)
		{
			TRACE("\t\tNome: %s\n", this->filename);
			delete [] this->filename;
			this->filename = NULL;
		}
		TRACE("\t\tTipo: ");
		switch(this->tipo)
		{
			case TR_TEXTURA:
				TRACE("Textura");
				this->GetTexture()->Release();
				break;
			case TR_SOM:
				TRACE("Som");
				this->GetSound()->Release();
				break;
		}
		TRACE("\n\n");
	}
	else
		this->ref--;
}*/

/*void ROnResetDevice()
{
	CResource *r = CResource::rlist;
	while(r) {
		r->OnResetDevice();
		r = r->GetNext();
	}
}

void ROnLostDevice()
{
	CResource *r = CResource::rlist;
	while(r) {
		r->OnLostDevice();
		r = r->GetNext();
	}
}*/