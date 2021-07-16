#include "stdafx.h"
#include "3DSound.h"
#include "AudioWave.h"
#include "3DEntity.h"
#include "GerenciadorDeRecursos.h"
#include "globalheader.h"
#include "3DFunctions.h"
#include "D3DObject.h"

LPDIRECTSOUND8 dsounddevice = NULL;
LPDIRECTSOUND3DLISTENER gListener;

// LOWPRIORITY: Ler MP3

CBaseSound::CBaseSound():wf(NULL), m_dwDSBufferSize(0), ExternalResource(CLASSE_SOM)
{
}

CBaseSound::~CBaseSound()
{
	SafeDelete(this->wf);
}

HRESULT InitSoundSystem(HWND hWnd)
{
	HRESULT hr = DirectSoundCreate8(NULL, &dsounddevice, NULL);
	LPDIRECTSOUNDBUFFER pDSBPrimary;
	TRACE("\nIniciando sistema de som...\n");
	if(SUCCEEDED(hr)) {
		TRACE("\tSucesso ao criar COM do DirectSound!\n");	
		hr = dsounddevice->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
		if(SUCCEEDED(hr)) {
			DSBUFFERDESC dsbd;

			TRACE("\tSucesso ao mudar nível cooperativo!\n");	

			ZeroMemory( &dsbd, sizeof( DSBUFFERDESC ) );
			dsbd.dwSize = sizeof( DSBUFFERDESC );
			dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
			dsbd.dwBufferBytes = 0;
			dsbd.lpwfxFormat = NULL;

			hr = dsounddevice->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL );
			if( SUCCEEDED(hr) ) {
				TRACE("\tSucesso ao criar buffer de som primário!\n");				

				WAVEFORMATEX wfx;
				ZeroMemory( &wfx, sizeof( WAVEFORMATEX ) );
				wfx.wFormatTag = static_cast<WORD>(WAVE_FORMAT_PCM);
				wfx.nChannels = static_cast<WORD>(2);
				wfx.nSamplesPerSec = static_cast<DWORD>(22050);
				wfx.wBitsPerSample = static_cast<WORD>(16);
				wfx.nBlockAlign = static_cast<WORD>( wfx.wBitsPerSample / 8 * wfx.nChannels );
				wfx.nAvgBytesPerSec = static_cast<DWORD>( wfx.nSamplesPerSec * wfx.nBlockAlign );

				hr = pDSBPrimary->SetFormat( &wfx );

				if( SUCCEEDED( hr ) ) {
					TRACE("\tSucesso ao setar formato do buffer primário de som!\n");

					if(pDSBPrimary)
						pDSBPrimary->Release();

					hr = Get3DListenerInterface(&gListener);
					if(SUCCEEDED(hr))
						TRACE("\tSistema de som carregado com sucesso!\n\n");
					else
						TRACE("\tFalha ao receber microfone 3D!\nCódigo do erro: %d\n\n", HRESULT_CODE(hr));
				} else
					TRACE("\tFalha ao setar formato do buffer primário de som!\nCódigo do erro: %d\n\n", HRESULT_CODE(hr));
			} else
				TRACE("\tFalha ao criar buffer primário de som!\nCódigo do erro: %d\n\n", HRESULT_CODE(hr));
		} else {
			TRACE("\tFalha ao mudar nível cooperativo!\n\tCódigo do erro: %d\n\n", HRESULT_CODE(hr));
		}
	} else
		TRACE("\tFalha ao criar COM do DirectSound!\n\tCódigo do erro: %d\n\n", HRESULT_CODE(hr));

	if(FAILED(hr))
		SafeRelease(dsounddevice);
	return hr;
}

