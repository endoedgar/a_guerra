#ifndef _GERENCIADORDERECURSOS_H
#define _GERENCIADORDERECURSOS_H

/*#include "3DSound.h"
#include "CMesh.h"*/
#include "SharedResource.h"

void ROnResetDevice();
void ROnLostDevice();

//#include "D3DObject.h"

class CSound;
class CTexture;

enum CLASSE_DO_RECURSO {
	CLASSE_VIDEO,
	CLASSE_SOM
};

class CResource : public SharedResource {
private:
	CLASSE_DO_RECURSO m_Class;
	bool ehExterno;
protected:
	CResource(const CLASSE_DO_RECURSO tipo, const bool ehExterno = false);
	virtual ~CResource();
public:
	CLASSE_DO_RECURSO getClass(void) const
	{ return this->m_Class; }

	inline bool isExternal(void) const
	{ return this->ehExterno; }

	friend void ROnResetDevice();
	friend void ROnLostDevice();
};

/*class CRecurso
{
	CRECURSO_TIPO tipo;
	void *ptr;
	char *filename;
	CRecurso *next;
public:
	unsigned int ref;

	CRecurso(const char *filename, const CRECURSO_TIPO tipo, void *ptr);
	inline LPDIRECT3DTEXTURE9 GetTexture() const
	{ return static_cast<LPDIRECT3DTEXTURE9>(this->ptr); }

	inline LPDIRECTSOUNDBUFFER GetSound() const
	{ return static_cast<LPDIRECTSOUNDBUFFER>(this->ptr); }

	inline CRECURSO_TIPO ObterTipo() const
	{ return this->tipo; }

	inline const char *ObterNome() const
	{ return this->filename; }

	inline CRecurso *ObterProximo() const
	{ return this->next; }

	inline void SetarProximo(CRecurso *r)
	{ this->next = r; }

	unsigned int AddRef();
	~CRecurso();
};

CResource *Recurso_Na_Memoria(const char *filename, const CRECURSO_TIPO tipo);
CResource *InserirRecurso(const char *filename, const CRECURSO_TIPO tipo, void *ptr);*/

#endif