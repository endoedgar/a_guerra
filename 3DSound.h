#ifndef _3DSOUND_H
#define _3DSOUND_H

#include "AudioWave.h"
#include "ExternalResource.h"
#include "TipoDeSom.h"

class Entity;

extern LPDIRECTSOUND8 dsounddevice;

class CBaseSound : public ExternalResource
{
protected:
	Smart::Array<LPDIRECTSOUNDBUFFER> m_pSoundBuffer;
	CWaveFile *wf;
	DWORD m_dwDSBufferSize;

	CBaseSound();
	virtual ~CBaseSound();
public:
	HRESULT FillBuffer(LPDIRECTSOUNDBUFFER pDSB);
	HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );

	HRESULT DuplicateBuffers()
	{
		HRESULT hr = S_OK;

		for( register unsigned int i = 1; i < this->m_pSoundBuffer.GetCapacity(); ++i )
		{
			if( FAILED( hr = dsounddevice->DuplicateSoundBuffer( this->m_pSoundBuffer[0], &this->m_pSoundBuffer[i] ) ) )
				break;
		}

		return hr;
	}
	LPDIRECTSOUNDBUFFER GetFreeBuffer();
};

int SBPlaying(LPDIRECTSOUNDBUFFER buffer);

class CSound : public CBaseSound
{
private:
	CSound(const unsigned int nBuffers = 1);
	~CSound();
public:	
	DWORD m_dwCreationFlags, dwFrequency;

	CSound *Load(const Smart::String &filename);
	LPDIRECTSOUNDBUFFER Play(const float = -1, const float = 1);
	void Stop(LPDIRECTSOUNDBUFFER);
};

class C3DSound : public CBaseSound
{
private:
	LPDIRECTSOUND3DBUFFER *m_pDS3DBuffer;

	C3DSound(const unsigned int m_nBuffers = 3);
	~C3DSound();
public:	
	DWORD m_dwCreationFlags;

	C3DSound *Load(const Smart::String &filename);
	HRESULT Play3D( Entity *, DWORD dwPriority, DWORD dwFlags, LONG lFrequency, const CTipoDeSom &ts );
	HRESULT Get3DBufferInterface(const unsigned int index, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer );
};

extern LPDIRECTSOUND3DLISTENER gListener;

HRESULT Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener );
HRESULT InitSoundSystem(HWND hWnd);
void CleanSoundSystem();

#endif