//-----------------------------------------------------------------------------
// Name: CSoundManager::Get3DListenerInterface()
// Desc: Returns the 3D listener interface associated with primary buffer.
//-----------------------------------------------------------------------------
HRESULT Get3DListenerInterface( LPDIRECTSOUND3DLISTENER* ppDSListener )
{
	HRESULT hr = S_OK;
	DSBUFFERDESC dsbdesc;
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	TRACE("\tObtendo interface do microfone 3D...\n");
	if( ppDSListener ) {
		*ppDSListener = NULL;

		// Obtain primary buffer, asking it for 3D control
		ZeroMemory( &dsbdesc, sizeof( DSBUFFERDESC ) );
		dsbdesc.dwSize = sizeof( DSBUFFERDESC );
		dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

		hr = dsounddevice->CreateSoundBuffer( &dsbdesc, &pDSBPrimary, NULL );
		if( SUCCEEDED(hr) ) {
			hr = pDSBPrimary->QueryInterface( IID_IDirectSound3DListener8, reinterpret_cast<VOID**>(ppDSListener) );
			if( SUCCEEDED( hr ) ) {
				(*ppDSListener)->SetDistanceFactor(300, DS3D_IMMEDIATE);
				TRACE("\t\tMicrofone 3D obtido com sucesso!\n");
			} else {
				TRACE("\t\tFalha ao consultar interface do microfone 3D do buffer de som primário!\nCódigo do erro: %d\n\n", HRESULT_CODE(hr));
			}
		} else {
			TRACE("\t\tFalha ao obter buffer de som primário!\nCódigo do erro: %d\n\n", HRESULT_CODE(hr));
		}
	} else {
		TRACE("\t\tParâmetros da função inválidos!\n");
		hr = E_INVALIDARG;
	}

	// Release the primary buffer, since it is not need anymore
	SafeRelease( pDSBPrimary );

	return hr;
}

void CleanSoundSystem()
{
	TRACE("\nLimpando sistema de som...\n");
	if(dsounddevice)
		dsounddevice->Release();
	dsounddevice = NULL;
	TRACE("\tSistema de som liberado com sucesso!\n\n");
}

C3DSound::C3DSound(const unsigned int nBuffers):m_dwCreationFlags(0), m_pDS3DBuffer(0)
{
	this->m_pSoundBuffer.SetSize(nBuffers);
	TRACE("Som 3D %p criado!\n", this);
}

LPDIRECTSOUNDBUFFER CBaseSound::GetFreeBuffer()
{
    DWORD i;
    for( i = 0; i < this->m_pSoundBuffer.GetCapacity() ; i++ )
    {
        if( this->m_pSoundBuffer[i] )
        {
            DWORD dwStatus = 0;
            this->m_pSoundBuffer[i]->GetStatus( &dwStatus );
            if( ( dwStatus & DSBSTATUS_PLAYING ) == 0 )
                break;
        }
    }

	if( i != this->m_pSoundBuffer.GetCapacity() )
        return this->m_pSoundBuffer[i];
    else
        return this->m_pSoundBuffer[ rand() % this->m_pSoundBuffer.GetCapacity() ];
}

HRESULT C3DSound::Get3DBufferInterface( const unsigned int index, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer )
{
	*ppDS3DBuffer = NULL;

	return this->m_pSoundBuffer[index]->QueryInterface( IID_IDirectSound3DBuffer, reinterpret_cast<VOID**>(ppDS3DBuffer) );
}

//-----------------------------------------------------------------------------
// Name: CSound::Play3D()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT C3DSound::Play3D( Entity *e, DWORD dwPriority, DWORD dwFlags, LONG lFrequency, const CTipoDeSom &ts )
{
	HRESULT hr = S_OK;
	BOOL bRestored;
	DS3DBUFFER p3DBuffer;
	DWORD dwBaseFrequency;

	PROFILE_START;

	D3DVECTOR pos;
	gListener->GetPosition(&pos);
	Vector lPos(pos.x, pos.y, pos.z);
	if(e->getDistanceSq(lPos) > ts.getBuffer().flMaxDistance*ts.getBuffer().flMaxDistance) {
		PROFILE_END;
		return S_OK;
	}


	LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();
	if(pDSB) {
		if(SUCCEEDED(RestoreBuffer(pDSB, &bRestored))) {
			if(bRestored)
				if(FAILED(this->FillBuffer(pDSB))) {
					PROFILE_END;
					return E_FAIL;
				}

			if( this->m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY )
			{
				pDSB->GetFrequency( &dwBaseFrequency );
				pDSB->SetFrequency( dwBaseFrequency + lFrequency );
			}

			LPDIRECTSOUND3DBUFFER pDS3DBuffer;
			if(SUCCEEDED(pDSB->QueryInterface( IID_IDirectSound3DBuffer, reinterpret_cast<LPVOID*>(&pDS3DBuffer)))) {
				ZeroMemory(&p3DBuffer, sizeof(DS3DBUFFER));
				p3DBuffer.dwSize = sizeof(DS3DBUFFER);

				pDS3DBuffer->GetAllParameters(&p3DBuffer);
				p3DBuffer.vPosition.x = e->GetPosition()->x;
				p3DBuffer.vPosition.y = e->GetPosition()->y;
				p3DBuffer.vPosition.z = e->GetPosition()->z;
				p3DBuffer.flMinDistance = ts.getBuffer().flMinDistance;
				p3DBuffer.flMaxDistance = ts.getBuffer().flMaxDistance;
				//hr = pDS3DBuffer->SetPosition(e->pos.x, e->pos.y, e->pos.z, DS3D_IMMEDIATE);
				if( SUCCEEDED(pDS3DBuffer->SetAllParameters( &p3DBuffer, DS3D_DEFERRED )) )
				{
					pDSB->SetCurrentPosition(0);
				    hr = pDSB->Play( 0, dwPriority | DSBPLAY_TERMINATEBY_DISTANCE, dwFlags );
				}

				pDS3DBuffer->Release();
			}
		}
	} else
		hr = E_FAIL;

	PROFILE_END;
	return hr;
}

