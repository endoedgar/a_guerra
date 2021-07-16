#include "stdafx.h"
#include "ExternalResource.h"
#include "d3dObject.h"
#include "GameGlobals.h"

//ExternalResource *ExternalResource::rlist = NULL;

ExternalResource::ExternalResource(CLASSE_DO_RECURSO tipo):m_Tipo(TR_INVALIDO), CResource(tipo, true)
{
}

ExternalResource::~ExternalResource(void)
{
	TRACE("%p::~ExternalResource() called!\n", this);
	/*if(rlist == this) {
		rlist = this->m_pNext;
		if(this->m_pNext)
			this->m_pNext->m_pPrev = NULL;
	}
	if(this->m_pPrev)
		this->m_pPrev->m_pNext = this->m_pNext;
	if(this->m_pNext)
		this->m_pNext->m_pPrev = this->m_pPrev;*/
}

void ExternalResource::Catalog(const Smart::String &filename, const CRECURSO_TIPO tipo)
{
	ASSERT(tipo >= 0 && tipo <= CRECURSO_TIPO::TR_NRECURSOS);
	TRACE("\tCatalogando Recurso\n");
	TRACE("\t\tNome: %s\n", filename.charstr());
	TRACE("\t\tTipo: %s\n", CRECURSO_TIPO_STR[tipo].charstr());
	TRACE("\t\tDados: %p\n", this);
	this->m_pFilename = filename;
	this->m_Tipo = tipo;
	//this->m_pPrev = NULL;
	//this->m_pNext = rlist;

	mywrapper->AddResource(this);
	/*if(rlist)
		rlist->m_pPrev = this;
	rlist = this;*/
}

