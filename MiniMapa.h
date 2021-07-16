#include "GameHUD.h"

#pragma once

class ObjetoDoMinimapa {

};

class MiniMapa
{
	CTexture *textura;
	int x, y, w, h;
public:
	MiniMapa(const int x, const int y, const int w, const int h);
	~MiniMapa(void);
	void desenharMiniMapa(const Vector &pos);
};
