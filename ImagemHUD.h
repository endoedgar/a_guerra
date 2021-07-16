#pragma once

#include "Vector.h"
#include "3DTexture.h"
#include "GameHUD.h"
#include "GameGlobals.h"

using Matematica::Vector;

class ImagemHUD
{
	Vector pos;
	Vector scale;
	CTexture *tex;
public:
	// ctor
	ImagemHUD(void);
	// dtor
	~ImagemHUD(void);

	// getters and setters
	void setPosition(const Vector &pos);
	const Vector &getPosition(void);
	void setScale(const Vector &pos);
	const Vector &getScale(void);

	// methods
	void setTextura(CTexture *tex);
	virtual void atualizar() = 0;
	void desenhar(GameHUD *gh);
};
