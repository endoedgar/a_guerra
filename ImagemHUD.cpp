#include "stdafx.h"
#include "ImagemHUD.h"

ImagemHUD::ImagemHUD(void):tex(NULL)
{
}

const Vector &ImagemHUD::getPosition(void)
{ return this->pos; }

void ImagemHUD::setPosition(const Vector &pos)
{ this->pos = pos; }

void ImagemHUD::setTextura(CTexture *tex)
{
	if(this->tex)
		SafeRelease(this->tex);
	this->tex = tex;
	if(this->tex)
		this->tex->AddRef();
}

void ImagemHUD::desenhar(GameHUD *gh)
{
	if(this->tex)
		gh->RenderQuad(this->getPosition().x, this->getPosition().y, this->tex->GetWidth(), this->tex->GetHeight(), this->tex, 0xFFFFFFFF);
}

ImagemHUD::~ImagemHUD(void)
{
	SafeRelease(this->tex);
}
