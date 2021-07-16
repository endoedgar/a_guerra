#include "stdafx.h"
#include "mciplayer.h"
#include "defines.h"
#include "globalheader.h"

using namespace std;

/* ESTE MCI PLAYER FUNCIONA POIS NÃO É UNICODE */

MCIPlayer::MCIPlayer(const int __nbuffer):m_pOpened(false), m_pPlaying(false)
{
	TRACE("MCI Player %p criado!\n", this);
	this->music = "";
	if(__nbuffer <= 0)
		return;
	else
		buffer.Reserve(__nbuffer);
	return;
}

MCIPlayer::~MCIPlayer(void)
{
	this->Parar();
	this->Fechar();
	TRACE("MCI Player %p destruído!\n", this);
}

bool MCIPlayer::FExiste(const Smart::String &filepath)
{
	FILE *fp = NULL;

	fopen_s(&fp, filepath.charstr(), "rb");
	if(fp) {
		fclose(fp);
		return true;
	}

	return false;
}

bool MCIPlayer::Abrir(const Smart::String &filepath, const Smart::String &tipo)
{
	if(!this->m_pOpened)
	{
		if(!FExiste(filepath))
			return false;

		Smart::String tmp;

		tmp = "open \"";
		tmp += filepath.c_str();
		tmp += "\" type ";
		tmp += tipo.c_str();
		tmp += " alias Media";

	
		/*astring temp = "open \"";
		temp += filepath;
		temp += "\" type ";
		temp += tipo;
		temp += " alias Media";*/
		this->Comando(tmp);
		this->m_pOpened = true;
		this->music = filepath;
		return true;
	}
	return false;
}

void MCIPlayer::Comando(const Smart::String &str)
{
	TRACE("Enviando comando MCI: \"%s\" \n", str.c_str());
	mciSendString(str.c_str(), NULL, 0, NULL);
}

void MCIPlayer::Tocar()
{
	Comando("seek Media to 0");
	Comando("play Media");
	this->m_pPlaying = true;
}

void MCIPlayer::Parar()
{
	Comando("stop Media");
	this->m_pPlaying = false;
}

void MCIPlayer::Aguardar()
{
	mciSendString(Smart::String("status Media length").c_str(), buffer.GetWritableBuffer(), buffer.GetSize(), NULL);
	unsigned int length = atoi(buffer.charstr()), actual = 0;
	unsigned char antporc = 0, porc = 0;
	do
	{
		mciSendString(Smart::String("status Media position").c_str(), buffer.GetWritableBuffer(), buffer.GetSize(), NULL);
		actual = atoi(buffer.charstr());
		porc = ( unsigned char)(((float)actual/(float)length)*100);
		if(antporc != porc)
		{
			printf("%d%%\n", porc);
			antporc = porc;
		}
		
		mciSendString(Smart::String("status Media mode").c_str(), buffer.GetWritableBuffer(), buffer.GetSize(), NULL);
	} while(buffer[0] == 'p');
}

int MCIPlayer::Tocando()
{
	mciSendString(Smart::String("status Media mode").c_str(), buffer.GetWritableBuffer(), buffer.GetSize(), NULL);
	if(buffer[0] == static_cast<Smart::String::char_type>('p'))
	{
		this->m_pPlaying = true;
		return TRUE;
	}
	this->m_pPlaying = false;
	return FALSE;
}

void MCIPlayer::Fechar()
{
	if(this->m_pOpened)
	{
		Comando("close Media");
		this->m_pOpened = false;
	}
}
