#pragma once
#include "gerenciadorderecursos.h"

class VideoResource {
public:
	VideoResource();
	virtual ~VideoResource();

	virtual HRESULT OnLostDevice() = 0;
	virtual HRESULT OnResetDevice() = 0;
};