int SBPlaying(LPDIRECTSOUNDBUFFER buffer)
{
	DWORD dwStatus;

	if(buffer) {
		buffer->GetStatus(&dwStatus);

		return dwStatus & DSBSTATUS_PLAYING;
	}
	return 0;
}

void CSound::Stop(LPDIRECTSOUNDBUFFER sb) {
	sb->Stop();
}

//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CBaseSound::RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored )
{
	HRESULT hr;
	PROFILE_START;

	if( pDSB == NULL )
	{
		PROFILE_END;
		return CO_E_NOTINITIALIZED;
	}
	if( pbWasRestored )
		*pbWasRestored = FALSE;

	DWORD dwStatus;
	if( FAILED( hr = pDSB->GetStatus( &dwStatus ) ) )
	{
		PROFILE_END;
		return hr ;
	}

	if( dwStatus & DSBSTATUS_BUFFERLOST )
	{
		// Since the app could have just been activated, then
		// DirectSound may not be giving us control yet, so
		// the restoring the buffer may fail.
		// If it does, sleep until DirectSound gives us control.
		do
		{
			hr = pDSB->Restore();
			if( hr == DSERR_BUFFERLOST )
				Sleep( 10 );
		} while( ( hr = pDSB->Restore() ) == DSERR_BUFFERLOST );

		if( pbWasRestored != NULL )
			*pbWasRestored = TRUE;

		PROFILE_END;
		return S_OK;
	}
	else
	{
		PROFILE_END;
		return S_OK;
	}
}

