#ifndef _CONTROLE_H
#define _CONTROLE_H

#include "3DCamera.h"
#include "ImagemHUD.h"
#include "objeto.h"

class InfoHUD : public ImagemHUD {
		unsigned int time;
	public:
		InfoHUD();
		void mostrar();
		virtual void atualizar();
		void desenhar(GameHUD *gh);
};

enum ControlTipo {
	CONTROLE_TIPO_TECLADO,
	CONTROLE_TIPO_JOYSTICK,

	CT_LIMIT = 0xFFFFFFFF
};

enum INPUTS {
	INPUT_ESQUERDA,
	INPUT_DIREITA,
	INPUT_ACELERAR,
	INPUT_DESACELERAR,
	INPUT_ATIRAR,
	INPUT_STEALTH,

	INPUT_MAX_BUTTONS
};

class CEixo {
private:
	float x, y;
public:
	CEixo()
	{ }

	CEixo(const float x, const float y):x(x), y(y)
	{ }

	inline void setX(const float x) {
		this->x = x;
	}

	inline void setY(const float y) {
		this->y = y;
	}

	inline float getX() const {
		return this->x;
	}

	inline float getY() const {
		return this->y;
	}

	inline void limitar(const float limite) {
		if(this->getX() > limite)
			this->setX(limite);
		else if(this->getX() < -limite)
			this->setX(-limite);
		if(this->getY() > limite)
			this->setY(limite);
		else if(this->getY() < -limite)
			this->setY(-limite);
	}
};

class mouseVars {
	POINT mousepos;
	CEixo eixo;
public:
	inline const CEixo &getEixo() const {
		return this->eixo;
	}
	
	mouseVars():eixo(0, 0)
	{
		mousepos.x = 0;
		mousepos.y = 0;
	}

	void suavizar(const real i) {
		this->eixo.setX(this->eixo.getX()*i);
		this->eixo.setY(this->eixo.getY()*i);
	}

	void atualizar() {
		POINT pt;
		GetCursorPos(&pt);

		this->eixo.setX(static_cast<float>(pt.x-mousepos.x));
		this->eixo.setY(static_cast<float>(pt.y-mousepos.y));

		mousepos.x = pt.x;
		mousepos.y = pt.y;
		SetCursorPos(60, 60);
		mousepos.x = mousepos.y = 60;
	}
};

class CControle
{
	ControlTipo tipo;
	float eixoX;
	float eixoY;
	unsigned int input[INPUT_MAX_BUTTONS];

public:
	mouseVars v;

	CControle(ControlTipo tipo):tipo(tipo) {
	};
	void Atualizar();
	ControlTipo GetType() const
	{ return tipo; }

	inline float getEixoX() {
		return this->eixoX;
	}

	inline float getEixoY() {
		return this->eixoY;
	}

	inline unsigned int operator()(const size_t i) const
	{ return this->input[i]; }
};

class CJogador
{
	char *nome;
	CCamera *cam;
	CObjeto **obj;
	CControle *controle;
	Smart::String texto;
	unsigned int texttime;
	D3DCOLOR textcolor;
	InfoHUD *info;
	unsigned int damagetime;
	Vector wdamage;

	char CameraTipo;

public:
	int odados;
	CEquipe *team;
	CJogador();
	~CJogador();

	void renderizarMapa(const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h);
	void mostrarInformacao(CTexture *tex);
	void showDamage(const Vector &w);
	void MostrarTexto(const Smart::String &texto, const unsigned int texttime, const D3DCOLOR textcolor = 0xFFFFFFFF);
	void Criar(D3DVIEWPORT9 &viewport, ControlTipo tipo);
	void Atualizar();
	void Atualizar3D();
	void AtualizarHUD();
	void Limpar();
	void ControleManual();
	void mostrarAlvo();
	void mostrarTarefas(CMissao *m);
	void SetViewport(D3DVIEWPORT9 &viewport)
	{
		this->cam->setViewport(viewport);
	}

	void SetObject(CObjeto *obj)
	{
		if(obj) {
			this->obj = obj->ptr->GetPtr();
			(*this->obj)->importante = true;
			(*this->obj)->j = this;
		} else
			this->obj = NULL;
	}

	inline bool IsObjectValid() const
	{
		if(this->obj)
			if(*this->obj)
				return true;
		return false;
	}

	inline CObjeto *GetObject() const
	{ return *this->obj; }

	inline CCamera *GetCamera() const
	{ return this->cam; }

	inline CControle *GetControl() const
	{ return this->controle; }

	friend class CObjeto;
};

#endif