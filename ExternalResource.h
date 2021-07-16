#pragma once
#include "gerenciadorderecursos.h"

enum CRECURSO_TIPO {
	TR_INVALIDO,
	TR_TEXTURA,
	TR_SOM,
	TR_SOM3D,
	TR_MESH,
	TR_NRECURSOS
};

extern const Smart::String CRECURSO_TIPO_STR[];

class ExternalResource : public CResource
{
	CRECURSO_TIPO m_Tipo;
	Smart::String m_pFilename;
	//ExternalResource *m_pPrev, *m_pNext;
protected:
	void Catalog(const Smart::String &filename, const CRECURSO_TIPO tipo);
	//static ExternalResource *FindResource(const Smart::String &filename, const CRECURSO_TIPO tipo);
public:
	//inline CResource *GetNext()
	//{ return this->m_pNext; }

	inline CRECURSO_TIPO GetType()
	{ return this->m_Tipo; }

	inline void setFilename(const Smart::String &filename)
	{ this->m_pFilename = filename; }

	inline const Smart::String &getFilename(void) const
	{ return this->m_pFilename; }

	ExternalResource(CLASSE_DO_RECURSO tipo);
	virtual ~ExternalResource(void);
};