HRESULT CBaseSound::FillBuffer(LPDIRECTSOUNDBUFFER pDSB)
{
	HRESULT hr;
	VOID* pDSLockedBuffer = NULL;	// Pointer to locked buffer memory
	DWORD dwDSLockedBufferSize = 0;	// Size of the locked DirectSound buffer
	DWORD dwWavDataRead = 0;		// Amount of data read from the wav file

	if( SUCCEEDED( hr = RestoreBuffer( pDSB, NULL ) ) ) {

		if( SUCCEEDED( hr = pDSB->Lock( 0, 0,
									&pDSLockedBuffer, &dwDSLockedBufferSize,
									NULL, NULL, DSBLOCK_ENTIREBUFFER ) ) ) {

			this->wf->ResetFile();

			if( SUCCEEDED( hr = this->wf->Read( static_cast<BYTE*>(pDSLockedBuffer),
												dwDSLockedBufferSize,
												&dwWavDataRead ) ) ) {

				if( dwWavDataRead == 0 )
				{
					// Wav is blank, so just fill with silence
					FillMemory( static_cast<BYTE*>(pDSLockedBuffer),
								dwDSLockedBufferSize,
								static_cast<BYTE>( this->wf->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
				}
				else if( dwWavDataRead < dwDSLockedBufferSize )
				{
					// If the wav file was smaller than the DirectSound buffer,
					// we need to fill the remainder of the buffer with data
					/* if( bRepeatWavIfBufferLarger )
					{
						// Reset the file and fill the buffer with wav data
						DWORD dwReadSoFar = dwWavDataRead;    // From previous call above.
						while( dwReadSoFar < dwDSLockedBufferSize )
						{
							// This will keep reading in until the buffer is full
							// for very short files
							if( FAILED( hr = m_pWaveFile->ResetFile() ) )
								return DXUT_ERR( L"ResetFile", hr );
	
							hr = m_pWaveFile->Read( ( BYTE* )pDSLockedBuffer + dwReadSoFar,
													dwDSLockedBufferSize - dwReadSoFar,
													&dwWavDataRead );
							if( FAILED( hr ) )
								return DXUT_ERR( L"Read", hr );
	
							dwReadSoFar += dwWavDataRead;
						}
					}
					else
					{*/
						// Don't repeat the wav file, just fill in silence
						FillMemory( static_cast<BYTE*>(pDSLockedBuffer) + dwWavDataRead,
									dwDSLockedBufferSize - dwWavDataRead,
									static_cast<BYTE>( this->wf->m_pwfx->wBitsPerSample == 8 ? 128 : 0 ) );
					//}
				}
			}
			pDSB->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );
		}
	}

	return hr;
}

CSound::CSound(const unsigned int nBuffers):m_dwCreationFlags(0)
{
	this->m_pSoundBuffer.SetSize(nBuffers);
	TRACE("Som %p criado!\n", this);
}

CSound::~CSound()
{
	TRACE("Som %p destruído!\n", this);
}

CSound *CSound::Load(const Smart::String &filename)
{
	CWaveFile *wf = NULL;
	DSBUFFERDESC dsbd;
	HRESULT hr = S_OK;
	DWORD dwDSBufferSize = NULL;
	CSound *mysnd = static_cast<CSound*>(mywrapper->FindResource(filename, TR_SOM));

	if(mysnd)
	{
		TRACE("Som já carregado!\n");
		mysnd->AddRef();
		return mysnd;
	}

	TRACE("Carregando som %s...\n", filename.c_str());

	mysnd = new CSound();
	if(dsounddevice) {
		wf = new CWaveFile();
		if(wf) {
			wf->Open(filename);

			if(wf->GetSize() != 0) {
				dwDSBufferSize = wf->GetSize();
				mysnd->m_dwDSBufferSize = dwDSBufferSize;

				if(dwDSBufferSize <= DSBSIZE_MAX) {
					ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
					dsbd.dwSize = sizeof(DSBUFFERDESC);
					dsbd.dwFlags = DSBCAPS_LOCDEFER | DSBCAPS_CTRLFREQUENCY;// | DSBCAPS_CTRLVOLUME;
					dsbd.dwBufferBytes = dwDSBufferSize-1;
					dsbd.guid3DAlgorithm = GUID_NULL ;
					dsbd.lpwfxFormat = wf->m_pwfx;

					mysnd->m_dwCreationFlags =  dsbd.dwFlags;

					mysnd->wf = wf;

					hr = dsounddevice->CreateSoundBuffer(&dsbd, &mysnd->m_pSoundBuffer[0], NULL);

					if(SUCCEEDED(hr)) {
						if(SUCCEEDED(mysnd->DuplicateBuffers())) {
		 					hr = mysnd->FillBuffer(mysnd->m_pSoundBuffer[0]);

							if(SUCCEEDED(hr)) {
								mysnd->Catalog(filename, TR_SOM);
								TRACE("\tSom %s carregado com sucesso!\n", filename.c_str());
								mysnd->m_pSoundBuffer[0]->GetFrequency(&mysnd->dwFrequency);
								return mysnd;
							} else {
								TRACE("\t");
								TRACE("Falha ao preencher buffer de som! (Código do erro: %d)\n", HRESULT_CODE(hr));
							}
						} else {
							TRACE("\t");
							TRACE("Falha ao duplicar buffers de som! (Código do erro: %d)\n", HRESULT_CODE(hr));
						}
					} else {
						TRACE("\t");
						TRACE("Falha ao criar buffer de som! (Código do erro: %d)\n", HRESULT_CODE(hr));
					}
				} else {
					TRACE("\t");
					TRACE("Tamanho de buffer muito grande! (%d > %d)\n", dwDSBufferSize, DSBSIZE_MAX);
					hr = E_OUTOFMEMORY;
				}
			} else {
				hr = E_FAIL;
				TRACE("\t");
				TRACE("Falha ao carregar arquivo!\n");
			}
		} else {
			hr =  E_OUTOFMEMORY;
			TRACE("\t");
			TRACE("Sem Memória!\n");
		}
	} else {
		hr = DSERR_NOINTERFACE;
		TRACE("\t");
		TRACE("Carregamento Cancelado! (Direct Sound não iniciado!)\n");
	}
	SafeDelete(wf);
	SafeRelease(mysnd);
	TRACE("\n");
	return NULL;
}


C3DSound *C3DSound::Load(const Smart::String &filename)
{
	CWaveFile *wf = NULL;
	DSBUFFERDESC dsbd;
	HRESULT hr = S_OK;
	DWORD dwDSBufferSize = NULL;
	C3DSound *mysnd = static_cast<C3DSound*>(mywrapper->FindResource(filename, TR_SOM3D));

	if(mysnd)
	{
		TRACE("Som já carregado!\n");
		mysnd->AddRef();
		return mysnd;
	}

	TRACE("Carregando som 3D %s...\n", filename.c_str());

	mysnd = new C3DSound();
	if(dsounddevice) {
		wf = new CWaveFile();
		if(wf) {
			hr = wf->Open(filename);

			if(SUCCEEDED(hr)) {
				dwDSBufferSize = wf->GetSize();
				mysnd->m_dwDSBufferSize = dwDSBufferSize;

				if(dwDSBufferSize <= DSBSIZE_MAX && wf->m_pwfx->nChannels <= 1) {
					ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
					dsbd.dwSize = sizeof(DSBUFFERDESC);
					dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_LOCDEFER | DSBCAPS_MUTE3DATMAXDISTANCE  ;
					dsbd.dwBufferBytes = dwDSBufferSize-1;
					dsbd.guid3DAlgorithm = DS3DALG_DEFAULT  ;
					dsbd.lpwfxFormat = wf->m_pwfx;

					mysnd->m_dwCreationFlags =  dsbd.dwFlags;

					mysnd->wf = wf;

					hr = dsounddevice->CreateSoundBuffer(&dsbd, &mysnd->m_pSoundBuffer[0], NULL);
					if(SUCCEEDED(hr)) {

						if(SUCCEEDED(mysnd->DuplicateBuffers())) {
							hr = mysnd->FillBuffer(mysnd->m_pSoundBuffer[0]);
						
							if(SUCCEEDED(hr)) {
								hr = mysnd->m_pSoundBuffer[0]->QueryInterface( IID_IDirectSound3DBuffer, reinterpret_cast< VOID** >(&mysnd->m_pDS3DBuffer) );
								if(SUCCEEDED(hr))
								{
									mysnd->Catalog(filename, TR_SOM3D);
									TRACE("\tSom %s carregado com sucesso!\n", filename.c_str());
									return mysnd;
								} else {
									TRACE("\t");
									TRACE("Erro ao obter interface 3D para o som!\n");
								}
							} else {
								TRACE("\t");
								TRACE("Falha ao preencher buffer de som! Código do erro: %d)\n",HRESULT_CODE(hr));
							}
						} else {
							TRACE("\t");
							TRACE("Falha ao duplicar buffers de som! Código do erro: %d)\n",HRESULT_CODE(hr));
						}
					} else {
						TRACE("\t");
						TRACE("Falha ao criar buffer de som! (Código do erro: %d)\n", HRESULT_CODE(hr));
					}
				} else {
					TRACE("\t");
					TRACE("Tamanho muito grande! (%d > %d)\n", dwDSBufferSize, DSBSIZE_MAX);
					TRACE("\tOu nChannels > 1 (%d)!\n", wf->m_pwfx->nChannels);
					hr = E_OUTOFMEMORY;
				}
			} else {
				TRACE("\t");
				TRACE("Falha ao carregar arquivo!\n");
				SafeDelete(wf);
			}
		} else {
			hr =  E_OUTOFMEMORY;
			TRACE("\t");
			TRACE("Sem Memória!\n");
		}
	} else {
		hr = DSERR_NOINTERFACE;
		TRACE("\t");
		TRACE("Carregamento Cancelado! (Direct Sound não iniciado!)\n");
	}

	if(mysnd) {
		SafeRelease(mysnd);
	} else {
		SafeDelete(wf);
	}
	TRACE("\n");
	return NULL;
}

LPDIRECTSOUNDBUFFER CSound::Play(float fFrequency, float fVolume)
{
	BOOL bRestored;
	PROFILE_START;
	LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();

	if(pDSB) {
		if(SUCCEEDED(RestoreBuffer(pDSB, &bRestored))) {
			if(bRestored)
				this->FillBuffer(pDSB);

			if(fFrequency != -1 && (this->m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY)) {
				DWORD dwFrequency;

				dwFrequency = static_cast<DWORD>(this->dwFrequency * fFrequency);
				pDSB->SetFrequency(dwFrequency);
			}

			/*if(fVolume != 1 && (this->m_dwCreationFlags & DSBCAPS_CTRLVOLUME)) {
				LONG dwVolume;
				ASSERT(fVolume >= 0 && fVolume <= 1)

				dwVolume = static_cast<LONG>(fVolume * (DSBVOLUME_MAX-DSBVOLUME_MIN));
				dwVolume += DSBVOLUME_MIN;
				pDSB->SetVolume(dwVolume);
			}*/

			pDSB->SetCurrentPosition(0);
			pDSB->Play(0, 0, 0);
		}
	}
	PROFILE_END;
	return pDSB;
}

C3DSound::~C3DSound()
{
	TRACE("Som 3D %s removido!\n", this->getFilename().charstr());
}