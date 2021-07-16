//-----------------------------------------------------------------------------
// File: WaveFile.h
//
// Copyright (c) Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _AUDIOWAVE_H
#define _AUDIOWAVE_H

//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{
public:
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO m_ck;          // Multimedia RIFF chunk
    MMCKINFO m_ckRiff;      // Use in opening a WAVE file
    DWORD m_dwSize;      // The size of the wave file
    MMIOINFO m_mmioinfoOut;
    BOOL m_bIsReadingFromMemory;
    BYTE* m_pbData;
    BYTE* m_pbDataCur;
    ULONG m_ulDataSize;
    CHAR* m_pResourceBuffer;

protected:
    HRESULT ReadMMIO();

public:
	CWaveFile();
	~CWaveFile();

	HRESULT Open(const Smart::String &strFileName);
	HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx );
	HRESULT Close();

	HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );

	DWORD   GetSize();
	HRESULT ResetFile();

	inline WAVEFORMATEX* GetFormat() const
    { return m_pwfx; };
};


#endif // DXUTWAVEFILE_H
